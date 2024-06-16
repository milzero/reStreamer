//
// Created by yile0 on 2023/11/14.
//

#ifndef RESTREAMER_ENGINE_H
#define RESTREAMER_ENGINE_H

extern "C" {
#include <libavutil/audio_fifo.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

#include <condition_variable>
#include <list>
#include <map>
#include <mutex>
#include <thread>
#include <memory>

#include "Source.h"
#include "filters/AudioMix.h"
#include "filters/Filter.h"
#include "filters/VideoOverlay.h"
#include "log/Log.h"
#include "service/Task.h"
#include "sink/Sink.h"
#include "spdlog/spdlog.h"
#include "util/BlockingQueue.h"

class Timer {

public:
  void setTimeout(auto function, int delay);
  void setInterval(auto function, int interval);
  void stop();
};

const uint64_t kVideoSendInterval = 40;
const uint64_t kAudioSendInterval = 23;

const uint64_t kAudioPacketLimit = 30 * 3;
const uint64_t kVideoPacketLimit = 30;

const uint64_t kAudioFrameLimit = 30 * 3;
const uint64_t kVideoFrameLimit = 30 * 3;
const int kSamplesPerFrame = 1024;

class Engine {
public:
  Engine() {
    _outUrl = "";
    _videoOverlay = nullptr;
    _audioMix = nullptr;
    _sink = nullptr;
    _videoCodecCtx = nullptr;
    _audioCodecCtx = nullptr;
    _videoSendTs = 0;
    _audioSendTs = 0;
    _mainSource = nullptr;
    _baseSource = nullptr;
    _timebase = {1, AV_TIME_BASE};
    _offset = 0;
  }

  static Engine &GetEngine() {
    static Engine engine;
    return engine;
  }

  void SetUrl(std::string url) { _outUrl = url; }

  void SetCallBack(std::string url) { _callback = url; }
  virtual ~Engine() = default;

public:
  void UpdateTasks(std::list<Task *> tasks);
  int Run();
  int Stop();
  int ClearSource();

private:
  int AddVideoBaseSource(std::shared_ptr<Source>source);
  int AddMainSource(std::shared_ptr<Source>source);
  int AddVideoSource(std::shared_ptr<Source>source);
  int AddAudioSource(std::shared_ptr<Source>source);
  int EncodeAudio(AVFrame *frame);
  int EncodeVideo(AVFrame *frame);
  int SendVideoInterval();
  int SendAudioInterval();
  int Reset();
  int Start();
  int CreateAudioFilter();
  int ReadAuidoFrameFn();
  int ReadVideoFrameFn();
  int AudioConvert(AVFrame *Frame, AVFrame **OutFrame);
  int InitAudioSwr(AVFrame *Frame);
  static int AddSamplesToFifo(AVAudioFifo *fifo,
                              uint8_t **converted_input_samples,
                              const int frame_size);

  void ReLoadSource();
  void ReBuildFilter();
  int ReadFrameFn();

protected:
  std::thread _readVideoThread;
  std::thread _readAudioThread;
  std::thread _videoEncodeThread;
  std::thread _audioEncodeThread;

  std::thread _readThread;

  std::mutex _videoReadMtx;
  std::condition_variable _videoReadFull;
  std::condition_variable _videoReadEmpty;

  std::mutex _audioReadMtx;
  std::condition_variable _audioReadFull;
  std::condition_variable _audioReadEmpty;

  std::mutex _videoEncodeMtx;
  std::condition_variable _videoEncodeEmpty;

  std::mutex _audioEncodeMtx;
  std::condition_variable _audioEncodeEmpty;
  int videoReadFN(AVFrame *frame, int64_t offset);
  int audioReadFN(AVFrame *frame);
  void videoEncodeFN();
  void audioEncodeFN();

private:
  std::string _outUrl;
  std::string _callback;
  Sink *_sink;

  std::list<Task *> _task;

  AVRational _timebase;
  std::shared_ptr<Source> _baseSource;
  std::shared_ptr<Source>_mainSource;
  VideoOverlay *_videoOverlay;
  AudioMix *_audioMix;

  std::map<int, std::shared_ptr<Source>> _audioSources;
  std::map<int, std::shared_ptr<Source>> _videoSources;

  BlockingQueue<AVPacket *> _videoPackets;
  BlockingQueue<AVPacket *> _audioPackets;

  std::list<AVFrame *> _videoFrames;
  std::list<AVFrame *> _audioFrames;

  AVCodecContext *_videoCodecCtx;
  AVCodecContext *_audioCodecCtx;
  SwrContext *_swrCtx = nullptr;
  AVAudioFifo *_audioFifo = nullptr;
  Timer _videoSendTimer;
  Timer _audioSendTimer;
  int64_t _videoSendTs;
  int64_t _audioSendTs;
  int64_t _offset;
  SwsContext * _sws;
  uint64_t _lastVideoTs = 0;
  uint64_t _lastAudioTs = 0;
  std::mutex _mtx;
  std::atomic<int> _running;
  std::atomic<int> _startPush;
};

#endif // RESTREAMER_ENGINE_H
