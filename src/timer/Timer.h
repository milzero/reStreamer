//
// Created by luzhi on 2023/11/24.
//

#ifndef RESTREAMER_SRC_TIMER_TIMER_H_
#define RESTREAMER_SRC_TIMER_TIMER_H_

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>


class Timer {
  std::atomic<bool> active{true};

public:
  void setTimeout(auto function, int delay);
  void setInterval(auto function, int interval);
  void stop();
};

#endif // RESTREAMER_SRC_TIMER_TIMER_H_
