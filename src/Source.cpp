
#include "Source.h"

#include "spdlog/spdlog.h"
#include "util/util.h"

#define kVideoDefaultInterVal 30
#define kAudioDefaultInterVal 20

int Source::Open() {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  if (_status != ReadStatus::kUnStarted) {
    return 0;
  }

  int ret = 0;
  std::string ext = getFileExt(_url);
  const AVInputFormat *inputFormat = av_find_input_format(ext.c_str());
  ret = avformat_open_input(&_formatCtx, _url.c_str(), nullptr, nullptr);
  if (ret != 0) {
    spdlog::error("open file {} failed , err: {}", _url.c_str(),
                  av_err2str(ret));
    return ret;
  }

  _name = av_basename(_formatCtx->url);
  ret = avformat_find_stream_info(_formatCtx, nullptr);
  for (int i = 0; i < _formatCtx->nb_streams; ++i) {
    if (_formatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
      spdlog::info("find video in  file  {}", _url.c_str());
      _videoIndex = i;
    } else if (_formatCtx->streams[i]->codecpar->codec_type ==
               AVMEDIA_TYPE_AUDIO) {
      spdlog::info("find audio in  file  {}", _url.c_str());
      _audioIndex = i;
    }
  }

  if (_videoIndex >= 0) {
    AVStream *video_stream = _formatCtx->streams[_videoIndex];
    const AVCodec *codec = nullptr;
    switch (video_stream->codecpar->codec_id) {
    case AV_CODEC_ID_VP9:
      codec = avcodec_find_decoder_by_name("libvpx-vp9");
      break;
    case AV_CODEC_ID_VP8:
      codec = avcodec_find_decoder_by_name("libvpx");
      break;
    default:
      codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
      break;
    }
    _videoCodecCtx = avcodec_alloc_context3(codec);
    ret = avcodec_parameters_to_context(_videoCodecCtx, video_stream->codecpar);
    _realWidth = video_stream->codecpar->width;
    _realHeight = video_stream->codecpar->height;
    if (ret != 0) {
      spdlog::error("cant find stream in file {} failed , err: {}",
                    _url.c_str(), av_err2str(ret));
    }
    if (_videoCodecCtx->codec_id == AV_CODEC_ID_VP9) {
      _videoCodecCtx->pix_fmt = AV_PIX_FMT_YUVA420P;
    }

    _sws = sws_alloc_context();
    _sws =
        sws_getContext(_realWidth, _realHeight, _videoCodecCtx->pix_fmt, _width,
                       _height, AV_PIX_FMT_ARGB, 0, NULL, NULL, NULL);

    ret = avcodec_open2(_videoCodecCtx, codec, nullptr);
    if (ret != 0) {
      spdlog::error("cant find stream in file {} failed , err: {}",
                    _url.c_str(), av_err2str(ret));
    }
  } else {
    spdlog::info("cant find video stream in file {} failed , err: {}",
                 _url.c_str(), av_err2str(ret));
  }

  if (_audioIndex >= 0) {
    AVStream *audio_stream = _formatCtx->streams[_audioIndex];
    const AVCodec *codec = nullptr;
    switch (audio_stream->codecpar->codec_id) {
    default:
      codec = avcodec_find_decoder(audio_stream->codecpar->codec_id);
      break;
    }
    if (!codec) {
      spdlog::error("can not  find codec {}",
                    (int)audio_stream->codecpar->codec_id);
    }

    _audioCodecCtx = avcodec_alloc_context3(codec);
    ret = avcodec_parameters_to_context(_audioCodecCtx, audio_stream->codecpar);
    if (ret != 0) {
      spdlog::error("cant find stream in file {} failed , err: {}",
                    _url.c_str(), av_err2str(ret));
    }
    ret = avcodec_open2(_audioCodecCtx, codec, nullptr);
    if (ret != 0) {
      spdlog::error("cant find stream in file {} failed , err: {}",
                    _url.c_str(), av_err2str(ret));
    }
  } else {
    spdlog::info("cant find audio stream in file {} failed ,  {}", _url.c_str(),
                 av_err2str(ret));
  }
  _status = ReadStatus::kStarting;

  //    if (_offset > 0) {
  _pitch = av_frame_alloc();
  _pitch->format = AV_PIX_FMT_ARGB;
  _pitch->width = _width;
  _pitch->height = _height;
  av_frame_get_buffer(_pitch, 1);
  AVFrame *src = GetPic();
  AVFrame *copy = copyFrame(src);
  SwsContext *sws = sws_alloc_context();
  sws =
      sws_getContext(copy->width, copy->height, AV_PIX_FMT_RGBA, _pitch->width,
                     _pitch->height, AV_PIX_FMT_ARGB, 0, NULL, NULL, NULL);

  int outHeight = sws_scale(sws, copy->data, copy->linesize, 0, copy->height,
                            _pitch->data, _pitch->linesize);
  if (ret != 0) {
    spdlog::error("cant find stream in file {} failed , err: {}", _url.c_str(),
                  av_err2str(ret));
  }
  //    }
  return 0;
}

AVFrame *Source::copyFrame(AVFrame *frame) {
  AVFrame *copyedFrame = av_frame_alloc();

  copyedFrame->format = frame->format;
  copyedFrame->width = frame->width;
  copyedFrame->height = frame->height;

  copyedFrame->ch_layout = frame->ch_layout;
  copyedFrame->nb_samples = frame->nb_samples;

  av_frame_get_buffer(copyedFrame, 1);
  av_frame_copy(copyedFrame, frame);
  av_frame_copy_props(copyedFrame, frame);
  return copyedFrame;
}

int Source::Shutdown() {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  if (_status == kUnStarted) {
    return 0;
  }
  AVFrame *frame = nullptr;

  for (auto itr = _videoFrames.begin(); itr != _videoFrames.end(); itr++) {
    frame = *itr;
    av_frame_unref(frame);
    av_frame_free(&frame);
  }

  _videoFrames.clear();
  for (auto itr = _audioFrames.begin(); itr != _audioFrames.end(); itr++) {
    frame = *itr;
    av_frame_unref(frame);
    av_frame_free(&frame);
  }
  _audioFrames.clear();

  for (auto itr = _videoCaches.begin(); itr != _videoCaches.end(); itr++) {
    frame = *itr;
    av_frame_unref(frame);
    av_frame_free(&frame);
  }

  _videoCaches.clear();
  for (auto itr = _audioCaches.begin(); itr != _audioCaches.end(); itr++) {
    frame = *itr;
    av_frame_unref(frame);
    av_frame_free(&frame);
  }
  _audioCaches.clear();
  avformat_close_input(&_formatCtx);
  avformat_free_context(_formatCtx);

  avcodec_close(_videoCodecCtx);
  avcodec_close(_audioCodecCtx);

  avcodec_free_context(&_videoCodecCtx);
  avcodec_free_context(&_videoCodecCtx);
  _status = ReadStatus::kUnStarted;

  _videoVal = 0;
  _audioVal = 0;

  av_frame_unref(_pitch);
  av_frame_free(&_pitch);

  return 0;
}

AVFrame *Source::GetVideoFrame(int64_t offset) {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  if (_videoIndex < 0) {
    spdlog::error("can not find video in {}", _name.c_str());
    return nullptr;
  }

  if (_offset != 0 && _offset > offset) {
    return copyFrame(_pitch);
  }

  if (_loop != 1 && _status == ReadStatus::kReadEnd) {
    return copyFrame(_pitch);
  }

  AVFrame *frame = nullptr;
  if (_status == ReadStatus::kReadEnd &&
      _materialType == MaterialType::kAdditionalMaterial) {
    AVFrame *cache = _videoCaches.front();
    frame = copyFrame(cache);
    _videoCaches.pop_front();
    _videoCaches.push_back(cache);
  } else if (_status == ReadStatus::kStarting) {
    while (_videoFrames.empty()) {
      int ret = ReadFrame();
      if (ret != 0) {
        return nullptr;
      }

      if (_loop != 1 && _status == ReadStatus::kReadEnd) {
        return copyFrame(_pitch);
      }

      if (_status == ReadStatus::kReadEnd &&
          _materialType == MaterialType::kAdditionalMaterial &&
          !_videoCaches.empty()) {
        AVFrame *cache = _videoCaches.front();
        frame = copyFrame(cache);
        _videoCaches.pop_front();
        _videoCaches.push_back(cache);
        return frame;
      }
    }

    frame = _videoFrames.front();
    _videoFrames.pop_front();
  }
  return frame;
}

AVFrame *Source::GetAudioFrame() {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  if (_audioIndex < 0) {
    spdlog::error("can not find audio in {}", _name.c_str());
    return nullptr;
  }
  AVFrame *frame = nullptr;
  if (_status == ReadStatus::kReadEnd &&
      _materialType == MaterialType::kAdditionalMaterial) {
    AVFrame *cache = _audioCaches.front();
    frame = copyFrame(cache);
    _audioCaches.pop_front();
    _audioCaches.push_back(cache);
  } else if (_status == ReadStatus::kStarting) {
    while (_audioFrames.empty()) {
      int ret = ReadFrame();
      if (ret != 0) {
        return nullptr;
      }

      if (_status == ReadStatus::kReadEnd &&
          _materialType == MaterialType::kAdditionalMaterial &&
          !_audioCaches.empty()) {
        AVFrame *cache = _audioCaches.front();
        frame = copyFrame(cache);
        _audioCaches.pop_front();
        _audioCaches.push_back(cache);
        return frame;
      }
    }
    frame = _audioFrames.front();
    _audioFrames.pop_front();
  }
  spdlog::debug("read audio frame list  size: {}", _audioFrames.size());
  return frame;
}

int Source::ReadFrame() {
  int ret = 0;
  AVPacket *pkt = av_packet_alloc();
  while (_status != ReadStatus::kReadEnd) {
    ret = av_read_frame(_formatCtx, pkt);
    if (ret == AVERROR_EOF) {
      spdlog::info("read end of file {}", _url.c_str());
      _status = ReadStatus::kReadEnd;
      break;
    }
    if (pkt->stream_index == _audioIndex) {
      ret = avcodec_send_packet(_audioCodecCtx, pkt);
      if (ret != 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
          continue;
        }
        spdlog::info("send pkt to codec failed , err:{}", av_err2str(ret));
        return -1;
      }
      AVFrame *frame = av_frame_alloc();
      ret = avcodec_receive_frame(_audioCodecCtx, frame);
      if (ret != 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
          break;
        }
        spdlog::info("receive frame from codec failed , err:{}",
                     av_err2str(ret));
        return -1;
      }
      _audioFrames.push_back(frame);
      if (_materialType == MaterialType::kAdditionalMaterial) {
        AVFrame *cacheFrame = copyFrame(frame);
        _audioCaches.push_back(cacheFrame);
      }
      av_packet_unref(pkt);
      av_packet_free(&pkt);
      if (_videoIndex < 0) {
        return 0;
      }
      break;
    } else if (pkt->stream_index == _videoIndex) {
      ret = avcodec_send_packet(_videoCodecCtx, pkt);
      if (ret != 0) {
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
          spdlog::info("send to decoder failed , err: {}", av_err2str(ret));
          continue;
        }
        return -1;
      }
      while (true) {
        AVFrame *frame = av_frame_alloc();
        int result = avcodec_receive_frame(_videoCodecCtx, frame);
        if (result != 0) {
          if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {
            break;
          }
          spdlog::error("decode failed {}", av_err2str(result));
          return -1;
        }
        AVFrame *out = this->scaleFrame(frame);
        _videoFrames.push_back(out);
        if (_materialType == MaterialType::kAdditionalMaterial) {
          AVFrame *cacheFrame = copyFrame(out);
          _videoCaches.push_back(cacheFrame);
        }
      }
      av_packet_unref(pkt);
      av_packet_free(&pkt);
      break;
    }
  }
  return 0;
}

const AVCodecContext *Source::GetAudioCodecContext() const {
  return _audioCodecCtx;
}

int Source::GetFrame(AVFrame **frame, uint64_t &timestamp) {
  AVPacket *pkt = av_packet_alloc();
  int ret = av_read_frame(_formatCtx, pkt);
  if (ret == AVERROR_EOF) {
    return ret;
  }
  timestamp = -1;
  if (pkt->stream_index == _audioIndex) {
    AVFrame *out = av_frame_alloc();
    ret = avcodec_send_packet(_audioCodecCtx, pkt);
    if (ret != 0) {
      spdlog::info("send pkt to codec failed , err:{}", av_err2str(ret));
      return ret;
    }
    ret = avcodec_receive_frame(_audioCodecCtx, out);

    if (ret != 0) {
      spdlog::info("receive frame from codec failed , err:{}", av_err2str(ret));
      return ret;
    }
    out->opaque = new char[2];
    strcpy((char *)out->opaque, "a");
    *frame = out;
    if (_videoVal == 0) {
      _videoVal = av_q2d(_formatCtx->streams[pkt->stream_index]->time_base) *
                  pkt->pts * 1000;
      timestamp = kVideoDefaultInterVal;
    } else {
      uint64_t videoVal =
          av_q2d(_formatCtx->streams[pkt->stream_index]->time_base) * pkt->pts *
          1000;
      timestamp = videoVal - _videoVal;
      _videoVal = videoVal;
    }

  } else if (pkt->stream_index == _videoIndex) {
    AVFrame *out = av_frame_alloc();
    ret = avcodec_send_packet(_videoCodecCtx, pkt);
    if (ret != 0) {
      return ret;
    }
    int result = avcodec_receive_frame(_videoCodecCtx, out);
    if (result != 0) {
      spdlog::error("decode failed {}", av_err2str(result));
      return result;
    }

    AVFrame *f = scaleFrame(out);
    f->opaque = new char[2];
    strcpy((char *)f->opaque, "v");
    *frame = f;
    if (_audioVal == 0) {
      _audioVal = av_q2d(_formatCtx->streams[pkt->stream_index]->time_base) *
                  pkt->pts * 1000;
      timestamp = kAudioDefaultInterVal;
    } else {
      uint64_t audioVal =
          av_q2d(_formatCtx->streams[pkt->stream_index]->time_base) * pkt->pts *
          1000;
      timestamp = audioVal - _audioVal;
      _audioVal = audioVal;
    }
  }

  return 0;
}
AVFrame *Source::scaleFrame(AVFrame *frame) {
  if (frame == nullptr) {
    return nullptr;
  }

  AVFrame *outFrame = av_frame_alloc();
  av_frame_copy_props(outFrame, frame);
  outFrame->width = _width;
  outFrame->height = _height;
  outFrame->format = AV_PIX_FMT_ARGB;
  av_frame_get_buffer(outFrame, 1);
  int outHeight = sws_scale(_sws, frame->data, frame->linesize, 0,
                            frame->height, outFrame->data, outFrame->linesize);

  av_frame_unref(frame);
  av_frame_free(&frame);
  return outFrame;
}
