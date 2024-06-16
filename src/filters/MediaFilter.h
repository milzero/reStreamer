//
// Created by yile0 on 2023/12/10.
//

#ifndef RESTREAMER_MEDIAFILTER_H
#define RESTREAMER_MEDIAFILTER_H

#include <map>
#include "Filter.h"

class MediaFilter {
public:

   MediaFilter() = default;
   virtual ~MediaFilter() = default;

    virtual int AddFilter(Filter *filter) = 0;
    virtual int ClearFilters() = 0;
    virtual int GenerateRules() = 0;
    virtual int AddFrames(std::map<int, AVFrame *> frames , int64_t ts) = 0;
    virtual int GetFrames(AVFrame *frame) = 0;
    virtual AVFilterGraph *GetGraph(){
        return _graph;
    };

protected:
    std::map<int, Filter *> _filters;
    AVFilterGraph *_graph;
};

#endif //RESTREAMER_MEDIAFILTER_H
