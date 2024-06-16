#include <iostream>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "Engine.h"
#include "http/Server.h"
#include "log/Log.h"
#include "service/Service.h"
#include "config/Config.h"

const char *kCreateTask = "/create_task";
const char *kUpdateTask = "/update_task";
const char *kDeleteTask = "/delete_task";
const char *kQueryTask = "/query_task";
const char *kCloseTask = "/close_task";

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::debug);
  spdlog::set_pattern("[%H:%M:%S.%f %z] [%l] [%P] [%t] %v");
  // av_log_set_callback(log_cb);
  loadConifg("config.json");
  try {
    auto logger = spdlog::basic_logger_mt("basic_logger", "logs/basic.log");
    spdlog::set_default_logger(logger);
  } catch (const spdlog::spdlog_ex &ex) {
    std::cout << "Log init failed: " << ex.what() << std::endl;
  }

  av_log_set_level(AV_LOG_ERROR);

  HttpServer server;
  server.AddHandler(kCreateTask, handle_create_task);
  server.AddHandler(kUpdateTask, handle_update_task);
  server.AddHandler(kDeleteTask, handle_delete_task);
  server.AddHandler(kCloseTask, handle_close_task);
  server.AddHandler(kQueryTask, handle_query_task);
  spdlog::info("listen on {}", 8000);
  server.Init("8000");

  GetPic();
  server.Start();
  return 0;
}