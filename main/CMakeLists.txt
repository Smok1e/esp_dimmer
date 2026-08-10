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
		"esp_timer"
		"anus"
		
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-parentheses)