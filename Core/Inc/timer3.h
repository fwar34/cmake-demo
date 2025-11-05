/*
 * timer.h
 *
 *  Created on: Mar 8, 2025
 *      Author: fwar3
 */

#ifndef INC_TIMER3_H_
#define INC_TIMER3_H_

#include "stm32f1xx_hal.h"
#include <inttypes.h>

void TIM3_Init(void);
void StartTimer3();
void StopTimer3();
uint32_t Timer3Count();
void delay_us(uint32_t us);

#endif /* INC_TIMER3_H_ */
