//
// Created by yile0 on 2023/12/24.
//

#ifndef RESTREAMER_CREATETASKRESP_H
#define RESTREAMER_CREATETASKRESP_H

#include "nlohmann/json.hpp"

const char *kCreateTaskSuccess = "create task successfully";
const char *kCreateTaskFailed = "create task failed";

namespace CreateTaskResp {
using nlohmann::json;

struct Data {
  std::string task_id;
};

struct CreateTaskResp {
  std::string appid;
  std::string version;
  int64_t code;
  std::string message;
  int64_t timestamp;
  Data data;
};
}  // namespace CreateTaskResp

namespace CreateTaskResp {
void from_json(const json &j, Data &x);

void to_json(json &j, const Data &x);

void from_json(const json &j, CreateTaskResp &x);

void to_json(json &j, const CreateTaskResp &x);

inline void from_json(const json &j, Data &x) {
  x.task_id = j.at("task_id").get<std::string>();
}

inline void to_json(json &j, const Data &x) {
  j = json::object();
  j["task_id"] = x.task_id;
}

inline void from_json(const json &j, CreateTaskResp &x) {
  x.appid = j.at("appid").get<std::string>();
  x.version = j.at("version").get<std::string>();
  x.code = j.at("code").get<int64_t>();
  x.message = j.at("message").get<std::string>();
  x.timestamp = j.at("timestamp").get<int64_t>();
  x.data = j.at("data").get<Data>();
}

inline void to_json(json &j, const CreateTaskResp &x) {
  j = json::object();
  j["appid"] = x.appid;
  j["version"] = x.version;
  j["code"] = x.code;
  j["message"] = x.message;
  j["timestamp"] = x.timestamp;
  j["data"] = x.data;
}
}  // namespace CreateTaskResp

#endif  // RESTREAMER_CREATETASKRESP_H
