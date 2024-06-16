//
// Created by yile0 on 2023/11/26.
//

#ifndef RESTREAMER_SERVICE_H
#define RESTREAMER_SERVICE_H

#include "../Engine.h"
#include "../Source.h"
#include "../http/Server.h"
#include "../log/Log.h"
#include "../util/util.h"
#include "CreateTaskReq.h"

static CreateTaskReq::CreateTaskReq createTaskReq;

bool handle_create_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback);
bool handle_update_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback);
bool handle_delete_task(std::string url, std::string body, mg_connection *c,
                        OnRspCallback rsp_callback);
bool handle_query_task(std::string url, std::string body, mg_connection *c,
                       OnRspCallback rsp_callback);
bool handle_close_task(std::string url, std::string body, mg_connection *c,
                       OnRspCallback rsp_callback);

#endif
