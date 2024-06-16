#ifndef UTIL_TIMER_H_
#define UTIL_TIMER_H_

#include <cstdlib>
#include <iostream>


#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <cstring>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <chrono>
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}


#ifdef av_err2str
#undef av_err2str
av_always_inline std::string av_err2string(int errnum) {
  char str[AV_ERROR_MAX_STRING_SIZE];
  return av_make_error_string(str, AV_ERROR_MAX_STRING_SIZE, errnum);
}
#define av_err2str(err) av_err2string(err).c_str()
#endif

inline int64_t GetNowMs() {
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch())
                .count();
  return ms;
}

std::string calculateMD5(const std::string &input);

inline int64_t GetNowSec() {
  auto now = std::chrono::system_clock::now();
  auto sec =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();
  return sec;
}

std::string getFileExt(std::string  url);
std::string getLocalIP();
int getCurrentProcessId();

AVFrame* GetPic();

#endif