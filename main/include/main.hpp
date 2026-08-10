#include <freertos/FreeRTOS.h>

#include <driver/gpio.h>

#include <esp_wifi.h>
#include <esp_netif.h>

#include <rotary_encoder.hpp>
#include <dimmer.hpp>

#include <anus/wifi.hpp>
#include <anus/discovery.hpp>
#include <anus/http_interface.hpp>

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
	
	anus::WiFi m_wifi;
	anus::Discovery m_discovery;
	anus::HttpInterface m_http_interface;
	
	float m_light_brightness = 0.f;
	bool m_light_active = true;
	
};

//========================================