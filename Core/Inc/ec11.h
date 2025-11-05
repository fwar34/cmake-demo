#ifndef __EC11_H_
#define __EC11_H_

#include <inttypes.h>
#include <stdbool.h>

#define KEY0 0
#define EC11_KEY 1

typedef struct {
	uint8_t keyIndex;
	uint8_t keyState;
	uint32_t encodeCounter;
} KeyInfo;

// 按键主状态
typedef enum {
	EC11_KEY_INVALID,
	EC11_KEY_PRESS, // 按下
	EC11_KEY_CLICK,	 // 单击（按下后释放触发）
	EC11_KEY_DOUBLE_CLICK, // 双击
	EC11_KEY_LONG_CLICK, // 长按
	EC11_KEY_LEFT_ROTATE, // 向左旋转一下
	EC11_KEY_RIGHT_ROTATE, // 向右旋转一下
	EC11_KEY_PRESS_LEFT_ROTATE, // 按住向左旋转一下
	EC11_KEY_PRESS_RIGHT_ROTATE, // 按住向右旋转一下
} Ec11KeyState;

void Ec11EncoderInit();
const char *GetKeyName(uint8_t keyIndex);
const char *GetKeyEventName(uint8_t keyEvent);
void Ec11StateMachineProcess();
void GetKeyState(KeyInfo *out, uint16_t *remain, uint8_t *ret);

#endif
