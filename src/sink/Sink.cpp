//
// Created by luzhi on 2023/11/13.
//

#include "Sink.h"

#include "../util/util.h"

int Sink::Push(AVPacket *pkt) {
  int ret = 0;
  spdlog::info("push media type:{} , pts:{} , dts:{}", pkt->stream_index,
               pkt->pts, pkt->dts);
  ret = av_interleaved_write_frame(_outFormatCtx, pkt);
  return ret;
}

int Sink::Stop() {
  int ret = 0;
  ret = av_write_trailer(_outFormatCtx);
  if (_outFormatCtx && !(_outFormatCtx->oformat->flags & AVFMT_NOFILE)) {
    avio_close(_outFormatCtx->pb);
  }
  avformat_free_context(_outFormatCtx);
  _outFormatCtx->max_interleave_delta;
  if (ret < 0 && ret != AVERROR_EOF) {
    spdlog::error("free out  error occurred :{}", av_err2str(ret));
    return -1;
  }
  return ret;
}

AVCodecContext *Sink::InitializeAudio() {
  int ret = 0;
  const AVCodec *audio_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
  AVStream *audio_stream = avformat_new_stream(_outFormatCtx, audio_codec);
  if (!audio_stream) {
    spdlog::error("could not create output stream :{}", av_err2str(ret));
    ret = AVERROR_UNKNOWN;
    return nullptr;
  }

  AVCodecContext *_audioCodecCtx = avcodec_alloc_context3(audio_codec);
  _audioCodecCtx->time_base = av_make_q(1, 44100);
  _audioCodecCtx->sample_fmt =
      AV_SAMPLE_FMT_FLTP; // 采样率，保持和输入的pcm一致
  _audioCodecCtx->bit_rate = 96 * 1024; // 音频码率，按照支持的最佳实践来选择
  _audioCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
  _audioCodecCtx->codec_id = AV_CODEC_ID_AAC;
  _audioCodecCtx->sample_rate = 44100;
  _audioCodecCtx->frame_size = 882;

  ret = avcodec_open2(_audioCodecCtx, audio_codec, nullptr);
  if (ret < 0) {
    spdlog::error("could not open codec :{}", av_err2str(ret));
    return nullptr;
  }
  _audioIndex = audio_stream->index;
  ret = avcodec_parameters_from_context(audio_stream->codecpar, _audioCodecCtx);
  if (ret < 0) {
    spdlog::error("could not copy codec parameters :{}", av_err2str(ret));
    return nullptr;
  }
  av_dump_format(_outFormatCtx, 0, _url.c_str(), 1);
  return _audioCodecCtx;
}

AVCodecContext *Sink::InitializeVideo() {
  int ret = 0;
  const AVCodec *video_codec = avcodec_find_encoder_by_name("libx264");
  AVStream *video_stream = avformat_new_stream(_outFormatCtx, video_codec);
  if (!video_stream) {
    spdlog::error("could not create output stream :{}", av_err2str(ret));
    ret = AVERROR_UNKNOWN;
    return nullptr;
  }

  AVCodecContext *_videoCodecCtx = avcodec_alloc_context3(video_codec);
  const AVRational dst_fps = {25, 1};

  _videoCodecCtx->codec_tag = 0;
  _videoCodecCtx->codec_id = AV_CODEC_ID_H264;
  _videoCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
  _videoCodecCtx->width = _width;
  _videoCodecCtx->height = _height;
  _videoCodecCtx->gop_size = 25;
  _videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
  _videoCodecCtx->framerate = dst_fps;
  _videoCodecCtx->time_base = av_inv_q(dst_fps);
  _videoCodecCtx->bit_rate = kVideoBitrate;

  if (_outFormatCtx->oformat->flags & AVFMT_GLOBALHEADER){
    _videoCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }


  _outFormatCtx->flags |= AVFMT_FLAG_NONBLOCK;
  _outFormatCtx->max_interleave_delta = 30 * 1000 * 1000 ;

  AVDictionary *codec_options = nullptr;
  av_dict_set(&codec_options, "profile", "baseline", 0);
  av_dict_set(&codec_options, "preset", "ultrafast", 0);
  av_dict_set(&codec_options, "tune", "zerolatency", 0);
  av_dict_set_int(&codec_options, "min-keyint", 20, 0);
  ret = avcodec_open2(_videoCodecCtx, video_codec, &codec_options);
  if (ret < 0) {
    spdlog::error("could not open codec :{}", av_err2str(ret));
    return nullptr;
  }

  ret = avcodec_parameters_from_context(video_stream->codecpar, _videoCodecCtx);
  if (ret < 0) {
    spdlog::error("could not open codec :{}", av_err2str(ret));
    return nullptr;
  }

  _videoIndex = video_stream->index;
  return _videoCodecCtx;
}

int Sink::Initialize() {
  int ret = 0;
  ret = avformat_alloc_output_context2(&_outFormatCtx, nullptr, "flv",
                                       _url.c_str());
  if (ret < 0) {
    spdlog::error("error occurred when opening output url :{}",
                  av_err2str(ret));
    return ret;
  }

  ret = av_opt_set(_outFormatCtx, "flvflags", "no_duration_filesize",
                   AV_OPT_SEARCH_CHILDREN);

  if (!_outFormatCtx) {
    spdlog::error("could not create output context :{}", av_err2str(ret));
    ret = AVERROR_UNKNOWN;
    return ret;
  }

  _outFormatCtx->max_delay;
  return 0;
}

int Sink::DeliveryVideo(AVPacket *pkt) {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  AVRational time_base = _outFormatCtx->streams[_videoIndex]->time_base;
  AVRational time_base_q = {1, AV_TIME_BASE};
  auto pts = pkt->pts;
  pkt->pts = av_rescale_q(pkt->pts, time_base_q, time_base);
  pkt->dts = av_rescale_q(pkt->dts, time_base_q, time_base);
  pkt->stream_index = _videoIndex;
  auto nowTs1 = GetNowMs();
  auto size = pkt->size;
  int ret = Push(pkt);
  spdlog::info("send video took {} , size {} ",
              GetNowMs() - nowTs1 , size);
  auto nowTs = GetNowMs();
  spdlog::info("send video interval {} ,pts {}  ,now : {} , last: {}",
               nowTs - _lastVideoSendTs, pts, nowTs, _lastVideoSendTs);
  _lastVideoSendTs = nowTs;
  if (ret < 0) {
    spdlog::error("write video maxing packet fail , {}", av_err2str(ret));
    return ret;
  }
  av_packet_unref(pkt);
  av_packet_free(&pkt);
  return 0;
}

int Sink::DeliveryAudio(AVPacket *pkt) {
  std::lock_guard<std::recursive_mutex> lck(_mutex);
  AVRational time_base = _outFormatCtx->streams[_audioIndex]->time_base;
  AVRational time_base_q = {1, AV_TIME_BASE};
  pkt->pts = av_rescale_q(pkt->pts, time_base_q, time_base);
  pkt->dts = av_rescale_q(pkt->dts, time_base_q, time_base);
  pkt->duration = 23;
  pkt->stream_index = _audioIndex;
  uint64_t pts = pkt->pts;
  uint64_t  now1 = GetNowMs();
  int ret = Push(pkt);
  spdlog::info("audio sent took {}", GetNowMs() - now1);
  auto nowTs = GetNowMs();
  spdlog::info("send audio interval {} ,pts {}  ,now : {} , last: {}",
               nowTs - _lastVideoSendTs, pts, nowTs, _lastVideoSendTs);
  _lastAudioSendTs = nowTs;
  if (ret < 0) {
    spdlog::error("write audio maxing packet fail , {}", av_err2str(ret));
    return ret;
  }
  av_packet_unref(pkt);
  av_packet_free(&pkt);
  return 0;
}

int Sink::Start() {
  int ret = 0;
  const AVOutputFormat *ofmt = nullptr;
  ofmt = _outFormatCtx->oformat;
  if (!(ofmt->flags & AVFMT_NOFILE)) {
    ret = avio_open(&_outFormatCtx->pb, _url.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
      spdlog::error("could not open output URL '{}'", _url.c_str());
      return ret;
    }
  }

  AVDictionary *opts = NULL;
  av_dict_set_int(&opts, "rtmp_live", -1, 0);
  av_dict_set_int(&opts, "tcp_nodelay", 1, 0);
  ret = avformat_write_header(_outFormatCtx, &opts);
  if (ret < 0) {
    spdlog::error("error occurred when opening output url :{}",
                  av_err2str(ret));
    return ret;
  }
  return 0;
}
