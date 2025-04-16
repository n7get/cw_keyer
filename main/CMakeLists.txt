idf_component_register(
	 SRCS "bcd.c" "button.c" "cat.c" "config.c" "ft857d.c" "ft991a.c" "gpio.c" "http.c" "main.c" "message.c" "mock_radio.c" "morse.c" "morse_code_characters.c" "network.c" "radio.c" "settings.c" "status.c" "tune.c" 
	REQUIRES "esp_http_server" 
	PRIV_REQUIRES "esp_driver_gpio" 
	PRIV_REQUIRES "esp_driver_uart"
	PRIV_REQUIRES "esp_wifi"
	PRIV_REQUIRES "json"
	PRIV_REQUIRES "nvs_flash"
	PRIV_REQUIRES "spiffs"
	INCLUDE_DIRS ".")
