idf_component_register(
	SRCS "morse_code.c" "index.c" "morse_code_characters.c" "main.c" "network.c" "http.c" "morse_code.c" "index.c" "api.c" "config.c" "settings.c"
	PRIV_REQUIRES "esp_driver_gpio" 
	PRIV_REQUIRES "esp_http_server" 
	PRIV_REQUIRES "esp_wifi"
	PRIV_REQUIRES "json"
	PRIV_REQUIRES "nvs_flash"
	INCLUDE_DIRS ".")
