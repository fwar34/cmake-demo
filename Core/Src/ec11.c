#include "ec11.h"
#include "circle_buffer.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <string.h>

#define KEY_BUF_SIZE 4
// 按键消抖时间
#define KEY_DEBOUNCING_TIME_10MS 10
// 双击阈值
#define KEY_DOUBLE_TIME_200MS 200
// 单击识别长按
#define KEY_LONG_CLICK_TIME_700MS 700

static int32_t encodeCounter = 0;

// 按键子状态
typedef enum {
    KEY_STEP_PRESS,              // 按下
    KEY_STEP_PRESS_DEBOUNCING,   // 按下去抖（可以和按下合并）
    KEY_STEP_RELEASE_DEBOUNCING, // 释放去抖
    KEY_STEP_RELEASE,            // 释放（可以和释放去抖合并）
} KeyStep;

typedef void (*ItemCallback)();
typedef struct {
    Ec11KeyState currentState; // 当前主状态
    KeyStep currentStep;       // 当前子状态
    ItemCallback callback;
} Ec11KeyStateMachineItem;

void ProcessClickPress();
void ProcessClickPressDebouncing();
void ProcessClickReleaseDebouncing();
void ProcessClickRelease();

void ProcessDoubleClickPress();
void ProcessDoubleClickDebouncing();
void ProcessDoubleClickReleaseDebouncing();
void ProcessDoubleClickRelease();

static Ec11KeyStateMachineItem ec11StateMachineTable[] = {
    // ec11 按键的检测
    {EC11_KEY_CLICK, KEY_STEP_PRESS, ProcessClickPress},
    {EC11_KEY_CLICK, KEY_STEP_PRESS_DEBOUNCING, ProcessClickPressDebouncing},
    {EC11_KEY_CLICK, KEY_STEP_RELEASE_DEBOUNCING,
     ProcessClickReleaseDebouncing},
    {EC11_KEY_CLICK, KEY_STEP_RELEASE, ProcessClickRelease},

    {EC11_KEY_DOUBLE_CLICK, KEY_STEP_PRESS, ProcessDoubleClickPress},
    {EC11_KEY_DOUBLE_CLICK, KEY_STEP_PRESS_DEBOUNCING,
     ProcessDoubleClickDebouncing},
    {EC11_KEY_DOUBLE_CLICK, KEY_STEP_RELEASE_DEBOUNCING,
     ProcessDoubleClickReleaseDebouncing},
    {EC11_KEY_DOUBLE_CLICK, KEY_STEP_RELEASE, ProcessDoubleClickRelease},
};

typedef enum {
    EC11_DIRECTION_INVALID = 0,
    EC11_DIRECTION_SEQUENCE_HALF = 1,
    EC11_DIRECTION_REVERSE_HALF,
} Ec11DirectionState;

DefineCircleBuffer(Ec11KeyCircleBuf, KeyInfo, KEY_BUF_SIZE);

typedef struct {
    Ec11KeyCircleBuf keyCircleBuf;
    Ec11KeyStateMachineItem ec11StateMachine;
    GPIO_PinState lastKeyLevel;
    uint32_t lastPressTick;  // 长按判断
    uint32_t debouncingTick; // 去抖
    uint32_t lastClickTick;  // 识别到单击的tick，判断双击用
    bool hasRotate;          // ec11是否旋转
} Ec11Encoder;

static Ec11Encoder ec11Encoder;

const char *GetKeyName(uint8_t keyIndex)
{
    static const char *keyIndexNameArray[] = {"key0", "ec11_key"};

    uint8_t size = sizeof(keyIndexNameArray) / sizeof(keyIndexNameArray[0]);
    if (keyIndex >= size) {
        return NULL;
    }
    return keyIndexNameArray[keyIndex];
}

const char *GetKeyEventName(uint8_t keyEvent)
{
    static const char *keyStateNameArray[] = {
        "invalid",          "press",       "click",        "double_click",
        "long_click",       "left_rotate", "right_rotate", "hold_left_rotate",
        "hold_right_rotate"};

    uint8_t size = sizeof(keyStateNameArray) / sizeof(keyStateNameArray[0]);
    if (keyEvent >= size) {
        return NULL;
    }
    return keyStateNameArray[keyEvent];
}

void Ec11ResetStateMachine()
{
    ec11Encoder.ec11StateMachine.currentState = EC11_KEY_CLICK;
    ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_PRESS;
    ec11Encoder.lastKeyLevel = GPIO_PIN_SET;
    ec11Encoder.lastPressTick = 0;
    ec11Encoder.debouncingTick = 0;
    ec11Encoder.lastClickTick = 0;
    ec11Encoder.hasRotate = false;
}

void Ec11EncoderInit()
{
    InitBuf(&ec11Encoder.keyCircleBuf, KeyInfo, KEY_BUF_SIZE);
    Ec11ResetStateMachine();
}

void ProcessClickPress()
{
    GPIO_PinState currentKeyLevel =
        HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin);
    if (currentKeyLevel == GPIO_PIN_RESET) {
        ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_PRESS_DEBOUNCING;
        ec11Encoder.debouncingTick = HAL_GetTick();
    }
}

void ProcessClickPressDebouncing()
{
    uint32_t currTick = HAL_GetTick();
    GPIO_PinState currentKeyLevel =
        HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin);
    if (currTick - ec11Encoder.debouncingTick > KEY_DEBOUNCING_TIME_10MS) {
        if (currentKeyLevel == GPIO_PIN_RESET) { // 去抖成功
            ec11Encoder.ec11StateMachine.currentStep =
                KEY_STEP_RELEASE_DEBOUNCING;
            ec11Encoder.lastPressTick = currTick; // 保存识别按下的tick
            ec11Encoder.debouncingTick = 0; // 重置去抖tick，为释放去抖准备

            KeyInfo keyInfo = {EC11_KEY, EC11_KEY_PRESS, 0};
            WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
        } else {
            Ec11ResetStateMachine();
        }
    }
}

void ProcessClickReleaseDebouncing()
{
    if (HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin) == GPIO_PIN_SET) {
        if (ec11Encoder.debouncingTick == 0) {
            ec11Encoder.debouncingTick = HAL_GetTick();
        }

        if (HAL_GetTick() - ec11Encoder.debouncingTick >
            KEY_DEBOUNCING_TIME_10MS) {
            ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_RELEASE;
            ec11Encoder.debouncingTick = 0;
        }
    }
}

void ProcessClickRelease()
{
    if (ec11Encoder.hasRotate) { // 旋转后放开
        Ec11ResetStateMachine();
        return;
    }

    KeyInfo keyInfo = {EC11_KEY, EC11_KEY_INVALID, 0};
    uint32_t resetLevelTick = HAL_GetTick() - ec11Encoder.lastPressTick;
    if (resetLevelTick <= KEY_LONG_CLICK_TIME_700MS) { // 单击
        ec11Encoder.lastClickTick =
            HAL_GetTick(); // 更新单击tick，判断双击的时候使用
                           //			keyInfo.keyState = EC11_KEY_CLICK;

        ec11Encoder.ec11StateMachine.currentState = EC11_KEY_DOUBLE_CLICK;
        ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_PRESS;
        ec11Encoder.lastPressTick = 0;
    } else if (resetLevelTick > KEY_LONG_CLICK_TIME_700MS) { // 长按
        keyInfo.keyState = EC11_KEY_LONG_CLICK;
        WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
        Ec11ResetStateMachine();
    }
}

void ProcessDoubleClickPress()
{
    uint32_t currTick = HAL_GetTick();
    uint32_t resetLevelTick = currTick - ec11Encoder.lastClickTick;
    GPIO_PinState currKeyLevel =
        HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin);

    if (currKeyLevel == GPIO_PIN_SET &&
        resetLevelTick >= KEY_DOUBLE_TIME_200MS) {
        if (ec11Encoder.lastClickTick !=
            0) { // 双击阈值内没有双击，则发送上次的单击事件
            KeyInfo keyInfo = {EC11_KEY, EC11_KEY_CLICK, 0};
            WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
        }
        ec11Encoder.ec11StateMachine.currentState = EC11_KEY_CLICK;
        ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_PRESS;
        ec11Encoder.lastClickTick = 0;
    }

    if (currKeyLevel == GPIO_PIN_RESET &&
        resetLevelTick < KEY_DOUBLE_TIME_200MS) {
        ec11Encoder.debouncingTick = currTick;
        ec11Encoder.ec11StateMachine.currentState = EC11_KEY_DOUBLE_CLICK;
        ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_PRESS_DEBOUNCING;
    }
}

void ProcessDoubleClickDebouncing()
{
    uint32_t currTick = HAL_GetTick();
    if (HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin) == GPIO_PIN_RESET) {
        if (currTick - ec11Encoder.debouncingTick > KEY_DEBOUNCING_TIME_10MS) {
            ec11Encoder.ec11StateMachine.currentState = EC11_KEY_DOUBLE_CLICK;
            ec11Encoder.ec11StateMachine.currentStep =
                KEY_STEP_RELEASE_DEBOUNCING;
            ec11Encoder.lastPressTick = currTick;
            ec11Encoder.debouncingTick = 0;
        }
    }
}

void ProcessDoubleClickReleaseDebouncing()
{
    uint32_t currTick = HAL_GetTick();
    GPIO_PinState currKeyLevel =
        HAL_GPIO_ReadPin(ec11_Key_GPIO_Port, ec11_Key_Pin);
    if (currKeyLevel == GPIO_PIN_SET) {
        if (ec11Encoder.debouncingTick == 0) {
            ec11Encoder.debouncingTick = currTick;
        }

        if (currTick - ec11Encoder.debouncingTick >
            KEY_DEBOUNCING_TIME_10MS) { // 去抖时间到达
            ec11Encoder.ec11StateMachine.currentState = EC11_KEY_DOUBLE_CLICK;
            ec11Encoder.ec11StateMachine.currentStep = KEY_STEP_RELEASE;
            ec11Encoder.debouncingTick = 0;
        }
    }
}

void ProcessDoubleClickRelease()
{
    if (ec11Encoder.hasRotate) {
        Ec11ResetStateMachine();
        return;
    }

    KeyInfo keyInfo = {EC11_KEY, EC11_KEY_DOUBLE_CLICK, 0};
    //	uint32_t currTick = HAL_GetTick();
    //	if (currTick - ec11Encoder.lastPressTick > KEY_LONG_CLICK_TIME_700MS) {
    //		keyInfo.keyState = EC11_KEY_LONG_CLICK;  //
    // 单击+双击阈值内长按识别为长按
    //	}
    WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
    Ec11ResetStateMachine();
}

static void Ec11TickProcess() // ec11 旋转的识别
{
    static Ec11DirectionState ec11CurrentState = EC11_DIRECTION_INVALID;
    static GPIO_PinState lastALevel = GPIO_PIN_SET;
    static GPIO_PinState lastBLevel = GPIO_PIN_SET;
    static GPIO_PinState currALevel = GPIO_PIN_SET;
    static GPIO_PinState currBLevel = GPIO_PIN_SET;
    static uint8_t switchACounter = 0;
    static uint8_t switchBCounter = 0;

    currALevel = HAL_GPIO_ReadPin(ec11_A_GPIO_Port, ec11_A_Pin);
    currBLevel = HAL_GPIO_ReadPin(ec11_B_GPIO_Port, ec11_B_Pin);
    if (currALevel != lastALevel) {
        switchACounter = (switchACounter + 1) > 2 ? 1 : (switchACounter + 1);
        if (currALevel == GPIO_PIN_RESET) { // A相下降沿
            if (currBLevel == GPIO_PIN_SET) {
                ec11CurrentState = EC11_DIRECTION_SEQUENCE_HALF;
            } else {
                ec11CurrentState = EC11_DIRECTION_REVERSE_HALF;
            }
        } else { // A相上升沿
            if (ec11CurrentState == EC11_DIRECTION_SEQUENCE_HALF) {
                if (switchBCounter == 2) {
                    ec11CurrentState = EC11_DIRECTION_INVALID;
                } else if (switchBCounter == 1) {
                    encodeCounter--;
                    KeyInfo keyInfo = {EC11_KEY, EC11_KEY_LEFT_ROTATE,
                                       encodeCounter};
                    if (ec11Encoder.ec11StateMachine.currentStep ==
                        KEY_STEP_RELEASE_DEBOUNCING) {
                        ec11Encoder.hasRotate = true;
                        keyInfo.keyState = EC11_KEY_PRESS_LEFT_ROTATE;
                    }
                    WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
                }
            } else if (ec11CurrentState == EC11_DIRECTION_REVERSE_HALF) {
                if (switchBCounter == 1) {
                    ec11CurrentState = EC11_DIRECTION_INVALID;
                } else if (switchBCounter == 2) {
                    encodeCounter++;
                    KeyInfo keyInfo = {EC11_KEY, EC11_KEY_RIGHT_ROTATE,
                                       encodeCounter};
                    if (ec11Encoder.ec11StateMachine.currentStep ==
                        KEY_STEP_RELEASE_DEBOUNCING) {
                        ec11Encoder.hasRotate = true;
                        keyInfo.keyState = EC11_KEY_PRESS_RIGHT_ROTATE;
                    }
                    WriteBuf(&ec11Encoder.keyCircleBuf, keyInfo);
                }
            }
        }
        lastALevel = currALevel;
    }

    if (currBLevel != lastBLevel) {
        switchBCounter = (switchBCounter + 1) > 2 ? 1 : (switchBCounter + 1);
        if (currBLevel == GPIO_PIN_SET) { // B 相上升沿
            if (ec11CurrentState == EC11_DIRECTION_SEQUENCE_HALF) {
                if (switchACounter == 1) {
                    ec11CurrentState = EC11_DIRECTION_INVALID;
                }
            } else if (ec11CurrentState == EC11_DIRECTION_REVERSE_HALF) {
                if (switchACounter == 2) {
                    ec11CurrentState = EC11_DIRECTION_INVALID;
                }
            }
        }
        lastBLevel = currBLevel;
    }
}

void Ec11StateMachineProcess()
{
    for (uint32_t i = 0;
         i < sizeof(ec11StateMachineTable) / sizeof(ec11StateMachineTable[0]);
         ++i) { // ec11 按键识别
        if (ec11StateMachineTable[i].currentState ==
                ec11Encoder.ec11StateMachine.currentState &&
            ec11StateMachineTable[i].currentStep ==
                ec11Encoder.ec11StateMachine.currentStep) {
            ec11StateMachineTable[i].callback();
        }
    }

    Ec11TickProcess(); // ec11 旋转识别
}

void GetKeyState(KeyInfo *out, uint16_t *remain, uint8_t *ret)
{
    ReadBuf(&ec11Encoder.keyCircleBuf, out, remain, ret);
}
