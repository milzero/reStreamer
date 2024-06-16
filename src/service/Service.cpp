//
// Created by yile0 on 2023/11/26.
//

#include "Service.h"
#include "CreateTaskReq.h"
#include "CreateTaskResp.h"

#include "nlohmann/json.hpp"

std::shared_ptr<Source> GetSource(const CreateTaskReq::BgImage &img) {

  int left, right, top, bottom;
  right = 0;
  if (img.right.has_value()) {
    right = img.right.value();
  }

  left = 0;
  if (img.left.has_value()) {
    left = img.left.value();
  }

  top = 0;
  if (img.top.has_value()) {
    top = img.top.value();
  }

  bottom = 0;
  if (img.bottom.has_value()) {
    bottom = img.bottom.value();
  }

  int level = 0;
  if (img.level.has_value()) {
    level = img.level.value();
  }

  std::string url;
  if (img.url.has_value()) {
    url = img.url.value();
  }

  double offset = 0;
  if (img.offset.has_value()) {
    offset = img.offset.value();
  }

  uint64_t loop = 1;
  if (img.is_loop.has_value()) {
    loop = img.is_loop.value();
  }
  auto source =
      std::make_shared<Source>(url, level, offset, loop, left, top,
                               right - left, bottom - top, kAdditionalMaterial);
  return source;
}

bool handle_create_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback) {
  std::string err;
  auto js = nlohmann::json::parse(body);
  CreateTaskReq::CreateTaskReq req;
  CreateTaskReq::from_json(js, req);
  auto &engin = Engine::GetEngine();
  createTaskReq = req;
  std::list<Task *> tasks;
  if (!createTaskReq.data.has_value()) {
    return false;
  }

  auto reqData = createTaskReq.data.value();
  if (!reqData.sources.has_value()) {
    return false;
  }

  auto sources = reqData.sources.value();
  for (int i = 0; i < sources.size(); ++i) {
    Task *task = new Task(i);
    auto &main = sources[i];
    if (!main.videos.has_value()) {
      return false;
    }

    auto videos = main.videos.value();
    if (!videos.main_video_url.has_value() || !videos.left.has_value() ||
        !videos.level.has_value() || !videos.right.has_value()) {
      return false;
    }

    std::shared_ptr<Source> mainSource = std::make_shared<Source>(
        videos.main_video_url.value(), videos.level.value(), 0, 0,
        videos.left.value(), videos.top.value(),
        videos.right.value() - videos.left.value(),
        videos.bottom.value() - videos.top.value(), kMaterial);

    task->_mainSource = mainSource;
    task->_baseSource = GetSource(main.bg_image.value());
    if (main.close_shot.has_value()) {
      std::shared_ptr<Source> close_shot = GetSource(main.close_shot.value());
      task->_videoSources.push_back(close_shot);
    }

    if (main.bg_audio.has_value()) {
      std::string url = "";
      auto bg_audio = main.bg_audio.value();
      if (bg_audio.url.has_value()) {
        url = bg_audio.url.value();
        std::shared_ptr<Source> bgm = std::make_shared<Source>(
            url, 100, 0, 0, 0, 0, 0, 0, kAdditionalMaterial);
        task->_audioSources.push_back(bgm);
      }
    }

    auto patchs = main.patches.value();
    for (int j = 0; j < patchs.size(); ++j) {
      auto &patch = patchs[j];
      std::shared_ptr<Source> patchSource = GetSource(patch);
      task->_videoSources.push_back(patchSource);
    }

    tasks.push_back(task);
  }

  CreateTaskResp::CreateTaskResp resp;
  nlohmann::json obj;
  auto rtmp = reqData.rtmp.value();
  if (rtmp.empty() || rtmp.size() == 0) {
    resp.version = req.version.value();
    resp.code = 0;
    resp.timestamp = GetNowSec();
    resp.appid = req.appid.value();
    resp.message = kCreateTaskFailed;
    CreateTaskResp::to_json(obj, resp);
    rsp_callback(c, obj.dump());
  }

  Engine::GetEngine().SetUrl(req.data.value().rtmp.value());
  Engine::GetEngine().SetCallBack(req.data.value().callback_url.value());
  Engine::GetEngine().UpdateTasks(tasks);
  Engine::GetEngine().Run();

  resp.version = req.version.value();
  resp.code = 0;
  resp.timestamp = GetNowSec();
  resp.appid = req.appid.value();
  resp.message = kCreateTaskSuccess;
  resp.data.task_id = getLocalIP() + std::to_string(getCurrentProcessId());
  CreateTaskResp::to_json(obj, resp);
  rsp_callback(c, obj.dump());
  return true;
}

bool handle_update_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback) {
  std::string err;
  auto js = nlohmann::json::parse(body);
  CreateTaskReq::CreateTaskReq req;
  CreateTaskReq::from_json(js, req);
  auto &engin = Engine::GetEngine();
  createTaskReq = req;
  CreateTaskResp::CreateTaskResp resp;
  nlohmann::json obj;
  std::list<Task *> tasks;
  if (!createTaskReq.data.has_value()) {
    resp.version = req.version.value();
    resp.code = -1;
    resp.timestamp = GetNowSec();
    resp.appid = req.appid.value();
    resp.message = "please check your , data is null";
    CreateTaskResp::to_json(obj, resp);
    return false;
  }

  auto reqData = createTaskReq.data.value();
  if (!reqData.sources.has_value()) {
    resp.version = req.version.value();
    resp.code = -1;
    resp.timestamp = GetNowSec();
    resp.appid = req.appid.value();
    resp.message = "please check your request sources";
    CreateTaskResp::to_json(obj, resp);
    return false;
  }

  auto sources = reqData.sources.value();
  for (int i = 0; i < sources.size(); ++i) {
    Task *task = new Task(i);
    auto &main = sources[i];
    if (!main.videos.has_value()) {
      resp.version = req.version.value();
      resp.code = -1;
      resp.timestamp = GetNowSec();
      resp.appid = req.appid.value();
      resp.message =
          "please check your request main_video_url or left or level or right";
      CreateTaskResp::to_json(obj, resp);
      rsp_callback(c, obj.dump());
      return false;
    }

    auto videos = main.videos.value();
    if (!videos.main_video_url.has_value() || !videos.left.has_value() ||
        !videos.level.has_value() || !videos.right.has_value()) {
      resp.version = req.version.value();
      resp.code = -1;
      resp.timestamp = GetNowSec();
      resp.appid = req.appid.value();
      resp.message =
          "please check your request main_video_url or left or level or right";
      CreateTaskResp::to_json(obj, resp);
      rsp_callback(c, obj.dump());
      return false;
    }

    std::shared_ptr<Source> mainSource = std::make_shared<Source>(
        videos.main_video_url.value(), videos.level.value(), 0, 0,
        videos.left.value(), videos.top.value(),
        videos.right.value() - videos.left.value(),
        videos.bottom.value() - videos.top.value(), kMaterial);

    task->_mainSource = mainSource;
    task->_baseSource = GetSource(main.bg_image.value());
    std::shared_ptr<Source> close_shot = GetSource(main.close_shot.value());
    task->_videoSources.push_back(close_shot);

    if (main.bg_audio.has_value()) {
      std::string url = "";
      auto bg_audio = main.bg_audio.value();
      if (bg_audio.url.has_value()) {
        url = bg_audio.url.value();
        std::shared_ptr<Source> bgm = std::make_shared<Source>(
            url, 100, 0, 0, 0, 0, 0, 0, kAdditionalMaterial);
        task->_audioSources.push_back(bgm);
      }
    }

    auto patchs = main.patches.value();
    for (int j = 0; j < patchs.size(); ++j) {
      auto &patch = patchs[j];
      std::shared_ptr<Source> patchSource = GetSource(patch);
      task->_videoSources.push_back(patchSource);
    }

    tasks.push_back(task);
  }


  auto rtmp = reqData.rtmp.value();
  if (rtmp.empty() || rtmp.size() == 0) {
    resp.version = req.version.value();
    resp.code = -1;
    resp.timestamp = GetNowSec();
    resp.appid = req.appid.value();
    resp.message = kCreateTaskFailed;
    CreateTaskResp::to_json(obj, resp);
    rsp_callback(c, obj.dump());
  }

  Engine::GetEngine().UpdateTasks(tasks);
  resp.version = req.version.value();
  resp.code = 0;
  resp.timestamp = GetNowSec();
  resp.appid = req.appid.value();
  resp.message = kCreateTaskSuccess;
  resp.data.task_id = getLocalIP() + std::to_string(getCurrentProcessId());
  CreateTaskResp::to_json(obj, resp);
  rsp_callback(c, obj.dump());
  return true;
}

bool handle_delete_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback) {
  std::string err;
  auto js = nlohmann::json::parse(body);
  CreateTaskReq::CreateTaskReq req;
  CreateTaskReq::from_json(js, req);
  auto &engin = Engine::GetEngine();

  CreateTaskResp::CreateTaskResp resp;
  nlohmann::json obj;
  resp.version = req.version.value();
  resp.code = 0;
  resp.timestamp = GetNowSec();
  resp.appid = req.appid.value();
  resp.message = kCreateTaskSuccess;
  resp.data.task_id = getLocalIP() + std::to_string(getCurrentProcessId());
  CreateTaskResp::to_json(obj, resp);
  rsp_callback(c, obj.dump());
  exit(0);
  return true;
}

bool handle_query_task(std::string url, std::string body, mg_connection *c,
                       OnRspCallback rsp_callback) {
  nlohmann::json obj;
  CreateTaskReq::to_json(obj, createTaskReq);
  rsp_callback(c, obj.dump());
  return true;
}

bool handle_close_task(std::string url, std::string body, mg_connection *c,
                       OnRspCallback rsp_callback) {
  std::string err;
  auto js = nlohmann::json::parse(body);
  CreateTaskReq::CreateTaskReq req;
  CreateTaskReq::from_json(js, req);
  auto &engin = Engine::GetEngine();

  CreateTaskResp::CreateTaskResp resp;
  nlohmann::json obj;
  resp.version = req.version.value();
  resp.code = 0;
  resp.timestamp = GetNowSec();
  resp.appid = req.appid.value();
  resp.message = kCreateTaskSuccess;
  resp.data.task_id = getLocalIP() + std::to_string(getCurrentProcessId());
  CreateTaskResp::to_json(obj, resp);
  rsp_callback(c, obj.dump());
  exit(0);
  return true;
}
