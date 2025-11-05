/*
 * w25q64.h
 *
 *  Created on: Mar 26, 2025
 *      Author: fwar3
 */

#ifndef INC_W25Q64_H_
#define INC_W25Q64_H_

#include "main.h"
#include "stm32f1xx_hal.h"
#include <inttypes.h>
#include <stdbool.h>

#define BLOCK_MAX_COUNT 128U
// 128 * 16 = 2048
#define SECTOR_MAX_COUNT BLOCK_MAX_COUNT * (64 / 4)
// 2048 * 16 = 32768
#define PAGE_MAX_COUNT SECTOR_MAX_COUNT * (4096 / 256)
#define PAGE_MAX_SIZE 256U

#define W25Q64_START()                                                         \
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define W25Q64_STOP()                                                          \
    HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)

void W25q64Init();
bool W25q64WaitforBusy();
int32_t W25q64ReadDeviceId(uint8_t *mid, uint16_t *did);
int32_t W25q64EraseSector(uint32_t address);
int32_t W25q64EraseBlock32K(uint32_t addresss);
int32_t W25q64EraseBlock64K(uint32_t addresss);
int32_t W25q64EraseChip();
int32_t W25q64WritePage(uint32_t address, uint8_t *data, uint32_t dataLen);
int32_t W25q64StandRead(uint32_t address, uint32_t readLen, uint8_t *readBuf);

#endif /* INC_W25Q64_H_ */
