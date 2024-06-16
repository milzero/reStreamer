//
// Created by luzhi on 2023/11/24.
//

#include "Timer.h"

void Timer::setTimeout(auto function, int delay) {
  active = true;
  std::thread t([=]() {
    if (!active.load())
      return;
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    if (!active.load())
      return;
    function();
  });
  t.detach();
}

void Timer::setInterval(auto function, int interval) {
  active = true;
  std::thread t([=]() {
    while (active.load()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(interval));
      if (!active.load())
        return;
      function();
    }
  });
  t.detach();
}

void Timer::stop() { active = false; }