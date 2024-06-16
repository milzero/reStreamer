#pragma once

#include <string>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "common/mongoose.h"


typedef void OnRspCallback(mg_connection *c, std::string);
using ReqHandler = std::function<bool (std::string, std::string, mg_connection *c, OnRspCallback)>;

class HttpServer
{
public:
	HttpServer() {}
	~HttpServer() {}
	void Init(const std::string &port); 
	bool Start(); 
	bool Close();
	void AddHandler(const std::string &url, ReqHandler req_handler); 
	void RemoveHandler(const std::string &url); 
	static std::string s_web_dir; 
	static mg_serve_http_opts s_server_option; 
	static std::unordered_map<std::string, ReqHandler> s_handler_map; 

private:

	static void OnHttpWebsocketEvent(mg_connection *connection, int event_type, void *event_data);

	static void HandleHttpEvent(mg_connection *connection, http_message *http_req);
	static void SendHttpRsp(mg_connection *connection, std::string rsp);

	static int isWebsocket(const mg_connection *connection);
	static void HandleWebsocketMessage(mg_connection *connection, int event_type, websocket_message *ws_msg); 
	static void SendWebsocketMsg(mg_connection *connection, std::string msg);
	static void BroadcastWebsocketMsg(std::string msg); 
	static std::unordered_set<mg_connection *> s_websocket_session_set; 

	std::string m_port;   
	mg_mgr m_mgr;         
};

