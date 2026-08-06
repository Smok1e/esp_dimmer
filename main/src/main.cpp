#include <esp_log.h>
#include <esp_timer.h>
#include <nvs_flash.h>

#include <algorithm>

#include <cmath>
#include <cstring>

#include <main.hpp>

//========================================

static const char* TAG = "main";

#ifdef CONFIG_WIFI_SECURITY_WPA2
	constexpr auto WIFI_SECURITY = WIFI_AUTH_WPA2_PSK;
#else
	constexpr auto WIFI_SECURITY = WIFI_AUTH_WPA3_PSK;
#endif

//======================================== Wifi

void Main::initNVS()
{
	auto result = nvs_flash_init();
	if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		result = nvs_flash_init();
	}
	ESP_ERROR_CHECK(result);
	
	ESP_LOGI(TAG, "nvs initialized");
}

void Main::initWifi()
{
	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	auto* netif = esp_netif_create_default_wifi_sta();
	ESP_ERROR_CHECK(esp_netif_set_hostname(netif, CONFIG_WIFI_HOSTNAME));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id, instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(
		WIFI_EVENT,
		ESP_EVENT_ANY_ID,
		&Main::WifiEventHandler,
		this,
		&instance_any_id
	));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(
		IP_EVENT,
		IP_EVENT_STA_GOT_IP,
		&Main::WifiEventHandler,
		this,
		&instance_got_ip
	));

	wifi_config_t wifi_config = {};
	wifi_config.sta.threshold.authmode = WIFI_SECURITY;

	strncpy(reinterpret_cast<char*>(wifi_config.sta.ssid    ), CONFIG_WIFI_SSID,     sizeof(wifi_config.sta.ssid    ));
	strncpy(reinterpret_cast<char*>(wifi_config.sta.password), CONFIG_WIFI_PASSWORD, sizeof(wifi_config.sta.password));

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi station initialized");
}

void Main::WifiEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	Main* instance = reinterpret_cast<Main*>(arg);

	if      (event_base == WIFI_EVENT) instance->onWifiEvent(event_id, event_data);
	else if (event_base == IP_EVENT  ) instance->onIpEvent  (event_id, event_data);
}

void Main::onWifiEvent(int32_t event_id, void *event_data)
{
	switch (event_id)
	{
		case WIFI_EVENT_STA_START:
			esp_wifi_connect();
			ESP_LOGI(TAG, "connecting to %s...", CONFIG_WIFI_SSID);
			break;

		case WIFI_EVENT_STA_DISCONNECTED:
		{
			esp_wifi_connect();
			ESP_LOGI(
				TAG,
				"wifi disconnected (%d); reconnecting to %s...",
				static_cast<int>(
					reinterpret_cast<wifi_event_sta_disconnected_t*>(event_data)->reason
				),
				CONFIG_WIFI_SSID
			);

			break;
		}
	}
}

void Main::onIpEvent(int32_t event_id, void* event_data)
{
	switch (event_id)
	{
		case IP_EVENT_STA_GOT_IP:
		{
			auto* event = reinterpret_cast<ip_event_got_ip_t*>(event_data);
			ESP_LOGI(TAG, "successfully connected to the AP; got ip: " IPSTR, IP2STR(&event->ip_info.ip));
			
			break;
		}
	}
}

//======================================== Getters/setters

void Main::setLightActive(bool active)
{
	m_light_active = active;
}

bool Main::isLightActive() const
{
	return m_light_active;
}

void Main::setLightBrightness(float brightness)
{
	m_light_brightness = std::clamp<float>(brightness, 0.f, 1.f);
}

float Main::getLightBrightness() const
{
	return m_light_brightness;
}

//======================================== Main cycle

void Main::run()
{
	initNVS();
	initWifi();
	
	m_dimmer.init(
		static_cast<gpio_num_t>(CONFIG_GPIO_NUM_ZERO_CROSS_DETECT),
		static_cast<gpio_num_t>(CONFIG_GPIO_NUM_GATE_CONTROL),
		CONFIG_NETWORK_PERIOD_THRESHOLD_US
	);
	
	m_encoder.init(
		static_cast<gpio_num_t>(CONFIG_GPIO_NUM_KNOB_A),
		static_cast<gpio_num_t>(CONFIG_GPIO_NUM_KNOB_B),
		static_cast<gpio_num_t>(CONFIG_GPIO_NUM_KNOB_BUTTON)
	);
	
	while (true)
	{
		RotaryEncoder::Event event;
		while (m_encoder.pollEvent(&event))
		{
			switch (event.type)
			{
				case RotaryEncoder::Event::Button:
					if (event.button == 1)
						setLightActive(!isLightActive());
					
					break;
					
				case RotaryEncoder::Event::Rotation:
					setLightBrightness(getLightBrightness() + .025f * event.delta);
					break;
					
			}
		}
		
		m_dimmer.setDutyCycle(
			m_light_active
				? m_light_brightness
				: 0.f
		);
		
		vTaskDelay(1);
	}
}

//========================================

extern "C" void app_main()
{
	Main instance;
	instance.run();
}

//========================================