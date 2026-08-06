#include <dimmer.hpp>

#include <esp_timer.h>

//========================================

void Dimmer::init(
	gpio_num_t gpio_num_zero_cross_detect,
	gpio_num_t gpio_num_gate_control,
	uint32_t network_period_threshold_us
)
{
	// GPIO
	m_gpio_num_zero_cross_detect = gpio_num_zero_cross_detect;
	m_gpio_num_gate_control = gpio_num_gate_control;
	m_network_period_threshold_us = network_period_threshold_us;
	
	ESP_ERROR_CHECK(gpio_install_isr_service(0));
	
	gpio_config_t zero_cross_detect_config = {};
	zero_cross_detect_config.pin_bit_mask = 1 << gpio_num_zero_cross_detect;
	zero_cross_detect_config.mode = GPIO_MODE_INPUT;
	zero_cross_detect_config.intr_type = GPIO_INTR_NEGEDGE;
	ESP_ERROR_CHECK(gpio_config(&zero_cross_detect_config));
	
	ESP_ERROR_CHECK(
		gpio_isr_handler_add(
			gpio_num_zero_cross_detect,
			Dimmer::InterruptHandler,
			this
		)
	);
	
	gpio_config_t gate_control_config = {};
	gate_control_config.pin_bit_mask = 1 << gpio_num_gate_control;
	gate_control_config.mode = GPIO_MODE_OUTPUT;
	ESP_ERROR_CHECK(gpio_config(&gate_control_config));
	
	// Timer
	gptimer_config_t timer_config = {};
	timer_config.clk_src = GPTIMER_CLK_SRC_DEFAULT;
	timer_config.direction = GPTIMER_COUNT_DOWN;
	timer_config.resolution_hz = 1'000'000;
	
	ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &m_timer));
	
	gptimer_alarm_config_t alarm_config = {};
	alarm_config.alarm_count = 0;
	alarm_config.flags.auto_reload_on_alarm = false;
	
	ESP_ERROR_CHECK(gptimer_set_alarm_action(m_timer, &alarm_config));
	
	gptimer_event_callbacks_t callbacks = {};
	callbacks.on_alarm = Dimmer::AlarmCallback;
	
	ESP_ERROR_CHECK(gptimer_register_event_callbacks(m_timer, &callbacks, this));
	ESP_ERROR_CHECK(gptimer_enable(m_timer));
}

//========================================

void Dimmer::InterruptHandler(void* ctx)
{
	auto& instance = *reinterpret_cast<Dimmer*>(ctx);
	uint64_t time = esp_timer_get_time();
	
	if (instance.m_last_interruption_time)
	{
		auto time_delta = time - instance.m_last_interruption_time;
		if (time_delta < instance.m_network_period_threshold_us)
			return;
		
		gpio_set_level(instance.m_gpio_num_gate_control, false);
		
		if (instance.m_duty_cycle > .035f)
		{
			uint64_t delay_us = time_delta * (1.f - instance.m_duty_cycle);
			gptimer_set_raw_count(instance.m_timer, delay_us);
			gptimer_start(instance.m_timer);
		}
	}

	instance.m_last_interruption_time = time;
}

bool Dimmer::AlarmCallback(
	gptimer_handle_t timer,
	const gptimer_alarm_event_data_t* event_data,
	void* ctx
)
{
	auto& instance = *reinterpret_cast<Dimmer*>(ctx);
	
	gptimer_stop(timer);
	gpio_set_level(instance.m_gpio_num_gate_control, true);
	return false;
}

//========================================

void Dimmer::setDutyCycle(float duty_cycle)
{
	m_duty_cycle = duty_cycle;
}

float Dimmer::getDutyCycle() const
{
	return m_duty_cycle;
}

//========================================