#include "VideoOverlay.h"
#include "../util/util.h"

int VideoOverlay::AddFilter(Filter *filter) {
  if (filter == nullptr) {
    return -1;
  }
  _filters[filter->index] = filter;
  return _filters.size();
}


int VideoOverlay::ClearFilters() {

  for (auto itr = _filters.begin(); itr != _filters.end(); itr++) {
    avfilter_free(itr->second->filterContext);
    delete itr->second;
  }
  _filters.clear();


  for (auto itr = _overlayFilters.begin(); itr != _overlayFilters.begin();
       itr++) {
    avfilter_free(*itr);
  }
  _overlayFilters.clear();
  avfilter_graph_free(&_graph);
  return 0;
}

int VideoOverlay::GenerateRules() {
    spdlog::info("starting generate overlay rules");
    if (_filters.empty()) {
    return -1;
  }

  int ret = 0;
  const AVFilter *overlay_filter = avfilter_get_by_name("overlay");
  auto itr = _filters.begin();
  AVFilterContext *lastFilter = itr->second->filterContext;
  itr++;
  for (; itr != _filters.end(); itr++) {
    AVFilterContext *overlay_ctx;
    std::string param =
        std::to_string(itr->second->x) + ":" + std::to_string(itr->second->y);
    ret = avfilter_graph_create_filter(&overlay_ctx, overlay_filter, "overlay",
                                       param.c_str(), nullptr, _graph);
    if (ret < 0) {
      spdlog::error("create overlay filter failed");
      break;
    }

    ret = avfilter_link(lastFilter, 0, overlay_ctx, 0);
    if (ret != 0) {
      spdlog::error("fail to link  filter's second pad and crop filter");
      return -1;
    }
    ret = avfilter_link(itr->second->filterContext, 0, overlay_ctx, 1);
    if (ret != 0) {
      spdlog::error("fail to link  filter's second pad and crop filter");
      return -1;
    }
    lastFilter = overlay_ctx;
    _overlayFilters.push_back(overlay_ctx);
  }


  const AVFilter *buffersink = avfilter_get_by_name("buffersink");
  ret = avfilter_graph_create_filter(&buffersink_ctx, buffersink, "out",
                                     nullptr, nullptr, _graph);
  if (ret != 0) {
    spdlog::error("open buffer sink failed, {}", av_err2str(ret));
  }
  ret = avfilter_link(lastFilter, 0, buffersink_ctx, 0);
  if (ret != 0) {
    spdlog::error("link buffer sink failed, {}", av_err2str(ret));
  }
  ret = avfilter_graph_config(_graph, nullptr);
  if (ret < 0) {
    spdlog::error("fail in filter graph");
    return -1;
  }

  char *str = avfilter_graph_dump(_graph, nullptr);
  spdlog::info("filter graph {}", str);

  return 0;
}

int VideoOverlay::AddFrames(std::map<int, AVFrame *> frames, int64_t ts) {
  if (frames.size() != _filters.size()) {
    return -1;
  }
  int ret = 0;
  for (auto itr = frames.begin(); itr != frames.end(); ++itr) {
    Filter *filterCtx = _filters[itr->first];
    if (!filterCtx) {
      spdlog::error("filter is null in filter graph index {}", itr->first);
      return -1;
    }
    AVFrame *frame = itr->second;
    frame->pkt_dts = ts;
    frame->pts = ts;
    ret = av_buffersrc_add_frame(filterCtx->filterContext, frame);
    if (ret < 0) {
      av_frame_unref(frame);
      av_frame_free(&frame);
      spdlog::error("add video frame failed {} ", av_err2str(ret));
      return -1;
    }
    av_frame_unref(frame);
    av_frame_free(&frame);
  }
  return 0;
}

int VideoOverlay::GetFrames(AVFrame *frame) {
  int ret = av_buffersink_get_frame(buffersink_ctx, frame);
  if (ret < 0) {
    spdlog::error("avcodec receive packet frame {}", av_err2str(ret));
    return -1;
  }
  return 0;
}
