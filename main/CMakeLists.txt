idf_component_register(SRCS "morse_code.c" "index.c" "morse_code_characters.c" "main.c" "network.c" "http.c" "morse_code.c" "index.c" "api.c"
	            REQUIRES "esp_http_server" 
	            REQUIRES "esp_driver_gpio" 
	            REQUIRES "esp_wifi"
	            REQUIRES "nvs_flash"
                    INCLUDE_DIRS ".")
