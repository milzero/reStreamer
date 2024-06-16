//
// Created by luzhi on 2023/11/17.
//

#ifndef RESTREAMER_SRC_FILTER_H_
#define RESTREAMER_SRC_FILTER_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#include <string>

#include "spdlog/spdlog.h"

struct Filter {
  const AVFilter *fileter;
  int x;
  int y;
  int index;
  std::string name;
  AVRational timebase;
  AVFilterGraph *filterGraph;
  AVFilterContext * filterContext;
};

Filter *createVideoFilter(std::string name,
                          int x,
                          int y,
                          int w,
                          int h,
                          int index,
                          AVRational time_base,
                          AVFilterGraph *filterGraph);

Filter *createAudioFilter(std::string name, int x, int y, int index,
                          const AVCodecContext *CodecContext,
                          AVFilterGraph *filterGraph);

#endif  // RESTREAMER_SRC_FILTER_H_
