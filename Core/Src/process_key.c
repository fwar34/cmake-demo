/*
 * process_key.c
 *
 *  Created on: Mar 23, 2025
 *      Author: fwar3
 */
#include "process_key.h"
#include "stm32f1xx_hal.h"
#include "ec11.h"
#include "usart.h"
#include "timer3.h"
#include "process_render.h"
#include <stdio.h>
#include <string.h>

void ProcessKey()
{
	static uint32_t lastTick = 0;
	uint32_t currentTick = HAL_GetTick();
	if (currentTick - lastTick < 1) {
		return;
	}
	lastTick = currentTick;

	KeyInfo keyInfo;
	uint16_t remainCount = 0;
	uint8_t ret = 0;
	do {
		GetKeyState(&keyInfo, &remainCount, &ret);
		if (ret != 0) {
			UartPrintf("read %s keyState(%s,%d) encodeCounter(%ld) remain count(%d)\n",
					GetKeyName(keyInfo.keyIndex), GetKeyEventName(keyInfo.keyState), keyInfo.keyState,
					keyInfo.encodeCounter, remainCount);

			if (keyInfo.keyIndex == EC11_KEY) {
				switch (keyInfo.keyState) {
				case EC11_KEY_PRESS:
					break;
				case EC11_KEY_CLICK:
					HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);
					break;
				case EC11_KEY_DOUBLE_CLICK:
				{
					// HAL_GPIO_TogglePin(testIO_GPIO_Port, testIO_Pin);
					// delay_us(20);
					// HAL_GPIO_TogglePin(testIO_GPIO_Port, testIO_Pin);
					UartPrintf("%s\n", "testxxxx");
					ImageMoveToggle();
					break;
				}
				case EC11_KEY_LEFT_ROTATE:
					ImageMoveLeft(2);
					break;
				case EC11_KEY_RIGHT_ROTATE:
					ImageMoveRight(2);
					break;
				default:
					break;
				}
			}
		}

	} while (remainCount != 0);
}
