//
// Created by luzhi on 2023/11/17.
//

#include "Filter.h"
#include "../log/Log.h"
#include "../util/util.h"
Filter *createVideoFilter(std::string name,
                          int x,
                          int y,
                          int w,
                          int h,
                          int index,
                          AVRational time_base,
                          AVFilterGraph *filterGraph) {
  std::string filterIn = name + "In";
  const AVFilter *avfilter = avfilter_get_by_name("buffer");
  AVFilterContext *buffersrc_ctx = nullptr;
  char args[512] = {0};
    sprintf(args,
          "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
          w, h, AV_PIX_FMT_ARGB,
          time_base.num, time_base.den,
          0,
          1);
  spdlog::info("create buffer source argc: {}", args);
  int ret = avfilter_graph_create_filter(
      &buffersrc_ctx, avfilter, filterIn.c_str(), args,
      NULL, filterGraph);
  if (ret < 0) {
    spdlog::error("Cannot create buffer source: {}", av_err2str(ret));
    return nullptr;
  }

  Filter *filter = new Filter();
  filter->name = name;
  filter->x = x;
  filter->y = y;
  filter->index = index;
  filter->timebase = time_base;
  filter->filterContext = buffersrc_ctx;
  return filter;
}

Filter *createAudioFilter(std::string name, int x, int y, int index,
                          const AVCodecContext *CodecContext,
                          AVFilterGraph *filterGraph) {
  std::string filterIn = name + "In";
  const AVFilter *avfilter = avfilter_get_by_name("abuffer");
  AVFilterContext *buffersrc_ctx = nullptr;

  if (!avfilter) {
    spdlog::error("Could not find the abuffer filter.");
    return nullptr;
  }

  char buf[64];
  av_channel_layout_describe(&CodecContext->ch_layout, buf, sizeof(buf));

  char args[512];
  snprintf(args, sizeof(args),
             "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%s",
             CodecContext->pkt_timebase.num, CodecContext->pkt_timebase.den, CodecContext->sample_rate,
             av_get_sample_fmt_name(CodecContext->sample_fmt),
             buf);


  int ret = 0;
  ret = avfilter_graph_create_filter(&buffersrc_ctx, avfilter, filterIn.c_str(), args,
                                     NULL, filterGraph);
  if (ret < 0) {
    spdlog::error("Cannot create audio buffer source");
    return nullptr;
  }

  Filter *filter = new Filter();
  filter->name = name;
  filter->filterContext = buffersrc_ctx;
  filter->filterGraph = filterGraph;
  filter->index = index;
  filter->name = name;
  return filter;
}
