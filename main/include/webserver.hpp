#pragma once

#include <esp_http_server.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>

//========================================

class Main;

class Webserver
{
public:
	Webserver() = default;
	
	void init(Main* main);
	void stop();
	
private:
	Main* m_main = nullptr;

	httpd_handle_t m_httpd_handle = 0;
	
	template<auto Handler, httpd_method_t Method = HTTP_GET>
	void registerEndpoint(const char* uri);
	
	void apiUpdateFirmwareHandler    (httpd_req_t* request);
	void apiGetStatusHandler         (httpd_req_t* request);
	void apiSetLightBrightnessHandler(httpd_req_t* request);
	void apiSetLightActiveHandler    (httpd_req_t* request);
	
};

//========================================

template<auto Handler, httpd_method_t Method /*= HTTP_GET*/>
void Webserver::registerEndpoint(const char* uri)
{
	httpd_uri_t config = {};
	config.uri = uri;
	config.method = Method;
	config.user_ctx = this;
	config.handler = [](httpd_req_t* request) -> esp_err_t {
		httpd_resp_set_hdr(request, "Access-Control-Allos-Origin", "*");
		httpd_resp_set_hdr(request, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
		httpd_resp_set_hdr(request, "Access-Control-Allow-Headers", "Auth, Content-Type, Authentication");
		
		(reinterpret_cast<Webserver*>(request->user_ctx)->*Handler)(request);
		
		return ESP_OK;
	};
	
	ESP_ERROR_CHECK(httpd_register_uri_handler(m_httpd_handle, &config));
}

//========================================