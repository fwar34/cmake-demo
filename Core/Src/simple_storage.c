#include "simple_storage.h"
#include "error_define.h"
#include "log.h"
#include "w25q64.h"
#include <stdbool.h>
#include <string.h>

#define PAGE_ALIGN_NUM 0x100U

#define UINT32_BIT_NUM sizeof(uint32_t)
// 32768 / 32 + 1 = 1025
// #define PAGE_BITMAP_ARRAY_LEN (PAGE_MAX_COUNT / UINT32_BIT_NUM + 1)

// 32768 / 32 = 1024
#define PAGE_BITMAP_ARRAY_LEN (PAGE_MAX_COUNT / UINT32_BIT_NUM)
// 1025 * 4 = 4100
#define BITMAP_ARRAY_BYTE_LEN (PAGE_BITMAP_ARRAY_LEN * sizeof(uint32_t))
// 4100 / 256 = 16
#define BITMAP_ARRAY_USE_PAGE_SIZE                                             \
    ((BITMAP_ARRAY_BYTE_LEN % PAGE_MAX_SIZE) == 0                              \
         ? ((BITMAP_ARRAY_BYTE_LEN) & (~PAGE_ALIGN_NUM))                       \
         : ((BITMAP_ARRAY_BYTE_LEN) & (~PAGE_ALIGN_NUM)) + 1)
#define BITMAP_ARRAY_USE_PAGE_START_ADDRESS 0U
#define MAX_STORAGE_COUNT 128U
#define MAX_PATH_LENGTH 16U

// 头部预留BITMAP_ARRAY_USE_PAGE_SIZE(16)个page来保存g_pageUsageBitMap
static uint32_t g_pageUsageBitMap[PAGE_BITMAP_ARRAY_LEN] = {0};

typedef enum {
    STORAGE_FLAG_FREE,
    STORAGE_FLAG_IN_USE,
} StorageFlag;

typedef struct {
    uint32_t flag;
    char path[MAX_PATH_LENGTH];
    uint32_t startAddress;
    uint32_t length;
} StorageBlockInfo;
static StorageBlockInfo g_stroageBlockInfo[MAX_STORAGE_COUNT] = {0};

static void SetPageUsageBit(uint16_t pageIndex)
{
    uint16_t segment = pageIndex / UINT32_BIT_NUM;
    uint16_t offset = pageIndex % UINT32_BIT_NUM;
    g_pageUsageBitMap[segment] |= 1 << offset;
}

static void ClearPageUsageBit(uint16_t pageIndex)
{
    uint16_t segment = pageIndex / UINT32_BIT_NUM;
    uint16_t offset = pageIndex % UINT32_BIT_NUM;
    g_pageUsageBitMap[segment] &= ~(1 << offset);
}

static int32_t SyncPageUsageBitMap()
{
    int32_t ret = W25q64EraseSector(BITMAP_ARRAY_USE_PAGE_START_ADDRESS);
    if (ret != RETURN_VALUE_OK) {
        LogError("erase bit map sector failed!");
        return ret;
    }
}

static int32_t FindNextEmptyPage()
{
    uint32_t index = 0;
    for (uint32_t i = 0; i < PAGE_BITMAP_ARRAY_LEN; i++) {
        for (uint8_t j = 0; j < UINT32_BIT_NUM; j++) {
            uint8_t tmp = (g_pageUsageBitMap[i] >> j) & 0x1;
            if (tmp == 0) {
                return i * UINT32_BIT_NUM + j;
            }
        }
    }

    return -1;
}

static bool IsPageInUse(uint16_t pageIndex)
{
    uint16_t segment = pageIndex / UINT32_BIT_NUM;
    uint16_t offset = pageIndex % UINT32_BIT_NUM;
    return (g_pageUsageBitMap[segment] & (1 << offset)) != 0;
}

static int32_t FoundExistOrUseablePath(const char *path)
{
    int32_t ret = STORAGE_ERROR_FULL;
    for (uint32_t i = 0; i < SECTOR_MAX_COUNT; i++) {
        if (strncmp(path, g_stroageBlockInfo[i].path, MAX_PATH_LENGTH) == 0) {
            return i;
        }

        if (g_stroageBlockInfo[i].flag == STORAGE_FLAG_FREE &&
            ret == STORAGE_ERROR_FULL) {
            ret = i;
        }
    }

    return ret;
}

int32_t Open(const char *path)
{
    if (path == NULL) {
        LogError("path is null");
        return STORAGE_ERROR_FAILED;
    }

    int32_t storageId = FoundExistOrUseablePath(path);
    if (storageId == STORAGE_ERROR_INVALID_ID) {
        return storageId;
    }

    strncpy(g_stroageBlockInfo[storageId].path, path, strlen(path));
    (g_stroageBlockInfo[storageId].path)[MAX_PATH_LENGTH] = '\0';

    return storageId;
}

int32_t Delete(const char *path)
{
    if (path == NULL) {
        LogError("path is null");
        return STORAGE_ERROR_FAILED;
    }

    int32_t storageId = FoundExistOrUseablePath(path);
    if (storageId == STORAGE_ERROR_INVALID_ID) {
        return STORAGE_ERROR_NOT_FOUND;
    }

    memset(g_stroageBlockInfo[storageId].path, 0, MAX_PATH_LENGTH);
    return STORAGE_ERROR_OK;
}

void SyncPageBitMap()
{

    // uint32_t beginAddress = 0;
    // for (uint16_t i = 0; i < BITMAP_ARRAY_BYTE_LEN)
}

void SyncStorageBlockInfo() {}

int32_t SaveData(const uint8_t *data, uint32_t dataLen) { return 0; }
