#include "load_image.h"
#include "error_define.h"
#include "log.h"
#include "w25q64.h"

void LoadImageInit()
{
    uint8_t mid = 0;
    uint16_t did = 0;
    W25q64Init();

    int32_t ret = W25q64ReadDeviceId(&mid, &did);
    if (ret != RETURN_VALUE_OK) {
        return;
    }

    LogInfo("mid: %d, did: %d\n", mid, did);
}

void LoadImage() {}
