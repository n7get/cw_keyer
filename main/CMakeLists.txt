idf_component_register(SRCS "morse_code_characters.c" "main.c" "network.c" "http.c"
	            REQUIRES "esp_http_server" 
	            REQUIRES "esp_driver_gpio" 
	            REQUIRES "esp_wifi"
	            REQUIRES "nvs_flash"
                    INCLUDE_DIRS ".")
