//
// Created by luzhi on 2023/11/17.
//

#ifndef RESTREAMER_SRC_VIDEOOVERLAY_H_
#define RESTREAMER_SRC_VIDEOOVERLAY_H_

#include <map>
#include <vector>

#include "MediaFilter.h"

class VideoOverlay : public  MediaFilter{
 public:
  VideoOverlay() {
    _graph = avfilter_graph_alloc();
    buffersink_ctx = nullptr;
  }
  virtual ~VideoOverlay() = default;

  int AddFilter(Filter *filter);
  int ClearFilters();
  int GenerateRules();
  int AddFrames(std::map<int, AVFrame *> frames, int64_t ts);
  int GetFrames(AVFrame *frame);

 private:
  std::vector<AVFilterContext *> _overlayFilters;
  AVFilterContext *buffersink_ctx;
};

#endif  // RESTREAMER_SRC_VIDEOOVERLAY_H_
