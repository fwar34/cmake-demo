#ifndef __SIMPLE_STORAGE_H_
#define __SIMPLE_STORAGE_H_

#include <stdint.h>

typedef struct
{
    uint32_t beginAddress;
} PageInfo;

int32_t SaveData(const uint8_t *data, uint32_t dataLen);


#endif // !__SIMPLE_STORAGE_H_