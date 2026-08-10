#include <main.hpp>

#include <esp_log.h>
#include <esp_timer.h>

#include <algorithm>

#include <cmath>
#include <cstring>

#include <anus/property/slider.hpp>
#include <anus/property/switch.hpp>


//========================================

using namespace anus::property;

static const char* TAG = "main";

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
	m_wifi.init();
	m_discovery.init();
	m_http_interface.init();
	
	m_http_interface.addProperty(new Slider("brightness"))->onValueUpdated(
		[&](Property* property) -> void {
			setLightBrightness(reinterpret_cast<Slider*>(property)->getValue());
		}
	);
	
	m_http_interface.addProperty(new Switch("active"))->onValueUpdated(
		[&](Property* property) -> void {
			setLightActive(reinterpret_cast<Switch*>(property)->getValue());
		}
	);
	
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