//
// Created by yile0 on 2023/11/26.
//

#ifndef RESTREAMER_TASK_H
#define RESTREAMER_TASK_H

#include "../Source.h"
#include <vector>
#include <memory>

class Task {
public:
  Task(int index) : _index(index) {}
  ~Task() = default;
  std::string _taskName;

  int _index;
  std::shared_ptr<Source> _baseSource;
  std::shared_ptr<Source>_mainSource;
  std::vector<std::shared_ptr<Source>> _videoSources;
  std::vector<std::shared_ptr<Source>> _audioSources;
};

#endif // RESTREAMER_TASK_H
