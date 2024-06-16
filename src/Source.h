//
// Created by yile0 on 2023/11/14.
//

#ifndef RESTREAMER_SOURCE_H
#define RESTREAMER_SOURCE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/avstring.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <list>
#include <mutex>
#include <string>
#include <utility>

#include "spdlog/spdlog.h"

enum MaterialType {
  kMaterial = 0,
  kAdditionalMaterial = 1,
};

enum ReadStatus {
  kUnStarted = 0,
  kStarting = 1,
  kReadEnd = 2,
  kClosed = 3,
};


class Source {
public:
  explicit Source(std::string url, int index,  double offset , int64_t  loop,int x, int y, int w, int h,
                  MaterialType materialType )
      : _url(url), _videoIndex(-1), _audioIndex(-1), _height(h), _width(w),
        _videoCodecCtx(nullptr), _audioCodecCtx(nullptr), _formatCtx(nullptr),
        _index(index), _x(x), _y(y), _status(kUnStarted),
        _materialType(materialType),_offset(uint64_t(offset)*1000*1000) ,_startVideoTs(0)
        ,_videoVal(0), _audioVal(0),_loop(loop) {}
  virtual ~Source() = default;

  int Open();
  int Shutdown();
  int GetFrame(AVFrame **frame, uint64_t &timestamp);
  AVFrame *GetVideoFrame(int64_t offset);
  AVFrame *GetAudioFrame();
  const AVCodecContext *GetAudioCodecContext() const;
  int Index() const { return _index; }
  std::string Name() const { return _name; }
  int X() const { return _x; }
  int Y() const { return _y; }
  int Width() const { return _width; }
  int Height() const { return _height; }

  AVRational VideoTimebase() const {
    return _formatCtx->streams[_videoIndex]->time_base;
  }
  AVRational AudioTimebase() const {
    return _formatCtx->streams[_audioIndex]->time_base;
  }

  std::string GetUrl() const { return _url; }

private:
  AVFrame *copyFrame(AVFrame *frame);
  AVFrame *scaleFrame(AVFrame *frame);
  int ReadFrame();

private:
  int _videoIndex;
  int _audioIndex;
  int _width;
  int _height;
  int _realWidth;
  int _realHeight;
  int _index;
  int _x;
  int _y;
  uint64_t _offset;
  uint64_t _startVideoTs;
  MaterialType _materialType;
  std::string _url;
  std::string _name;
  std::list<AVFrame *> _videoCaches;
  std::list<AVFrame *> _audioCaches;
  std::list<AVFrame *> _videoFrames;
  std::list<AVFrame *> _audioFrames;
  std::recursive_mutex _mutex;
  AVCodecContext *_videoCodecCtx;
  AVCodecContext *_audioCodecCtx;
  AVFormatContext *_formatCtx;
  AVFrame * _pitch;
  ReadStatus _status;
  SwsContext * _sws;
  std::vector<uint8_t > BG;
  uint64_t _videoVal;
  uint64_t _audioVal;
  uint64_t _loop;

};

#endif // RESTREAMER_SOURCE_H
