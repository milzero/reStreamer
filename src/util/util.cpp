//
// Created by yile0 on 2023/12/24.
//
#include <cstring>
#include <iostream>
#include <sys/types.h>


#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wspiapi.h>

#else
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#endif
#include <regex>
#include <string>
#include <openssl/md5.h>
#include "util.h"
#include "../config/Config.h"


extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}



static AVFrame *BG;

int getCurrentProcessId() {
#ifdef _WIN32
  return GetCurrentProcessId();
#else
  return getpid();
#endif
}

std::string calculateMD5(const std::string &input) {
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    MD5_Update(&md5Context, input.c_str(), input.size());

    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &md5Context);

    char md5String[MD5_DIGEST_LENGTH * 2 + 1];
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        sprintf(&md5String[i * 2], "%02x", (unsigned int) digest[i]);
    }

    return std::string(md5String);
}


std::string getLocalIP() {
  char hostname[256];
  if (gethostname(hostname, sizeof(hostname)) != 0) {
    std::cerr << "Error getting hostname." << std::endl;
    return "";
  }
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int status = getaddrinfo(hostname, NULL, &hints, &res);
  if (status != 0) {
    std::cerr << "Error getting address info: " << gai_strerror(status)
              << std::endl;
    return "";
  }
  char ip[INET_ADDRSTRLEN];
  void *addr;
  struct sockaddr_in *addr_in;
  for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
    if (p->ai_family == AF_INET) {
      addr_in = (struct sockaddr_in *)p->ai_addr;
      addr = &(addr_in->sin_addr);
      inet_ntop(AF_INET, addr, ip, INET_ADDRSTRLEN);
      break;
    }
  }
  freeaddrinfo(res);
  return std::string(ip);
}

std::string getFileExt(std::string  url){
    size_t queryStart = url.find('?');
    std::string urlWithoutQuery = url.substr(0, queryStart);
    size_t lastSlash = urlWithoutQuery.find_last_of('/');
    std::string lastPath = urlWithoutQuery.substr(lastSlash + 1);
    size_t dotPos = lastPath.find_last_of('.');
    if (dotPos != std::string::npos && dotPos < lastPath.length() - 1) {
        std::string fileExtension = lastPath.substr(dotPos + 1);
        return fileExtension;
    }
    return "";
}


AVCodecContext *create_decode(AVStream *Stream) {
    const AVCodec *codec = avcodec_find_decoder(Stream->codecpar->codec_id);
    AVCodecContext *codecCtx = avcodec_alloc_context3(codec);
    int ret = avcodec_parameters_to_context(codecCtx, Stream->codecpar);
    if (ret != 0) {
        printf(av_err2str(ret));
        return NULL;
    }
    ret = avcodec_open2(codecCtx, codec, NULL);
    if (ret != 0) {
        printf(av_err2str(ret));
        return NULL;
    }
    return codecCtx;
}

AVFrame* GetPic() {
    if (BG != nullptr){
        return BG;
    }
    AVFormatContext *inFmt = NULL;
    int ret = avformat_open_input(&inFmt, Config::getConfig().BG.c_str(), NULL, NULL);
    if (ret != 0) {
        return NULL;
    }

    int videoIndex =
            av_find_best_stream(inFmt, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (videoIndex < 0) {
        return NULL;
    }

    AVStream *videoStream = inFmt->streams[videoIndex];
    AVCodecContext *videoCodecCtx = create_decode(videoStream);

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();


    while (av_read_frame(inFmt, pkt) >= 0) {
        if (pkt->stream_index == videoIndex) {
            ret = avcodec_send_packet(videoCodecCtx, pkt);
            if (ret != 0) {
                if (ret == AVERROR(EAGAIN)) {
                    printf("send to decoder failed %s \n", av_err2str(ret));
                    continue;
                }
                printf(av_err2str(ret));
                return NULL;
            }
            int result = avcodec_receive_frame(videoCodecCtx, frame);
        }

    }
    BG = frame;
    av_packet_free(&pkt);
    avformat_free_context(inFmt);

    return BG;
}
