#include "log.h"
#include "usart.h"
#include <stdarg.h>
#include <stdio.h>

void Log(uint8_t level, const char *fmt, ...)
{
    static char logBuffer[256];
    const char *logStr = "";

    switch (level) {
    case LOG_DEBUG:
        logStr = "[DEBUG] ";
        break;
    case LOG_INFO:
        logStr = "[INFO] ";
        break;
    case LOG_WARNING:
        logStr = "[WARNING] ";
        break;
    case LOG_ERROR:
        logStr = "[ERROR] ";
        break;
    default:
        break;
    }

    va_list args;
    va_start(args, fmt);

    // 生成日志前缀
    int32_t prefixLen = snprintf(logBuffer, sizeof(logBuffer), "%s", logStr);

    // 生成日志主体（预留1字节给换行符）
    int32_t available =
        sizeof(logBuffer) - prefixLen - 1; // -1 为换行符预留空间
    if (available < 0) {
        available = 0;
    }
    int32_t msgLen = vsnprintf(logBuffer + prefixLen, available, fmt, args);
    va_end(args);

    // 自动追加换行符并计算最终长度
    int32_t totalLen = prefixLen + msgLen;
    if (totalLen < sizeof(logBuffer) - 1) {
        logBuffer[totalLen] = '\n'; // 追加换行符
        totalLen++;
    } else {
        logBuffer[sizeof(logBuffer) - 1] = '\n'; // 缓冲区满时，覆盖最后一个字符
        totalLen = sizeof(logBuffer);
    }

    // 输出日志
    if (totalLen > 0) {
        UartPrint(logBuffer, totalLen);
    }
}