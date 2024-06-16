//
// Created by yile0 on 2023/12/19.
//

#ifndef RESTREAMER_CONFIG_H
#define RESTREAMER_CONFIG_H

#include <fstream>
#include <string>

#include "nlohmann/json.hpp"
struct Config {
  static Config &getConfig() {
    static Config instance;
    return instance;
  }
  std::string logs = "logs";
  int level = 0;
  int Port = 8000;
  std::string Prefix = "/api/v1";
  std::string BG = "output.png";
};
namespace ConfigInterface {
using nlohmann::json;

struct XConfig {
  std::string logs;
  int64_t log_level;
  int64_t port;
  std::string prefix;
  std::string bg;
};
}  // namespace ConfigInterface

namespace ConfigInterface {
void from_json(const json &j, XConfig &x);
void to_json(json &j, const XConfig &x);

inline void from_json(const json &j, XConfig &x) {
  x.logs = j.at("logs").get<std::string>();
  x.log_level = j.at("log_level").get<int64_t>();
  x.port = j.at("port").get<int64_t>();
  x.prefix = j.at("prefix").get<std::string>();
  x.bg = j.at("BG").get<std::string>();
}

inline void to_json(json &j, const XConfig &x) {
  j = json::object();
  j["logs"] = x.logs;
  j["log_level"] = x.log_level;
  j["port"] = x.port;
  j["prefix"] = x.prefix;
  j["BG"] = x.bg;
}
}  // namespace ConfigInterface

static int  loadConifg(std::string path) {
  std::ifstream f(path);
  nlohmann::json data = nlohmann::json::parse(f);
  ConfigInterface::XConfig config;
  ConfigInterface::from_json(data, config);
  Config::getConfig().BG = config.bg;
  return 0;
}

#endif  // RESTREAMER_CONFIG_H
