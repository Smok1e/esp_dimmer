idf_component_register(
	SRCS
		"src/main.cpp"
		"src/dimmer.cpp"
		"src/rotary_encoder.cpp"
	
	INCLUDE_DIRS
		"include"
	
	REQUIRES
		"esp_driver_gptimer"
		"esp_driver_gpio"
		"esp_http_server"
		"esp_timer"
		"esp_wifi"
		"nvs_flash"
		"app_update"
		
)
