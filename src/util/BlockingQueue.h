//
// Created by luzhi on 2024/3/5.
//

#ifndef RESTREAMER_SRC_UTIL_BLOCKINGQUEUE_H_
#define RESTREAMER_SRC_UTIL_BLOCKINGQUEUE_H_

#include <condition_variable>
#include <mutex>
#include <queue>


template <typename T> class BlockingQueue {
 public:
  void push(T item) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(item);
    cv_.notify_one();
  }

  void clear() {
    std::unique_lock<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        T item = queue_.front();
        av_packet_unref(item);
        av_packet_free(&item);
        queue_.pop();
    }
  }

  int size() {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty(); });
    T item = queue_.front();
    queue_.pop();
    return item;
  }

 private:
  std::queue<T> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
};
#endif //RESTREAMER_SRC_UTIL_BLOCKINGQUEUE_H_
