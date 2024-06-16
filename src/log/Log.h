//
// Created by yile0 on 2023/12/7.
//

#ifndef RESTREAMER_LOG_H
#define RESTREAMER_LOG_H

extern "C"{
#include <libavutil/log.h>
};

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

void log_cb(void *avcl, int level, const char *fmt, va_list vl);

#endif //RESTREAMER_LOG_H
