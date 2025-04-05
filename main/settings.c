#include "cJSON.h"
#include "config.h"
#include "esp_log.h"
#include "http.h"
#include "nvs_flash.h"
#include "status.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG = "SETTINGS";

int wpm = 20;
char ap_ssid[32] = "cw_keyer";
char ap_password[64] = "";
char sta_ssid[32] = "";
char sta_password[64] = "";
char radio_model[16] = "MOCK"; // Default radio model
int baud_rate = 38400; // Default baud rate

void load_settings(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    uint8_t u8_v;
    if (get_u8("wpm", &u8_v) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded WPM from NVS: %d", u8_v);
        wpm = (int)u8_v;
    } else {
        ESP_LOGW(TAG, "Failed to load WPM from NVS, using default: %d", wpm);
        set_u8("wpm", (uint8_t)wpm);
        ESP_LOGI(TAG, "Default WPM saved to NVS: %d", wpm);
    }

    if (get_string("ap_ssid", ap_ssid, sizeof(ap_ssid)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded AP SSID from NVS: %s", ap_ssid);
    } else {
        ESP_LOGW(TAG, "Failed to load AP SSID from NVS, using default: %s", ap_ssid);
        set_string("ap_ssid", ap_ssid);
        ESP_LOGI(TAG, "Default AP SSID saved to NVS: %s", ap_ssid);
    }

    if (get_string("ap_password", ap_password, sizeof(ap_password)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded AP Password from NVS: %s", ap_password);
    } else {
        ESP_LOGW(TAG, "Failed to load AP Password from NVS, using default: %s", ap_password);
        set_string("ap_password", ap_password);
        ESP_LOGI(TAG, "Default AP Password saved to NVS: %s", ap_password);
    }

    if (get_string("sta_ssid", sta_ssid, sizeof(sta_ssid)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded STA SSID from NVS: %s", sta_ssid);
    } else {
        ESP_LOGW(TAG, "Failed to load STA SSID from NVS, using default: %s", sta_ssid);
        set_string("sta_ssid", sta_ssid);
        ESP_LOGI(TAG, "Default STA SSID saved to NVS: %s", sta_ssid);
    }

    if (get_string("sta_password", sta_password, sizeof(sta_password)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded STA Password from NVS: %s", sta_password);
    } else {
        ESP_LOGW(TAG, "Failed to load STA Password from NVS, using default: %s", sta_password);
        set_string("sta_password", sta_password);
        ESP_LOGI(TAG, "Default STA Password saved to NVS: %s", sta_password);
    }

    if (get_string("radio_model", radio_model, sizeof(radio_model)) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded radio model from NVS: %s", radio_model);
    } else {
        ESP_LOGW(TAG, "Failed to load radio model from NVS, using default: %s", radio_model);
        set_string("radio_model", radio_model);
        ESP_LOGI(TAG, "Default radio model saved to NVS: %s", radio_model);
    }

    uint32_t u32_v;
    if (get_u32("baud_rate", &u32_v) == ESP_OK) {
        ESP_LOGI(TAG, "Loaded baud rate from NVS: %d", u32_v);
        baud_rate = (int)u32_v;
    } else {
        ESP_LOGW(TAG, "Failed to load baud rate from NVS, using default: %d", baud_rate);
        set_u32("baud_rate", (uint32_t)baud_rate);
    }
}

static esp_err_t set_settings_handler(httpd_req_t *req) {
    char content[256];
    int content_len = httpd_req_recv(req, content, sizeof(content) - 1);

    if (content_len <= 0) {
        ESP_LOGE(TAG, "Failed to receive request body");
        const char *response = "{\"error\": \"Invalid request body\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    content[content_len] = '\0';
    ESP_LOGI(TAG, "Received settings: %s", content);

    cJSON *json = cJSON_Parse(content);
    if (!json) {
        ESP_LOGE(TAG, "Failed to parse JSON");
        const char *response = "{\"error\": \"Invalid JSON format\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    cJSON *wpm_json = cJSON_GetObjectItem(json, "wpm");
    if (wpm_json && cJSON_IsNumber(wpm_json)) {
        int new_wpm = wpm_json->valueint;
        if (new_wpm < 5 || new_wpm > 50) {
            ESP_LOGE(TAG, "WPM out of range (5-50), using default: %d", 20);
            new_wpm = 20;
        }
        if (new_wpm != wpm) {
            wpm = new_wpm;
            set_u8("wpm", (uint8_t)wpm);
            ESP_LOGI(TAG, "WPM updated and saved to NVS: %d", wpm);
        }
    } else {
        ESP_LOGE(TAG, "WPM parameter missing or invalid");
    }

    cJSON *ap_ssid_json = cJSON_GetObjectItem(json, "ap_ssid");
    if (ap_ssid_json && cJSON_IsString(ap_ssid_json)) {
        if (strcmp(ap_ssid, ap_ssid_json->valuestring) != 0) {
            strncpy(ap_ssid, ap_ssid_json->valuestring, sizeof(ap_ssid) - 1);
            ap_ssid[sizeof(ap_ssid) - 1] = '\0';
            set_string("ap_ssid", ap_ssid);
            ESP_LOGI(TAG, "AP SSID updated and saved to NVS: %s", ap_ssid);
        }
    } else {
        ESP_LOGE(TAG, "AP SSID parameter missing or invalid");
    }

    cJSON *ap_password_json = cJSON_GetObjectItem(json, "ap_password");
    if (ap_password_json && cJSON_IsString(ap_password_json)) {
        if (strcmp(ap_password, ap_password_json->valuestring) != 0) {
            strncpy(ap_password, ap_password_json->valuestring, sizeof(ap_password) - 1);
            ap_password[sizeof(ap_password) - 1] = '\0';
            set_string("ap_password", ap_password);
            ESP_LOGI(TAG, "AP Password updated and saved to NVS: %s", ap_password);
        }
    } else {
        ESP_LOGE(TAG, "AP Password parameter missing or invalid");
    }

    cJSON *sta_ssid_json = cJSON_GetObjectItem(json, "sta_ssid");
    if (sta_ssid_json && cJSON_IsString(sta_ssid_json)) {
        if (strcmp(sta_ssid, sta_ssid_json->valuestring) != 0) {
            strncpy(sta_ssid, sta_ssid_json->valuestring, sizeof(sta_ssid) - 1);
            sta_ssid[sizeof(sta_ssid) - 1] = '\0';
            set_string("sta_ssid", sta_ssid);
            ESP_LOGI(TAG, "STA SSID updated and saved to NVS: %s", sta_ssid);
        }
    } else {
        ESP_LOGE(TAG, "STA SSID parameter missing or invalid");
    }

    cJSON *sta_password_json = cJSON_GetObjectItem(json, "sta_password");
    if (sta_password_json && cJSON_IsString(sta_password_json)) {
        if (strcmp(sta_password, sta_password_json->valuestring) != 0) {
            strncpy(sta_password, sta_password_json->valuestring, sizeof(sta_password) - 1);
            sta_password[sizeof(sta_password) - 1] = '\0';
            set_string("sta_password", sta_password);
            ESP_LOGI(TAG, "STA Password updated and saved to NVS: %s", sta_password);
        }
    } else {
        ESP_LOGE(TAG, "STA Password parameter missing or invalid");
    }

    cJSON *radio_model_json = cJSON_GetObjectItem(json, "radio_model");
    if (radio_model_json && cJSON_IsString(radio_model_json)) {
        if (strcmp(radio_model, radio_model_json->valuestring) != 0) {
            strncpy(radio_model, radio_model_json->valuestring, sizeof(radio_model) - 1);
            radio_model[sizeof(radio_model) - 1] = '\0';
            set_string("radio_model", radio_model);
            ESP_LOGI(TAG, "Radio model updated and saved to NVS: %s", radio_model);
        }
    } else {
        ESP_LOGE(TAG, "Radio model parameter missing or invalid");
    }

    cJSON *baud_rate_json = cJSON_GetObjectItem(json, "baud_rate");
    if (baud_rate_json && cJSON_IsNumber(baud_rate_json)) {
        int new_baud_rate = baud_rate_json->valueint;
        if (new_baud_rate != baud_rate) {
            baud_rate = new_baud_rate;
            set_u32("baud_rate", (uint32_t)baud_rate);
            ESP_LOGI(TAG, "Baud rate updated and saved to NVS: %d", baud_rate);
        }
    } else {
        ESP_LOGE(TAG, "Baud rate parameter missing or invalid");
    }

    cJSON_Delete(json);

    const char *response = "{\"result\": \"Settings updated successfully\"}";
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    return ESP_OK;
}

static esp_err_t get_settings_handler(httpd_req_t *req) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        ESP_LOGE(TAG, "Failed to create JSON object");
        const char *response = "{\"error\": \"Failed to create JSON object\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, response, strlen(response));
        return ESP_FAIL;
    }

    cJSON_AddNumberToObject(json, "wpm", wpm);
    cJSON_AddStringToObject(json, "ap_ssid", ap_ssid);
    cJSON_AddStringToObject(json, "ap_password", ap_password);
    cJSON_AddStringToObject(json, "sta_ssid", sta_ssid);
    cJSON_AddStringToObject(json, "sta_password", sta_password);
    cJSON_AddStringToObject(json, "radio_model", radio_model);
    cJSON_AddNumberToObject(json, "baud_rate", baud_rate);

    const char *response = cJSON_Print(json);
    if (!response) {
        ESP_LOGE(TAG, "Failed to print JSON object");
        cJSON_Delete(json);
        const char *error_response = "{\"error\": \"Failed to generate JSON response\"}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, error_response, strlen(error_response));
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    cJSON_Delete(json);
    free((void *)response);

    return ESP_OK;
}

static esp_err_t settings_page_handler(httpd_req_t *req) {
    const char *html_text =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>Settings</title>\n"
        "    <style>\n"
        "        body {\n"
        "            font-family: Arial, sans-serif;\n"
        "            margin: 20px;\n"
        "            display: flex;\n"
        "            flex-direction: column;\n"
        "            align-items: center;\n"
        "        }\n"
        "        header {\n"
        "            display: flex;\n"
        "            justify-content: space-between;\n"
        "            align-items: center;\n"
        "            width: 100%;\n"
        "            max-width: 400px;\n"
        "            margin-bottom: 20px;\n"
        "        }\n"
        "        header h1 {\n"
        "            margin: 0;\n"
        "            text-align: center;\n"
        "            flex-grow: 1;\n"
        "        }\n"
        "        #cancelButton {\n"
        "            background-color: #FF4C4C;\n"
        "            color: white;\n"
        "            border: none;\n"
        "            border-radius: 5px;\n"
        "            cursor: pointer;\n"
        "            padding: 10px 15px;\n"
        "        }\n"
        "        #cancelButton:hover {\n"
        "            background-color: #CC0000;\n"
        "        }\n"
        "        form {\n"
        "            display: flex;\n"
        "            flex-direction: column;\n"
        "            align-items: flex-start;\n"
        "            width: 100%;\n"
        "            max-width: 400px;\n"
        "        }\n"
        "        label {\n"
        "            margin-top: 10px;\n"
        "        }\n"
        "        input, select {\n"
        "            margin-bottom: 10px;\n"
        "            padding: 5px;\n"
        "            width: 100%;\n"
        "            box-sizing: border-box;\n"
        "        }\n"
        "        button {\n"
        "            padding: 10px 15px;\n"
        "            margin-top: 10px;\n"
        "            align-self: flex-start;\n"
        "        }\n"
        "        #status {\n"
        "            margin-top: 20px;\n"
        "            padding: 10px;\n"
        "            border: 1px solid #ccc;\n"
        "            background-color: #f9f9f9;\n"
        "            width: 100%;\n"
        "            max-width: 400px;\n"
        "            box-sizing: border-box;\n"
        "        }\n"
        "        #status h3 {\n"
        "            margin: 0 0 10px 0;\n"
        "        }\n"
        "    </style>\n"
        "</head>\n"
        "<body>\n"
        "    <header>\n"
        "        <h1>Settings</h1>\n"
        "        <button id=\"cancelButton\" onclick=\"navigateToIndex()\">Cancel</button>\n"
        "    </header>\n"
        "    <form id=\"settingsForm\">\n"
        "        <label for=\"wpm\">WPM (Words Per Minute):</label>\n"
        "        <input type=\"number\" id=\"wpm\" name=\"wpm\" placeholder=\"Enter WPM\" min=\"5\" max=\"50\">\n"
        "\n"
        "        <label for=\"ap_ssid\">AP SSID:</label>\n"
        "        <input type=\"text\" id=\"ap_ssid\" name=\"ap_ssid\" placeholder=\"Enter AP SSID\">\n"
        "\n"
        "        <label for=\"ap_password\">AP Password:</label>\n"
        "        <input type=\"password\" id=\"ap_password\" name=\"ap_password\" placeholder=\"Enter AP Password\">\n"
        "\n"
        "        <label for=\"sta_ssid\">STA SSID:</label>\n"
        "        <input type=\"text\" id=\"sta_ssid\" name=\"sta_ssid\" placeholder=\"Enter STA SSID\">\n"
        "\n"
        "        <label for=\"sta_password\">STA Password:</label>\n"
        "        <input type=\"password\" id=\"sta_password\" name=\"sta_password\" placeholder=\"Enter STA Password\">\n"
        "\n"
        "        <label for=\"radio_model\">Radio Model:</label>\n"
        "        <select id=\"radio_model\" name=\"radio_model\">\n"
        "            <option value=\"MOCK\">Mock Radio</option>\n"
        "            <option value=\"FT-857D\">FT-857D</option>\n"
        "        </select>\n"
        "\n"
        "        <label for=\"baud_rate\">Baud Rate:</label>\n"
        "        <select id=\"baud_rate\" name=\"baud_rate\">\n"
        "            <option value=\"4800\">4800</option>\n"
        "            <option value=\"9600\">9600</option>\n"
        "            <option value=\"19200\">19200</option>\n"
        "            <option value=\"38400\">38400</option>\n"
        "        </select>\n"
        "\n"
        "        <button type=\"button\" onclick=\"updateSettings()\">Update Settings</button>\n"
        "    </form>\n"
        "\n"
        "    <div id=\"status\">\n"
        "        <h3>Status</h3>\n"
        "        <p id=\"statusText\">Loading...</p>\n"
        "    </div>\n"
        "\n"
        "    <script>\n"
        "        // Navigate back to the index page\n"
        "        function navigateToIndex() {\n"
        "            window.location.href = '/';\n"
        "        }\n"
        "\n"
        "        // Fetch current settings from the server\n"
        "        async function fetchSettings() {\n"
        "            try {\n"
        "                const response = await fetch('/api/settings');\n"
        "                if (!response.ok) {\n"
        "                    throw new Error('Failed to fetch settings');\n"
        "                }\n"
        "                const data = await response.json();\n"
        "                document.getElementById('wpm').value = data.wpm;\n"
        "                document.getElementById('ap_ssid').value = data.ap_ssid;\n"
        "                document.getElementById('ap_password').value = data.ap_password;\n"
        "                document.getElementById('sta_ssid').value = data.sta_ssid;\n"
        "                document.getElementById('sta_password').value = data.sta_password;\n"
        "                document.getElementById('radio_model').value = data.radio_model;\n"
        "                document.getElementById('baud_rate').value = data.baud_rate;\n"
        "                document.getElementById('statusText').innerText = 'Settings loaded successfully';\n"
        "            } catch (error) {\n"
        "                console.error('Error fetching settings:', error);\n"
        "                document.getElementById('statusText').innerText = 'Error fetching settings';\n"
        "            }\n"
        "        }\n"
        "\n"
        "        // Update settings on the server\n"
        "        async function updateSettings() {\n"
        "            const settings = {\n"
        "                wpm: parseInt(document.getElementById('wpm').value, 10),\n"
        "                ap_ssid: document.getElementById('ap_ssid').value,\n"
        "                ap_password: document.getElementById('ap_password').value,\n"
        "                sta_ssid: document.getElementById('sta_ssid').value,\n"
        "                sta_password: document.getElementById('sta_password').value,\n"
        "                radio_model: document.getElementById('radio_model').value,\n"
        "                baud_rate: parseInt(document.getElementById('baud_rate').value, 10),\n"
        "            };\n"
        "\n"
        "            try {\n"
        "                const response = await fetch('/api/settings', {\n"
        "                    method: 'POST',\n"
        "                    headers: {\n"
        "                        'Content-Type': 'application/json',\n"
        "                    },\n"
        "                    body: JSON.stringify(settings),\n"
        "                });\n"
        "\n"
        "                if (!response.ok) {\n"
        "                    throw new Error('Failed to update settings');\n"
        "                }\n"
        "\n"
        "                const data = await response.json();\n"
        "                document.getElementById('statusText').innerText = data.result || 'Settings updated successfully';\n"
        "\n"
        "                // Navigate back to the index page after a short delay\n"
        "                setTimeout(() => {\n"
        "                    window.location.href = '/';\n"
        "                }, 2000); // 2-second delay\n"
        "            } catch (error) {\n"
        "                console.error('Error updating settings:', error);\n"
        "                document.getElementById('statusText').innerText = 'Error updating settings';\n"
        "            }\n"
        "        }\n"
        "\n"
        "        // Load settings on page load\n"
        "        window.onload = fetchSettings;\n"
        "    </script>\n"
        "</body>\n"
        "</html>\n";

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_text, strlen(html_text));
    return ESP_OK;
}

void register_settings_endpoints(void) {
    register_html_page("/settings", HTTP_GET, settings_page_handler);

    register_html_page("/api/settings", HTTP_GET, get_settings_handler);
    register_html_page("/api/settings", HTTP_POST, set_settings_handler);

    ESP_LOGI(TAG, "Settings endpoints registered");
}
