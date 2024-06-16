//
// Created by luzhi on 2023/11/13.
//

#ifndef RESTREAMER__SINK_H_
#define RESTREAMER__SINK_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <vector>

#include <spdlog/spdlog.h>

const uint64_t kVideoFps = 30;
const uint64_t kViodeGOP = 1 * 1000;
const uint64_t kVideoBitrate = 4 * 1024 * 1024;
const uint64_t kAudioBitrate = 128 * 1024;
const uint8_t kAudioChannels = 2;
const uint64_t kAudioSampleRate = 44100;

class Sink {
public:
  Sink(std::string url, int width, int height)
      : _url(url), _width(width), _height(height) {
    _isRunning = false;
    _lastVideoSendTs = 0;
    _lastAudioSendTs = 0;
    spdlog::info("create out ulr {}", url);
  }
  virtual ~Sink() { spdlog::info("desc out ulr {}", _url); }

  int Initialize();
  int Start();
  int Stop();
  int DeliveryVideo(AVPacket *pkt);
  int DeliveryAudio(AVPacket *pkt);

  AVCodecContext *InitializeAudio();
  AVCodecContext *InitializeVideo();
  int Reset();

private:
  int Push(AVPacket *pkt);

private:
  int64_t _lastVideoSendTs;
  int64_t _lastAudioSendTs;
  bool _isRunning;
  int _videoIndex;
  int _audioIndex;
  int _width;
  int _height;
  std::string _url;
  std::recursive_mutex _mutex;
  AVFormatContext *_outFormatCtx;
};

#endif // RESTREAMER__SINK_H_
