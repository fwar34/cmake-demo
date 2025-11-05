#ifndef __LOG_H_
#define __LOG_H_

#include <inttypes.h>

typedef enum {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} LogLevel;

#define LogDebug(fmt, ...) Log(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...) Log(LOG_INFO, fmt, ##__VA_ARGS__)
#define LogWarning(fmt, ...) Log(LOG_WARNING, fmt, ##__VA_ARGS__)
#define LogError(fmt, ...) Log(LOG_ERROR, fmt, ##__VA_ARGS__)

void Log(uint8_t level, const char *fmt, ...);

#endif // !__LOG_H_
