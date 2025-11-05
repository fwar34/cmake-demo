#include "dht11.h"
#include "spinlock.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <timer3.h>

#define START_LOW_TIME_18MS 18
#define START_HIGH_TIME_20US 20
#define DHT11_DATA_BIT_THRESHOLD_50US 50
#define DHT11_DATA_BIT_NUM_OF_BYTE_8 8
#define DHT11_DATA_BYTE_NUM_5 5

typedef enum {
	DHT_STATE_INVALID = -1,
	DHT_STATE_START_MEASURE,
//	DHT_STATE_START_MEASURE_WRITE_HIGH,
	DHT_STATE_READ_DATA,
	DHT_STATE_READ_COMPLETE,
} Dht11State;

typedef struct {
	Dht11Result result;
	SpinLock resultLock;
	uint32_t lastTick;
	Dht11State currentState;
} Dht11Context;

static Dht11Context dht11Context;

typedef void (*StateCallback)();

typedef struct {
	Dht11State state;
	StateCallback callback;
} Dht11StateItem;

void StartMeasure();
void ReadData();

static Dht11StateItem stateTable[] = {
	{ DHT_STATE_START_MEASURE, StartMeasure },
	{ DHT_STATE_READ_DATA, ReadData },
};

void Dht11Init()
{
	memset(&dht11Context.result, 0, sizeof(dht11Context.result));
	dht11Context.resultLock = 0;
	dht11Context.currentState = DHT_STATE_START_MEASURE;
}

void Dht11Process()
{
	static uint32_t lastTick = 0;
	uint32_t currTick = HAL_GetTick();
	if (dht11Context.currentState == DHT_STATE_START_MEASURE && currTick - lastTick < 3000) {
		return;
	}
	lastTick = currTick;

	for (uint8_t i = 0; i < sizeof(stateTable) / sizeof(stateTable[0]); i++) {
		if (stateTable[i].state == dht11Context.currentState) {
			stateTable[i].callback();
		}
	}
}

static void Dht11Output()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = dht11_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(dht11_GPIO_Port, &GPIO_InitStruct);
}

static bool CheckSum(uint8_t *data)
{
	return (data[0] + data[1] + data[2] + data[3]) == data[4];
}

static void Dht11Input()
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = dht11_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(dht11_GPIO_Port, &GPIO_InitStruct);
}

void StartMeasure()
{
	Dht11Output();
	HAL_GPIO_WritePin(dht11_GPIO_Port, dht11_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(dht11_GPIO_Port, dht11_Pin, GPIO_PIN_RESET);
	dht11Context.lastTick = HAL_GetTick();
	dht11Context.currentState = DHT_STATE_READ_DATA;
}

void ReadData()
{
	uint32_t currTick = HAL_GetTick();
	if (currTick - dht11Context.lastTick < START_LOW_TIME_18MS) {
		return;
	}

    __disable_irq();  // 禁用中断
	StartTimer3();
	HAL_GPIO_WritePin(ec11_A_GPIO_Port, ec11_A_Pin, GPIO_PIN_SET);
	uint32_t lastTick = Timer3Count();
	uint32_t tmr3Count = 0;
	Dht11Input();
	while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_SET) {
		tmr3Count = Timer3Count();
		if (tmr3Count - lastTick > 50) {
			__enable_irq();
			StopTimer3();
			dht11Context.currentState = DHT_STATE_START_MEASURE;
			return;
		}
	}

	lastTick = Timer3Count();
	while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_RESET) {
		tmr3Count = Timer3Count();
		if (tmr3Count - lastTick > 100) {
			__enable_irq();
			StopTimer3();
			dht11Context.currentState = DHT_STATE_START_MEASURE;
			return;
		}
	}

	lastTick = Timer3Count();
	while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_SET) {
		tmr3Count = Timer3Count();
		if (tmr3Count - lastTick > 100) {
			__enable_irq();
			StopTimer3();
			dht11Context.currentState = DHT_STATE_START_MEASURE;
			return;
		}
	}

	lastTick = Timer3Count();
	while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_RESET) {
		tmr3Count = Timer3Count();
		if (tmr3Count - lastTick > 60) {
			__enable_irq();
			StopTimer3();
			dht11Context.currentState = DHT_STATE_START_MEASURE;
			return;
		}
	}

	uint8_t data[DHT11_DATA_BYTE_NUM_5] = {0};
	lastTick = Timer3Count();
	for (uint8_t i = 0; i < DHT11_DATA_BYTE_NUM_5; ++i) {
		for (uint8_t j = 0; j < DHT11_DATA_BIT_NUM_OF_BYTE_8; ++j) {
			while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_SET) {
				tmr3Count = Timer3Count();
				if (tmr3Count - lastTick > 100) {
					__enable_irq();
					StopTimer3();
					dht11Context.currentState = DHT_STATE_START_MEASURE;
					return;
				}
			}
			uint32_t delta = Timer3Count() - lastTick;
			if (delta > DHT11_DATA_BIT_THRESHOLD_50US) {
				data[i] = (data[i] << 1) | 0x1;
			} else {
				data[i] = data[i] << 1;
			}

			lastTick = Timer3Count();
			while (HAL_GPIO_ReadPin(dht11_GPIO_Port, dht11_Pin) == GPIO_PIN_RESET) {
				tmr3Count = Timer3Count();
				if (tmr3Count - lastTick > 60) {
					__enable_irq();
					StopTimer3();
					dht11Context.currentState = DHT_STATE_START_MEASURE;
					return;
				}
			}
			lastTick = Timer3Count();
		}
	}

	StopTimer3();
	dht11Context.currentState = DHT_STATE_START_MEASURE;
    __enable_irq();  // 恢复中断

	if (CheckSum(data)) {
		dht11Context.result.humidity = (float)(data[0]) + (float)(data[1]) / 100;
		dht11Context.result.temperature = (float)(data[2]) + (float)(data[3]) / 100;
	}
}

void GetDht11Result(Dht11Result *result)
{
	*result = dht11Context.result;
}
