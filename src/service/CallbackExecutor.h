//
// Created by yile0 on 2023/12/25.
//

#ifndef RESTREAMER_CALLBACKEXECUTOR_H
#define RESTREAMER_CALLBACKEXECUTOR_H

#include <string>
#include <memory>

#define kTaskStart "TaskStart"
#define kTaskExit "TaskExit"
#define kMergeStart "MergeStart"
#define kMergeError "MergeError"
#define kMergeEnd "MergeEnd"
#define kVodSourceFileStart "VodSourceFileStart"
#define kVodSourceFileFinish "VodSourceFileFinish"
#define kResetTaskConfig "ResetTaskConfig"
#define kPullFileUnstable "PullFileUnstable"
#define kPushStreamUnstable "PushStreamUnstable"
#define kPullFileFailed "PullFileFailed"
#define kPushStreamFailed "PushStreamFailed"
#define kFileEndEarly "FileEndEarly"

class Task;

class CallbackExecutor {
 public:
  static CallbackExecutor& GetCallbackExecutor() {
    static CallbackExecutor executor;
    return executor;
  }

  void executor(std::string event , std::shared_ptr<Task> task);
 private:
  CallbackExecutor() = default;

};

#endif  // RESTREAMER_CALLBACKEXECUTOR_H
