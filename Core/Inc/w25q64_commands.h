#ifndef W25Q64_COMMANDS_H
#define W25Q64_COMMANDS_H

#include <stdint.h> // 确保 uint8_t 类型可用

// W25Q64 指令集定义 (符合JEDEC标准)
static const uint8_t W25Q64_WRITE_ENABLE = 0x06;         // 写使能
static const uint8_t W25Q64_WRITE_DISABLE = 0x04;        // 写禁止
static const uint8_t W25Q64_READ_STATUS_REG1 = 0x05;     // 读取状态寄存器1
static const uint8_t W25Q64_READ_STATUS_REG2 = 0x35;     // 读取状态寄存器2
static const uint8_t W25Q64_WRITE_STATUS_REG = 0x01;     // 写状态寄存器
static const uint8_t W25Q64_PAGE_PROGRAM = 0x02;         // 页编程
static const uint8_t W25Q64_QUAD_PAGE_PROGRAM = 0x32;    // 四线页编程
static const uint8_t W25Q64_BLOCK_ERASE_64K = 0xD8;      // 64KB块擦除
static const uint8_t W25Q64_BLOCK_ERASE_32K = 0x52;      // 32KB块擦除
static const uint8_t W25Q64_SECTOR_ERASE = 0x20;         // 4KB扇区擦除
static const uint8_t W25Q64_CHIP_ERASE = 0xC7;           // 整片擦除
static const uint8_t W25Q64_ERASE_SUSPEND = 0x75;        // 擦除暂停
static const uint8_t W25Q64_ERASE_RESUME = 0x7A;         // 擦除恢复
static const uint8_t W25Q64_POWER_DOWN = 0xB9;           // 进入掉电模式
static const uint8_t W25Q64_HIGH_PERF_MODE = 0xA3;       // 高性能模式
static const uint8_t W25Q64_CONTINUOUS_READ_MODE = 0xFF; // 连续读取模式
static const uint8_t W25Q64_RELEASE_POWER_DOWN = 0xAB;   // 退出掉电模式
static const uint8_t W25Q64_MANUFACTURER_ID = 0x90;      // 读制造商ID
static const uint8_t W25Q64_DEVICE_ID = 0x9F;            // 读设备ID
static const uint8_t W25Q64_UNIQUE_ID = 0x4B;            // 读唯一ID
static const uint8_t W25Q64_READ_DATA = 0x03;            // 标准数据读取
static const uint8_t W25Q64_FAST_READ = 0x0B;            // 快速读取
static const uint8_t W25Q64_DUAL_OUT_FAST_READ = 0x3B;   // 双线输出快速读取
static const uint8_t W25Q64_DUAL_INOUT_FAST_READ = 0xBB; // 双线输入输出快速读取
static const uint8_t W25Q64_QUAD_OUT_FAST_READ = 0x6B;   // 四线输出快速读取
static const uint8_t W25Q64_QUAD_INOUT_FAST_READ = 0xEB; // 四线输入输出快速读取

static const uint8_t W25Q64_DUMMY_BYTE = 0xFF; // 空指令字节（用于读取占位）

#endif // W25Q64_COMMANDS_H