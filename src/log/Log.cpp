//
// Created by yile0 on 2023/12/7.
//

#include "Log.h"

void log_cb(void *avcl, int level, const char *fmt, va_list vl) {
    static int printPrefix = 1;
    static constexpr size_t lineSize = 1024;
    char line[lineSize];
    char newFmt[128] = {0};
    memcpy(newFmt, fmt, strlen(fmt) - 1);
    av_log_format_line2(avcl, level, newFmt, vl, line, lineSize, &printPrefix);
    switch (level) {
        case AV_LOG_DEBUG:
            spdlog::debug(line);
            break;
        case AV_LOG_VERBOSE:
            spdlog::debug(line);
            break;
        case AV_LOG_INFO:
            spdlog::info(line);
            break;
        case AV_LOG_WARNING:
            spdlog::warn(line);
            break;
        case AV_LOG_ERROR:
            spdlog::error(line);
            break;
    }
}
