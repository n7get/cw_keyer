idf_component_register(
	SRCS "api.c" "config.c" "http.c" "index.c" "main.c" "message.c" "morse_code.c" "morse_code_characters.c" "network.c" "settings.c"
	PRIV_REQUIRES "esp_driver_gpio" 
	PRIV_REQUIRES "esp_http_server" 
	PRIV_REQUIRES "esp_wifi"
	PRIV_REQUIRES "json"
	PRIV_REQUIRES "nvs_flash"
	INCLUDE_DIRS ".")
