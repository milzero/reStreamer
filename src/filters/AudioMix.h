//
// Created by luzhi on 2023/11/14.
//

#ifndef RESTREAMER_SRC_AUDIOMIX_H_
#define RESTREAMER_SRC_AUDIOMIX_H_

#include <map>
#include <vector>

#include "Filter.h"
#include "MediaFilter.h"

class AudioMix  : public  MediaFilter {
 public:
  explicit AudioMix() {
    _graph = avfilter_graph_alloc();
    _buffersinkCtx = nullptr;
    _amixFilters = nullptr;
  }

  virtual ~AudioMix() = default;

  int AddFilter(Filter *filter);
  int ClearFilters();
  int GenerateRules();
  int AddFrames(std::map<int, AVFrame *> frames , int64_t ts);
  int GetFrames(AVFrame *frame);
 private:
  AVFilterContext *_amixFilters;
  AVFilterContext *_buffersinkCtx;
};

#endif  // RESTREAMER_SRC_AUDIOMIX_H_
