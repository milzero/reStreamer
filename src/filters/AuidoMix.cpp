//
// Created by luzhi on 2023/11/14.
//

#include "AudioMix.h"
#include "../log/Log.h"
#include "../util/util.h"


int AudioMix::GenerateRules() {
  std::string args;
  auto mixFilter = avfilter_get_by_name("amix");
  if (!mixFilter) {
    spdlog::error("Could not find the mix filter.");
    return AVERROR_FILTER_NOT_FOUND;
  }

  int inputs = _filters.size();
  args = "inputs=" + std::to_string(inputs);
  int ret = avfilter_graph_create_filter(&_amixFilters, mixFilter, "amix",
                                         args.c_str(), NULL, _graph);

  int i = 0;
  for (auto itr = _filters.begin(); itr != _filters.end(); itr++) {
    ret = avfilter_link(itr->second->filterContext, 0, _amixFilters, i++);
    if (ret != 0) {
      spdlog::error("fail to link  filter's second pad and crop filter");
      return -1;
    }
  }

  const AVFilter *abuffersink = avfilter_get_by_name("abuffersink");
  ret = avfilter_graph_create_filter(&_buffersinkCtx, abuffersink, "out",
                                     nullptr, nullptr, _graph);
  if (ret != 0) {
    spdlog::error("open buffer sink failed, {}",
           av_err2str(ret));
  }

  ret = avfilter_link(_amixFilters, 0, _buffersinkCtx, 0);
  if (ret != 0) {
    spdlog::error("open buffer sink failed, {}",
           av_err2str(ret));
  }

  ret = avfilter_graph_config(_graph, nullptr);
  if (ret < 0) {
    spdlog::error("fail in filter graph");
    return -1;
  }
  char *str = avfilter_graph_dump(_graph, nullptr);
  spdlog::info("filter graph {}", str);
  if (ret != 0) {
    spdlog::error("open amix filter failed: {}",
           av_err2str(ret));
    return -1;
  }
  return 0;
}

int AudioMix::AddFilter(Filter *filter) {
  if (filter == nullptr) {
    return -1;
  }
  _filters[filter->index] = filter;
  return _filters.size();
}


int AudioMix::ClearFilters() {
  for (auto itr = _filters.begin(); itr != _filters.end(); itr++) {
    avfilter_free(itr->second->filterContext);
  }
  _filters.clear();
  avfilter_free(_amixFilters);
  avfilter_graph_free(&_graph);
  return 0;
}

int AudioMix::AddFrames(std::map<int, AVFrame *> frames , int64_t ts) {
  if (frames.size() != _filters.size()) {
    return -1;
  }
  int ret = 0;
  for (auto itr = frames.begin(); itr != frames.end(); ++itr) {
    Filter *filterCtx = _filters[itr->first];
    if (!filterCtx) {
      return -1;
    }
    AVFrame *frame = itr->second;
    ret = av_buffersrc_add_frame(filterCtx->filterContext, itr->second);
    if (ret < 0) {
        av_frame_unref(frame);

      spdlog::error("add audio frame failed {} ", av_err2str(ret));
      return -1;
    }

      av_frame_free(&frame);
  }
  return 0;
}

int AudioMix::GetFrames(AVFrame *frame) {
  int ret = av_buffersink_get_frame(_buffersinkCtx, frame);
  if (ret < 0) {
    spdlog::error("avcodec receive packet frame {}",
           av_err2str(ret));
    return -1;
  }
}
