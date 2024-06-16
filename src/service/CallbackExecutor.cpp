//
// Created by yile0 on 2023/12/25.
//

#include "CallbackExecutor.h"
#include "Callback.h"
#include "Task.h"
#include "../http/Client.h"
#include "../util/util.h"
#include "Service.h"

#define secretkey  "mx72a7KRgiLod0rkWVIwMrknZ1MW38gb302BYItfE3vg3Lz62vDtqWACXl77Szmo"


std::string GenerateKey(int ts) {
    std::string salt = std::to_string(ts) + "&&" + secretkey + "&&" + std::to_string(ts);
    std::string  key = calculateMD5(salt);
    return key;
}


void CallbackExecutor::executor(std::string event, std::shared_ptr<Task> task) {
    Callback::Callback cb;
    int ts = GetNowSec();
    std::string sign = GenerateKey(ts);
    cb.sign = sign;
    cb.task_id = task->_index;
    cb.appid = "" ;
    if (createTaskReq.appid.has_value()){
        cb.appid = createTaskReq.appid.value();
    }
    cb.message = "";
    cb.version = "";
    if (createTaskReq.version.has_value()){
        cb.version = createTaskReq.version.value();
    }
    cb.timestamp = ts;
    cb.rtmp_id = "";
    cb.data.url = "" ;
    if(createTaskReq.data.has_value()){
        auto item = createTaskReq.data.value();
        cb.data.url = item.callback_url.value();
    }

    cb.data.index = task->_index;
    cb.data.source_urls.push_back(task->_baseSource->GetUrl());
    cb.data.source_urls.push_back(task->_mainSource->GetUrl());
    for (auto s : task->_videoSources) {
        cb.data.source_urls.push_back(s->GetUrl());
    }

    for (auto s : task->_audioSources) {
        cb.data.source_urls.push_back(s->GetUrl());
    }

    nlohmann::json  obj;
    Callback::to_json(obj , cb);

    HttpClient::SendReq(cb.data.url, obj.dump().c_str() , [](std::string rsp) {
        std::cout << "http rsp1: " << rsp << std::endl;
    });
}
