#include "index.h"
#include "http.h"
#include <esp_err.h>
#include <esp_http_server.h>

esp_err_t index_handler(httpd_req_t *req) {
    const char *html_text =
        "<!DOCTYPE html>\n"
        "<html lang=\"en\">\n"
        "<head>\n"
        "    <meta charset=\"UTF-8\">\n"
        "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
        "    <title>CW Sender</title>\n"
        "    <style>\n"
        "        body {\n"
        "            font-family: Arial, sans-serif;\n"
        "            margin: 20px;\n"
        "            display: flex;\n"
        "            flex-direction: column;\n"
        "            align-items: center;\n"
        "            position: relative;\n"
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
        "        #settingsButton {\n"
        "            background-color: #007BFF;\n"
        "            color: white;\n"
        "            border: none;\n"
        "            border-radius: 5px;\n"
        "            cursor: pointer;\n"
        "            padding: 10px 15px;\n"
        "        }\n"
        "        #settingsButton:hover {\n"
        "            background-color: #0056b3;\n"
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
        "        input {\n"
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
        "        <h1>CW Sender</h1>\n"
        "        <button id=\"settingsButton\" onclick=\"navigateToSettings()\">Settings</button>\n"
        "    </header>\n"
        "    <form>\n"
        "        <label for=\"message\">Message:</label>\n"
        "        <input type=\"text\" id=\"message\" placeholder=\"Enter your message\">\n"
        "\n"
        "        <button type=\"button\" onclick=\"updateMessage()\">Update Message</button>\n"
        "    </form>\n"
        "\n"
        "    <div id=\"status\">\n"
        "        <h3>Status</h3>\n"
        "        <p id=\"statusText\">Loading...</p>\n"
        "    </div>\n"
        "\n"
        "    <script>\n"
        "        let lastMessage = '';\n"
        "\n"
        "        document.getElementById('message').addEventListener('input', function (event) {\n"
        "            event.target.value = event.target.value.toUpperCase();\n"
        "        });\n"
        "\n"
        "        async function getStatus() {\n"
        "            try {\n"
        "                const response = await fetch('/api/status');\n"
        "                if (!response.ok) {\n"
        "                    throw new Error('Failed to fetch status');\n"
        "                }\n"
        "\n"
        "                const data = await response.json();\n"
        "                document.getElementById('statusText').innerText = `Status: ${data.status} Busy: ${data.busy ? 'Yes' : 'No'}`;\n"
        "            } catch (error) {\n"
        "                console.error('Error fetching status:', error);\n"
        "                document.getElementById('statusText').innerText = 'Error fetching status';\n"
        "            }\n"
        "        }\n"
        "\n"
        "        async function loadCurrentMessage() {\n"
        "            try {\n"
        "                const response = await fetch('/api/message');\n"
        "                if (!response.ok) {\n"
        "                    throw new Error('Failed to fetch current message');\n"
        "                }\n"
        "                const data = await response.json();\n"
        "                lastMessage = data.message ? decodeURIComponent(data.message) : '';\n"
        "                document.getElementById('message').value = lastMessage;\n"
        "            } catch (error) {\n"
        "                console.error('Error fetching current message:', error);\n"
        "                document.getElementById('statusText').innerText = 'Error fetching current message';\n"
        "            }\n"
        "        }\n"
        "\n"
        "        async function updateMessage() {\n"
        "            const messageInput = document.getElementById('message').value;\n"
        "\n"
        "            if (!messageInput) {\n"
        "                document.getElementById('statusText').innerText = 'Please enter a message';\n"
        "                return;\n"
        "            }\n"
        "\n"
        "            try {\n"
        "                // Update the message only if it has changed\n"
        "                if (lastMessage !== messageInput) {\n"
        "                    const updateResponse = await fetch('/api/message', {\n"
        "                        method: 'POST',\n"
        "                        headers: { 'Content-Type': 'application/json' },\n"
        "                        body: JSON.stringify({ message: messageInput })\n"
        "                    });\n"
        "                    if (!updateResponse.ok) {\n"
        "                        throw new Error('Failed to update message');\n"
        "                    }\n"
        "                    const updateData = await updateResponse.json();\n"
        "                    document.getElementById('statusText').innerText = updateData.result || 'Message updated successfully';\n"
        "                    lastMessage = messageInput;\n"
        "                }\n"
        "            } catch (error) {\n"
        "                console.error('Error:', error);\n"
        "                document.getElementById('statusText').innerText = 'Error: ' + error.message;\n"
        "            }\n"
        "        }\n"
        "\n"
        "        function navigateToSettings() {\n"
        "            window.location.href = '/settings';\n"
        "        }\n"
        "\n"
        "        setInterval(getStatus, 1000);\n"
        "\n"
        "        window.onload = function () {\n"
        "            loadCurrentMessage();\n"
        "            getStatus();\n"
        "        };\n"
        "    </script>\n"
        "</body>\n"
        "</html>\n";
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_text, strlen(html_text));
    return ESP_OK;
}

void register_index_page(void) {
    register_html_page("/", HTTP_GET, index_handler);
}
