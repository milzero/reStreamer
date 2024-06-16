//
// Created by yile0 on 2023/12/25.
//

#ifndef RESTREAMER_CALLBACK_H
#define RESTREAMER_CALLBACK_H

#include "nlohmann/json.hpp"

namespace Callback {
using nlohmann::json;

struct Context {};

struct Data {
  std::string url;
  int64_t index;
  std::vector<nlohmann::json> source_urls;
};

struct Callback {
  std::string appid;
  int64_t timestamp;
  std::string version;
  std::string sign;
  std::string callback_event;
  std::string task_id;
  std::string rtmp_id;
  Context context;
  Data data;
  std::string message;
};
} // namespace Callback

namespace Callback {
void from_json(const json &j, Context &x);
void to_json(json &j, const Context &x);

void from_json(const json &j, Data &x);
void to_json(json &j, const Data &x);

void from_json(const json &j, Callback &x);
void to_json(json &j, const Callback &x);

inline void from_json(const json &j, Context &x) {}

inline void to_json(json &j, const Context &x) { j = json::object(); }

inline void from_json(const json &j, Data &x) {
  x.url = j.at("url").get<std::string>();
  x.index = j.at("index").get<int64_t>();
  x.source_urls = j.at("source_urls").get<std::vector<nlohmann::json>>();
}

inline void to_json(json &j, const Data &x) {
  j = json::object();
  j["url"] = x.url;
  j["index"] = x.index;
  j["source_urls"] = x.source_urls;
}

inline void from_json(const json &j, Callback &x) {
  x.appid = j.at("appid").get<std::string>();
  x.timestamp = j.at("timestamp").get<int64_t>();
  x.version = j.at("version").get<std::string>();
  x.sign = j.at("sign").get<std::string>();
  x.callback_event = j.at("callback_event").get<std::string>();
  x.task_id = j.at("task_id").get<std::string>();
  x.rtmp_id = j.at("rtmp_id").get<std::string>();
  x.context = j.at("context").get<Context>();
  x.data = j.at("data").get<Data>();
  x.message = j.at("message").get<std::string>();
}

inline void to_json(json &j, const Callback &x) {
  j = json::object();
  j["appid"] = x.appid;
  j["timestamp"] = x.timestamp;
  j["version"] = x.version;
  j["sign"] = x.sign;
  j["callback_event"] = x.callback_event;
  j["task_id"] = x.task_id;
  j["rtmp_id"] = x.rtmp_id;
  j["context"] = x.context;
  j["data"] = x.data;
  j["message"] = x.message;
}
} // namespace Callback

#endif // RESTREAMER_CALLBACK_H
