#include "Engine.h"

#include "util/util.h"

void preciseSleep(double seconds) {
  using namespace std;
  using namespace std::chrono;

  static double estimate = 5e-3;
  static double mean = 5e-3;
  static double m2 = 0;
  static int64_t count = 1;

  while (seconds > estimate) {
    auto start = high_resolution_clock::now();
    this_thread::sleep_for(milliseconds(1));
    auto end = high_resolution_clock::now();

    double observed = (end - start).count() / 1e9;
    seconds -= observed;

    ++count;
    double delta = observed - mean;
    mean += delta / count;
    m2 += delta * (observed - mean);
    double stddev = sqrt(m2 / (count - 1));
    estimate = mean + stddev;
  }

  auto start = high_resolution_clock::now();
  while ((high_resolution_clock::now() - start).count() / 1e9 < seconds)
    ;
}

void Timer::setTimeout(auto function, int delay) {
  std::thread t([=]() {
    auto now = GetNowMs();
    double latency = delay / 1000.0;
    preciseSleep(latency);
    function();
  });
  t.detach();
}

void Timer::setInterval(auto function, int interval) {}

void Timer::stop() {}

int Engine::AddVideoSource(std::shared_ptr<Source> source) {
  if (!source) {
    spdlog::error("add ad null video source");
    return -1;
  }
  int ret = source->Open();
  if (ret != 0) {
    spdlog::error("open file %{} failed", source->Name().c_str());
    return ret;
  }

  if (_videoOverlay == nullptr) {
    _videoOverlay = new VideoOverlay();
  }
  auto filter = createVideoFilter(
      source->Name(), source->X(), source->Y(), source->Width(),
      source->Height(), source->Index(), _timebase, _videoOverlay->GetGraph());
  _videoSources[source->Index()] = source;
  _videoOverlay->AddFilter(filter);
  spdlog::info("add a video source  file {} , index {}", source->Name().c_str(),
               source->Index());
  return 0;
}

int Engine::Stop() {
  std::lock_guard<std::mutex> lck(_mtx);
  return 0;
}

int Engine::AddAudioSource(std::shared_ptr<Source> source) {
  if (!source) {
    spdlog::error("add ad null audio source");
    return -1;
  }
  int ret = source->Open();
  if (ret != 0) {
    spdlog::error("open file {} failed: {}", source->Name().c_str(),
                  av_err2str(ret));
    return ret;
  }
  _audioSources[source->Index()] = source;
  spdlog::info(" add a audio source  file {} , index {}",
               source->Name().c_str(), source->Index());

  if (_audioSources.empty()) {
    Filter *filter = createAudioFilter(
        source->Name(), source->X(), source->Y(), source->Index(),
        source->GetAudioCodecContext(), _audioMix->GetGraph());
    if (!filter) {
      spdlog::error(" createAudioFilter file {} failed",
                    source->Name().c_str());
      return -1;
    }

    if (_audioMix == nullptr) {
      _audioMix = new AudioMix();
    }
    _audioMix->AddFilter(filter);
    return 0;
  }

  return 0;
}

int Engine::Run() {
  std::lock_guard<std::mutex> lck(_mtx);
  int ret = 0;
  if (_task.empty()) {
    return -1;
  }

  if (_sink != nullptr) {
    spdlog::info("start out put");
    ret = _sink->Initialize();
    if (ret != 0) {
      spdlog::error("create  output failed");
      return ret;
    }

    _videoCodecCtx = _sink->InitializeVideo();
    if (_videoCodecCtx == nullptr) {
      spdlog::error("create video output encodec failed");
      return -1;
    }

    _audioCodecCtx = _sink->InitializeAudio();
    if (_audioCodecCtx == nullptr) {
      spdlog::error("create audio output encodec failed");
      return -1;
    }

    ret = _sink->Start();
    if (ret != 0) {
      spdlog::error("open output failed");
      return ret;
    }
  }

  _videoEncodeThread = std::thread(&Engine::videoEncodeFN, this);
  _audioEncodeThread = std::thread(&Engine::audioEncodeFN, this);
  return ret;
}

int Engine::ReadFrameFn() {
  while (_running.load() == 1) {
    std::lock_guard<std::mutex> lck(_mtx);
    AVFrame *frame = nullptr;
    uint64_t interval = 0;
    int ret = _mainSource->GetFrame(&frame, interval);

    if (ret == AVERROR_EOF) {
      spdlog::info("read end of {}", _mainSource->Name());
      ReBuildFilter();
      ReLoadSource();
      continue;
    }

    if (std::string((char *)frame->opaque) == "a") {
      audioReadFN(frame);
    } else if (std::string((char *)frame->opaque) == "v") {
      _offset = _offset + interval * 1000;
      _videoSendTs = _videoSendTs + interval * 1000;
      videoReadFN(frame, _offset);
    }
  }
  return 0;
}

int Engine::AudioConvert(AVFrame *pInFrame, AVFrame **OutFrame) {
  int64_t Samples =
      av_rescale_rnd(pInFrame->nb_samples, _audioCodecCtx->sample_rate,
                     pInFrame->sample_rate, AV_ROUND_UP);
  AVFrame *pOutFrame = av_frame_alloc();
  pOutFrame->format = _audioCodecCtx->sample_fmt;
  pOutFrame->nb_samples = (int)_audioCodecCtx->frame_size;
  pOutFrame->ch_layout = _audioCodecCtx->ch_layout;
  int res = av_frame_get_buffer(pOutFrame, 0);
  if (res < 0) {
    return res;
  }
  int nb_samples = swr_convert(
      _swrCtx, const_cast<uint8_t **>(pOutFrame->data), (int)Samples,
      const_cast<const uint8_t **>(pInFrame->data), pInFrame->nb_samples);
  if (nb_samples <= 0) {
    return nb_samples;
  }

  pOutFrame->nb_samples = nb_samples;
  pOutFrame->pts = pInFrame->pts;
  pOutFrame->pkt_dts = pInFrame->pkt_dts;

  av_frame_unref(pInFrame);
  av_frame_free(&pInFrame);
  (*OutFrame) = pOutFrame;
  return 0;
}

int Engine::AddVideoBaseSource(std::shared_ptr<Source> source) {
  int ret = source->Open();
  if (ret != 0) {
    spdlog::error("open file {} failed {} ", source->Name().c_str(),
                  av_err2str(ret));
    return ret;
  }

  if (_videoOverlay == nullptr) {
    _videoOverlay = new VideoOverlay();
  }
  auto filter = createVideoFilter(
      source->Name(), source->X(), source->Y(), source->Width(),
      source->Height(), source->Index(), _timebase, _videoOverlay->GetGraph());
  if (!filter) {
    spdlog::error("createVideoFilter file {} failed", source->Name().c_str());
    return -1;
  }

  _videoOverlay->AddFilter(filter);

  _baseSource = source;
  _videoSources[source->Index()] = source;

  if (!_sink) {
    _sink = new Sink(_outUrl, _baseSource->Width(), _baseSource->Height());
  }
  return 0;
}

int Engine::Start() {

  _videoSendTimer.setTimeout([&]() { SendVideoInterval(); },
                             kVideoSendInterval);

  _audioSendTimer.setTimeout([&]() { SendAudioInterval(); },
                             kAudioSendInterval);
  return 0;
}

int Engine::SendVideoInterval() {
  AVPacket *pkt = nullptr;
  uint64_t now = GetNowMs();
  {
    auto pkt = _videoPackets.pop();
    if (!pkt) {
      spdlog::error("get null pkt from buffer");
      return 0;
    }
    _sink->DeliveryVideo(pkt);
    _videoSendTimer.setTimeout([&]() { SendVideoInterval(); },
                               kVideoSendInterval);
    _lastVideoTs = now;
    return 0;
  }
}

int Engine::SendAudioInterval() {
  uint64_t now = GetNowMs();
  auto pkt = _audioPackets.pop();
  if (!pkt) {
    spdlog::error("get null pkt from buffer");
    return 0;
  }

  if (!pkt) {
    return 0;
  }
  _sink->DeliveryAudio(pkt);

  int took = 25;
  if (_lastVideoTs > 0) {
    took = GetNowMs() - _lastAudioTs;
  }

  int var = 1;
  if (took > kAudioSendInterval) {
    var = took - kAudioSendInterval;
  }
  int next = kAudioSendInterval - var;
  spdlog::info("send audio pkg , pts:{} , took {}", pkt->pts / 1000, next);

  _audioSendTimer.setTimeout([&]() { SendAudioInterval(); },
                             kAudioSendInterval);
  _lastAudioTs = now;
  return 0;
}

int Engine::AddSamplesToFifo(AVAudioFifo *fifo,
                             uint8_t **converted_input_samples,
                             const int frame_size) {
  int error = 0;

  if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) +
                                               frame_size)) < 0) {
    spdlog::error("could not reallocate fifo :{}", av_err2str(error));
    return error;
  }

  if (av_audio_fifo_write(fifo, (void **)converted_input_samples, frame_size) <
      frame_size) {
    spdlog::error("could not write fifo");

    return AVERROR_EXIT;
  }
  return 0;
}

int Engine::EncodeAudio(AVFrame *frame) {
  if (_swrCtx == nullptr) {
    int ret = InitAudioSwr(frame);
    if (ret < 0) {
      return ret;
    }
    _audioFifo = av_audio_fifo_alloc(_audioCodecCtx->sample_fmt,
                                     _audioCodecCtx->ch_layout.nb_channels,
                                     kSamplesPerFrame);
  }

  AVFrame *newFrame = av_frame_alloc();
  int ret = AudioConvert(frame, &newFrame);
  if (ret < 0) {
    return ret;
  }
  AddSamplesToFifo(_audioFifo, newFrame->data, newFrame->nb_samples);
  if (av_audio_fifo_size(_audioFifo) < _audioCodecCtx->frame_size) {
    return 0;
  }

  av_frame_unref(newFrame);
  av_frame_free(&newFrame);

  AVFrame *pOutFrame = av_frame_alloc();
  pOutFrame->format = _audioCodecCtx->sample_fmt;
  pOutFrame->nb_samples = _audioCodecCtx->frame_size;
  pOutFrame->ch_layout = _audioCodecCtx->ch_layout;
  int res = av_frame_get_buffer(pOutFrame, 0);
  if (res < 0) {
    return res;
  }
  if (av_audio_fifo_read(_audioFifo, (void **)pOutFrame->data, 1024) < 1024) {
    return 0;
  }

  _audioSendTs = _audioSendTs + 23100;
  pOutFrame->pts = _audioSendTs;
  pOutFrame->pkt_dts = _audioSendTs;

  ret = avcodec_send_frame(_audioCodecCtx, pOutFrame);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    }
    spdlog::error("avcodec send frame {}", av_err2str(ret));
    return ret;
  }

  AVPacket *packet = av_packet_alloc();
  ret = avcodec_receive_packet(_audioCodecCtx, packet);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    }
    spdlog::error("avcodec receive packet frame {} ", av_err2str(ret));
    return ret;
  }

  { _audioPackets.push(packet); }

  av_frame_unref(pOutFrame);
  av_frame_free(&pOutFrame);
  spdlog::debug("size of audio packet list: {} ", _audioFrames.size());
  return 0;
}

int Engine::EncodeVideo(AVFrame *frame) {
  frame->pict_type = AV_PICTURE_TYPE_NONE;
  int ret = avcodec_send_frame(_videoCodecCtx, frame);
  if (ret < 0) {
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return 0;
    }
    spdlog::info("avcodec send frame: {} ", av_err2str(ret));
    return ret;
  }

  while (true) {
    AVPacket *packet = av_packet_alloc();
    ret = avcodec_receive_packet(_videoCodecCtx, packet);
    if (ret < 0) {
      if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
        ret = 0;
        break;
      }
      spdlog::error("avcodec receive packet frame {}", av_err2str(ret));
      break;
    }

    {
      _videoPackets.push(packet);
      if (_startPush.load() == 0 && _videoPackets.size() > 75) {
        Start();
        _startPush.store(1);
      }
      spdlog::info("put pkt in video buffer , size:{}", _videoPackets.size());
    }
  }
  av_frame_unref(frame);
  av_frame_free(&frame);
  return 0;
}

int Engine::Reset() {
  _videoSendTimer.stop();
  _videoPackets.clear();
  _audioPackets.clear();

  return 0;
}

int Engine::videoReadFN(AVFrame *frame, int64_t offset) {
  int ret = 0;
  spdlog::info("~~~~~~~~~~~~~~~~~~encode  frame to "
               "overlay~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  std::map<int, AVFrame *> frames;
  for (auto itr = this->_videoSources.begin(); itr != _videoSources.end();
       itr++) {
    auto source = itr->second;
    if (source->Name() == _mainSource->Name()) {
      continue;
    }
    spdlog::info("start get frame source {}", source->Name());
    auto nowTs = GetNowMs();
    auto frame1 = source->GetVideoFrame(offset);
    if (!frame1) {
      spdlog::error("get video frame from {} failed", source->Name());
    }
    spdlog::info("{} get video frame took {} ms ", source->Name(),
                 GetNowMs() - nowTs);
    frames[itr->first] = frame1;
  }

  frames[_mainSource->Index()] = frame;

  spdlog::info("~~~~~~~~~~~~~~~~~~add  frame to "
               "overlay~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

  _videoOverlay->AddFrames(frames, _videoSendTs);
  AVFrame *outFrame = av_frame_alloc();
  ret = _videoOverlay->GetFrames(outFrame);
  spdlog::info("!!!!!!!!!!!!!!!!!!!!!!!!!!get  frame to "
               "overlay!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  if (ret != 0) {
    spdlog::error("get frame failed {}", av_err2str(ret));
  }

  {
    std::unique_lock<std::mutex> lck(_videoReadMtx);
    while (this->_videoFrames.size() > kAudioFrameLimit) {
      this->_videoReadFull.wait(lck);
    }
  }

  {
    std::unique_lock<std::mutex> lck(_videoReadMtx);
    _videoFrames.push_back(outFrame);
    _videoReadEmpty.notify_all();
  }

  return 0;
}

int Engine::audioReadFN(AVFrame *frame) {
  int ret = 0;
  std::map<int, AVFrame *> frames;
  AVFrame *outFrame = nullptr;
  if (_audioSources.size() > 1) {
    for (auto itr = _audioSources.begin(); itr != _audioSources.end(); itr++) {
      auto source = itr->second;
      if (source->Name() == _mainSource->Name()) {
        continue;
      }
      AVFrame *frame1 = source->GetAudioFrame();
      if (!frame1) {
        spdlog::error("[{}] get audio frame from {} failed",
                      itr->second->Name().c_str(), source->Name().c_str());
      }
      spdlog::info("{} new frame pts: {}  ", source->Name().c_str(),
                   frame1->pts);
      frames[itr->first] = frame1;
    }
    frames[_mainSource->Index()] = frame;
    _audioMix->AddFrames(frames, _audioSendTs);
    outFrame = av_frame_alloc();
    ret = _audioMix->GetFrames(outFrame);
    if (ret != 0) {
      return ret;
    }
  } else {
    outFrame = frame;
  }

  {
    std::unique_lock<std::mutex> lck(_audioReadMtx);
    while (_audioFrames.size() > kVideoFrameLimit) {
      _audioReadFull.wait(lck);
    }
  }

  {
    std::unique_lock<std::mutex> lck(_audioReadMtx);
    _audioFrames.push_back(outFrame);
    spdlog::info("size of audio list: {} ", _audioFrames.size());
    _audioReadEmpty.notify_all();
  }
  return 0;
}

void Engine::videoEncodeFN() {
  while (true) {
    AVFrame *frame = nullptr;
    {
      std::unique_lock<std::mutex> lck(_videoReadMtx);
      while (_videoFrames.empty()) {
        _videoReadEmpty.wait(lck);
      }
      frame = _videoFrames.front();
      _videoFrames.pop_front();
      _videoReadFull.notify_all();
    }

    auto pts = frame->pts;
    auto now = GetNowMs();
    spdlog::info("put frame in  encode pts:{}", pts / 1000);
    AVFrame *outFrame = av_frame_alloc();
    av_frame_copy_props(outFrame, frame);
    outFrame->width = frame->width;
    outFrame->height = frame->height;
    outFrame->format = AV_PIX_FMT_YUV420P;
    av_frame_get_buffer(outFrame, 1);
    if (_sws == nullptr) {
      _sws = sws_alloc_context();
      _sws = sws_getContext(_videoCodecCtx->width, _videoCodecCtx->height,
                            static_cast<AVPixelFormat>(frame->format),
                            _videoCodecCtx->width, _videoCodecCtx->height,
                            AV_PIX_FMT_YUV420P, 0, NULL, NULL, NULL);
    }
    int outHeight =
        sws_scale(_sws, frame->data, frame->linesize, 0, frame->height,
                  outFrame->data, outFrame->linesize);

    av_frame_unref(frame);
    av_frame_free(&frame);

    EncodeVideo(outFrame);
    spdlog::info("video encode cost {} , pts:{}", GetNowMs() - now, pts / 1000);
  }
}
void Engine::audioEncodeFN() {
  while (true) {
    AVFrame *frame = nullptr;
    {
      std::unique_lock<std::mutex> lck(_audioReadMtx);
      while (_audioFrames.empty()) {
        _audioReadEmpty.wait(lck);
      }
      frame = _audioFrames.front();
      _audioFrames.pop_front();
      _audioReadFull.notify_all();
    }
    auto pts = frame->pts;
    auto now = GetNowMs();
    EncodeAudio(frame);
    spdlog::info("audio encode cost {} , pts {}", GetNowMs() - now, pts);
  }
}

int Engine::InitAudioSwr(AVFrame *Frame) {
  int ret = 0;
  _swrCtx = swr_alloc();
  if (!_swrCtx) {
    spdlog::error("failed to alloc the resampling context");
    return -1;
  }

  ret = swr_alloc_set_opts2(
      &_swrCtx, &(_audioCodecCtx->ch_layout), _audioCodecCtx->sample_fmt,
      _audioCodecCtx->sample_rate, &(Frame->ch_layout),
      (AVSampleFormat)Frame->format, Frame->sample_rate, 0, nullptr);

  if ((ret = swr_init(_swrCtx)) < 0) {
    spdlog::error("failed to initialize the resampling context");
    return ret;
  }
  return 0;
}

int Engine::AddMainSource(std::shared_ptr<Source> source) {
  int ret = source->Open();
  if (ret != 0) {
    spdlog::error("open file {} failed {} ", source->Name().c_str(),
                  av_err2str(ret));
    return ret;
  }

  if (_videoOverlay == nullptr) {
    _videoOverlay = new VideoOverlay();
  }
  auto videoFilter = createVideoFilter(
      source->Name(), source->X(), source->Y(), source->Width(),
      source->Height(), source->Index(), _timebase, _videoOverlay->GetGraph());
  if (!videoFilter) {
    spdlog::error("createVideoFilter file {} failed", source->Name().c_str());
    return -1;
  }
  _videoOverlay->AddFilter(videoFilter);

  _mainSource = source;

  _videoSources[source->Index()] = source;
  _audioSources[source->Index()] = source;

  if (_sink == nullptr) {
    _sink = new Sink(_outUrl, _baseSource->Width(), _baseSource->Height());
  }

  return 0;
}

void Engine::UpdateTasks(std::list<Task *> tasks) {
  _running.store(0);
  std::lock_guard<std::mutex> lck(_mtx);
  tasks.sort([](Task *a, Task *b) { return a->_index < b->_index; });
  _task = tasks;
  ReBuildFilter();
  ReLoadSource();
  _running.store(1);
  _readThread = std::thread(&Engine::ReadFrameFn, this);
  _readThread.detach();
}

int Engine::ClearSource() {

  if (_mainSource != nullptr) {
    _mainSource->Shutdown();
  }

  if (_baseSource != nullptr) {
    _baseSource->Shutdown();
  }

  for (auto i = _audioSources.begin(); i != _audioSources.end(); ++i) {
    i->second->Shutdown();
  }
  _audioSources.clear();

  for (auto i = _videoSources.begin(); i != _videoSources.end(); ++i) {
    i->second->Shutdown();
  }
  _videoSources.clear();

  return 0;
}
void Engine::ReLoadSource() {
  ClearSource();

  Task *task = _task.front();
  _task.pop_front();
  _task.push_back(task);

  AddVideoBaseSource(task->_baseSource);
  AddMainSource(task->_mainSource);

  for (const auto &_audioSource : task->_audioSources) {
    AddAudioSource(_audioSource);
  }

  for (auto &videoSource : task->_videoSources) {
    AddVideoSource(videoSource);
  }

  if (CreateAudioFilter() == 0) {
    if (_audioMix != nullptr) {
      _audioMix->GenerateRules();
    }
  }

  _videoOverlay->GenerateRules();

  _offset = 0;
}

void Engine::ReBuildFilter() {
  if (_videoOverlay != nullptr) {
    _videoOverlay->ClearFilters();
    delete _videoOverlay;
    _videoOverlay = nullptr;
  }

  if (_audioMix != nullptr) {
    _audioMix->ClearFilters();
    delete _audioMix;
    _audioMix = nullptr;
  }
}

int Engine::CreateAudioFilter() {
  if (_audioSources.size() < 2) {
    return 0;
  }

  if (_audioMix == nullptr) {
    _audioMix = new AudioMix();
  }
  for (auto i = _audioSources.begin(); i != _audioSources.end(); ++i) {
    auto source = i->second;
    Filter *filter = createAudioFilter(
        source->Name(), source->X(), source->Y(), source->Index(),
        source->GetAudioCodecContext(), _audioMix->GetGraph());
    if (!filter) {
      spdlog::error(" createAudioFilter file {} failed",
                    source->Name().c_str());
      return -1;
    }
    _audioMix->AddFilter(filter);
  }
  return 0;
}
