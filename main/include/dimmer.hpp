#include <driver/gpio.h>
#include <driver/gptimer.h>

//========================================

class Dimmer
{
public:
	Dimmer() = default;

	void init(
		gpio_num_t gpio_num_zero_cross_detect,
		gpio_num_t gpio_num_gate_control,
		uint32_t network_period_threshold_us
	);
	
	void setDutyCycle(float duty_cycle);
	float getDutyCycle() const;
	
	static void InterruptHandler(void* ctx);
	
	static bool AlarmCallback(
		gptimer_handle_t timer,
		const gptimer_alarm_event_data_t* event_data,
		void* ctx
	);
	
private:
	gpio_num_t m_gpio_num_zero_cross_detect = GPIO_NUM_NC;
	gpio_num_t m_gpio_num_gate_control = GPIO_NUM_NC;
	uint32_t m_network_period_threshold_us = 0;
	uint64_t m_last_interruption_time = 0;
	float m_duty_cycle = 0.f;
	gptimer_handle_t m_timer = 0;

};

//========================================