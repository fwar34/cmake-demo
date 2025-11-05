#include "w25q64.h"
#include "error_define.h"
#include "log.h"
#include "spi.h"
#include "usart.h"
#include "w25q64_commands.h"

#define ADDRESS_ALIGN_256B 0xFF
#define ADDRESS_ALIGN_4K 0x1000
#define ADDRESS_ALIGN_32K 0x8000
#define ADDRESS_ALIGN_64K 0x10000

void W25q64Init() {}

/// @brief 获取mid和did
/// @param mid 239
/// @param did 166407
/// @return
int32_t W25q64ReadDeviceId(uint8_t *mid, uint16_t *did)
{
    uint8_t response[3] = {0};
    int32_t ret = HAL_OK;

    W25Q64_START();
    do {
        // 发送命令
        ret = HAL_SPI_Transmit(&hspi1, &W25Q64_DEVICE_ID, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            LogError("Send command failed: %d", ret);
            break;
        }

        // 接收3字节响应（Manufacturer ID + Device ID高字节 + Device ID低字节）
        ret = HAL_SPI_TransmitReceive(&hspi1, &W25Q64_DUMMY_BYTE, response, 3,
                                      HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            LogError("Receive data failed: %d", ret);
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        *mid = response[0];
        *did = (response[1] << 8) | response[2]; // 合并高、低字节
        return RETURN_VALUE_OK;
    } else {
        return RETURN_VALUE_ERROR;
    }
}

int32_t W25q64EraseSector(uint32_t address)
{
    address &= ~ADDRESS_ALIGN_4K;
    int ret = HAL_OK;

    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t[]){W25Q64_SECTOR_ERASE, (address >> 16) & 0xFF,
                        (address >> 8) & 0xFF, address & 0xFF},
            4, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        W25q64WaitforBusy();
        ret = RETURN_VALUE_OK;
    } else {
        ret = RETURN_VALUE_ERROR;
    }

    return ret;
}

int32_t W25q64EraseBlock32K(uint32_t address)
{
    address &= ~ADDRESS_ALIGN_32K;
    int ret = HAL_OK;

    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t[]){W25Q64_BLOCK_ERASE_32K, (address >> 16) & 0xFF,
                        (address >> 8) & 0xFF, address & 0xFF},
            4, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        W25q64WaitforBusy();
    }

    return 0;
}

int32_t W25q64EraseBlock64K(uint32_t addresss)
{
    addresss &= ~ADDRESS_ALIGN_64K;
    int ret = HAL_OK;

    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t[]){W25Q64_BLOCK_ERASE_64K, (addresss >> 16) & 0xFF,
                        (addresss >> 8) & 0xFF, addresss & 0xFF},
            4, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        W25q64WaitforBusy();
        ret = RETURN_VALUE_OK;
    } else {
        ret = RETURN_VALUE_ERROR;
    }

    return 0;
}

int32_t W25q64EraseChip()
{
    int ret = HAL_OK;
    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(&hspi1, &W25Q64_CHIP_ERASE, 1, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        ret = RETURN_VALUE_OK;
    } else {
        ret = RETURN_VALUE_ERROR;
    }

    return 0;
}

int32_t W25q64WritePage(uint32_t address, uint8_t *data, uint32_t dataLen)
{
    int ret = HAL_OK;
    if (dataLen > PAGE_MAX_SIZE) {
        return RETURN_VALUE_ERROR;
    }

    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t[]){W25Q64_WRITE_ENABLE, (address >> 16) & 0xFF,
                        (address >> 8) & 0xFF, address & 0xFF},
            4, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }

        ret = HAL_SPI_Transmit(&hspi1, data, dataLen, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        W25q64WaitforBusy();
        ret = RETURN_VALUE_OK;
    } else {
        ret = RETURN_VALUE_ERROR;
    }

    return ret;
}

int32_t W25q64StandRead(uint32_t address, uint32_t readLen, uint8_t *readBuf)
{
    int32_t ret = HAL_OK;

    W25Q64_START();
    do {
        ret = HAL_SPI_Transmit(
            &hspi1,
            (uint8_t[]){W25Q64_READ_DATA, (address >> 16) & 0xFF,
                        (address >> 8) & 0xFF, address & 0xFF},
            4, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }

        ret = HAL_SPI_Receive(&hspi1, readBuf, readLen, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            break;
        }
    } while (0);
    W25Q64_STOP();

    if (ret == HAL_OK) {
        ret = RETURN_VALUE_OK;
    } else {
        ret = RETURN_VALUE_ERROR;
    }
    return 0;
}

bool W25q64WaitforBusy()
{
    uint32_t count = 0;
    HAL_SPI_Transmit(&hspi1, &W25Q64_READ_STATUS_REG1, 1, HAL_MAX_DELAY);
    uint8_t reg1State = 0;
    while (HAL_SPI_TransmitReceive(&hspi1, &W25Q64_DUMMY_BYTE, &reg1State, 1,
                                   HAL_MAX_DELAY) == HAL_OK &&
           ((reg1State & 0x1) == 1)) {
        count++;
        if (count >= 1000000) {
            return false;
        }
    }

    return true;
}
