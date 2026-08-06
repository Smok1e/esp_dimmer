#pragma once

#include <freertos/FreeRTOS.h>

#include <esp_timer.h>

#include <driver/gpio.h>

//========================================

class RotaryEncoder
{
public:
	struct Event
	{
		enum Type: uint8_t
		{
			Rotation,
			Button
		} type: 1;
	
		bool   button: 1;
		int8_t delta:  2;
	};
	
	RotaryEncoder() = default;
	RotaryEncoder(const RotaryEncoder& copy) = delete;
	~RotaryEncoder();
	
	void init(
		gpio_num_t pin_a,
		gpio_num_t pin_b,
		gpio_num_t pin_press
	);
	
	void setValue(int value);
	int  getValue() const;
	
	bool isPressed() const;
	
	bool pollEvent(Event* event);
	
private:
	union State
	{
		struct
		{
			uint8_t a:     1;
			uint8_t b:     1;
			uint8_t press: 1;
		};
	
		uint8_t ab: 2;
	};
	
	gpio_num_t m_pin_a     = GPIO_NUM_NC;
	gpio_num_t m_pin_b     = GPIO_NUM_NC;
	gpio_num_t m_pin_press = GPIO_NUM_NC;
	
	int8_t        m_counter = 0;
	State         m_last_state {};
	QueueHandle_t m_event_queue = nullptr;
	
	uint64_t m_last_button_event_time = esp_timer_get_time();
	
	int  m_value = 0;
	bool m_press = 0;
	
	static void InterruptHandler(void* arg);
	void enqueueEvent(Event event);
	void updateCounter(int8_t direction);
	
};

//========================================