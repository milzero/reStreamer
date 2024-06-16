//
// Created by yile0 on 2023/12/24.
//

#ifndef RESTREAMER_DELETETASKREQ_H
#define RESTREAMER_DELETETASKREQ_H
#include "nlohmann/json.hpp"


namespace DeleteTaskReq {
    using nlohmann::json;

    struct Data {
        std::string task_id;
    };

    struct DeleteTaskReq {
        std::string appid;
        int64_t timestamp;
        std::string version;
        std::string sign;
        std::string vod_refresh_type;
        Data data;
    };
}

namespace DeleteTaskReq {
    void from_json(const json & j, Data & x);
    void to_json(json & j, const Data & x);

    void from_json(const json & j, DeleteTaskReq & x);
    void to_json(json & j, const DeleteTaskReq & x);

    inline void from_json(const json & j, Data& x) {
        x.task_id = j.at("task_id").get<std::string>();
    }

    inline void to_json(json & j, const Data & x) {
        j = json::object();
        j["task_id"] = x.task_id;
    }

    inline void from_json(const json & j, DeleteTaskReq& x) {
        x.appid = j.at("appid").get<std::string>();
        x.timestamp = j.at("timestamp").get<int64_t>();
        x.version = j.at("version").get<std::string>();
        x.sign = j.at("sign").get<std::string>();
        x.vod_refresh_type = j.at("vod_refresh_type").get<std::string>();
        x.data = j.at("data").get<Data>();
    }

    inline void to_json(json & j, const DeleteTaskReq & x) {
        j = json::object();
        j["appid"] = x.appid;
        j["timestamp"] = x.timestamp;
        j["version"] = x.version;
        j["sign"] = x.sign;
        j["vod_refresh_type"] = x.vod_refresh_type;
        j["data"] = x.data;
    }
}


#endif //RESTREAMER_DELETETASKREQ_H
