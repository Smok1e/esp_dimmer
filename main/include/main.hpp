#include <freertos/FreeRTOS.h>

#include <driver/gpio.h>

#include <esp_wifi.h>
#include <esp_netif.h>

#include <rotary_encoder.hpp>
#include <dimmer.hpp>
#include <webserver.hpp>

//========================================

class Main
{
public:
	Main() = default;
	
	void run();
	
	void setLightActive(bool active);
	bool isLightActive() const;
	
	void setLightBrightness(float brightness);
	float getLightBrightness() const;
	
private:
	Dimmer m_dimmer;
	RotaryEncoder m_encoder;
	Webserver m_webserver;
	
	float m_light_brightness = 0.f;
	bool m_light_active = true;
	
	static void WifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
	void onWifiEvent(int32_t event_id, void* event_data);
	void onIpEvent(int32_t event_id, void* event_data);

	void initNVS();
	void initWifi();
	
};

//========================================