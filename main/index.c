#include "http.h"
#include "index.h"
#include <esp_err.h>
#include <esp_http_server.h>

esp_err_t index_handler(httpd_req_t *req)
{
    const char *pattern = 
        "<!DOCTYPE html> <html lang=\"en\"> <head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <title>Morse Code Sender</title> <style> body { font-family: Arial, sans-serif; margin: 20px; } label { display: block; margin-top: 10px; } input { margin-bottom: 10px; padding: 5px; width: 200px; } button { padding: 5px 10px; margin-top: 10px; } #status { margin-top: 20px; padding: 10px; border: 1px solid #ccc; background-color: #f9f9f9; } </style> </head> <body> <h1>Morse Code Sender</h1> <label for=\"message\">Message:</label> <input type=\"text\" id=\"message\" placeholder=\"Enter your message\"> <label for=\"wpm\">WPM (Words Per Minute):</label> <input type=\"number\" id=\"wpm\" placeholder=\"Enter WPM\" value=\"20\"> <button onclick=\"sendMorse()\">Send Morse Code</button> <div id=\"status\"> <h3>Status</h3> <p id=\"statusText\">Loading...</p> </div> <script> document.getElementById('message').addEventListener('input', function (event) { event.target.value = event.target.value.toUpperCase(); }); async function getStatus() { try { const response = await fetch('/status'); if (!response.ok) { throw new Error('Failed to fetch status'); } const data = await response.json(); const decodedMessage = data.message ? decodeURIComponent(data.message) : 'None'; document.getElementById('statusText').innerText = ` Status: ${data.status} Message: ${decodedMessage} WPM: ${data.wpm} Busy: ${data.busy ? 'Yes' : 'No'} `; } catch (error) { console.error('Error fetching status:', error); document.getElementById('statusText').innerText = 'Error fetching status'; } } async function sendMorse() { const message = document.getElementById('message').value; const wpm = document.getElementById('wpm').value; if (!message || !wpm) { document.getElementById('statusText').innerText = 'Please enter both message and WPM'; return; } try { const response = await fetch(`/morse?message=${encodeURIComponent(message)}&wpm=${wpm}`, { method: 'POST' }); if (!response.ok) { throw new Error('Failed to send Morse code'); } const data = await response.json(); document.getElementById('statusText').innerText = data.result || 'Morse code sent successfully'; } catch (error) { console.error('Error sending Morse code:', error); document.getElementById('statusText').innerText = 'Error sending Morse code'; } } setInterval(getStatus, 1000); window.onload = getStatus; </script> </body> </html>";      

    httpd_resp_set_type(req, "text/html");         // Set response type to HTML
    httpd_resp_send(req, pattern, strlen(pattern)); // Send the HTML content
    return ESP_OK;
}

void register_index_page(void)
{
    register_html_page("/", HTTP_GET, index_handler);
}
