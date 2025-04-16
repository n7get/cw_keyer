import os
from http.server import SimpleHTTPRequestHandler, HTTPServer
import requests

ESP32_URL = "http://192.168.68.69"  # Replace with the actual ESP32 address
SPIFFS_DIR = "spiffs_image"  # Directory to serve files from

class DebugHTTPRequestHandler(SimpleHTTPRequestHandler):
    def translate_path(self, path):
        # Serve files from the spiffs_image directory
        return os.path.join(SPIFFS_DIR, path.lstrip("/"))

    def do_GET(self):
        if os.path.exists(self.translate_path(self.path)):
            # Serve the file if it exists locally
            super().do_GET()
        else:
            # Redirect to ESP32 for non-local resources
            self.redirect_to_esp32()

    def do_POST(self):
        # Redirect POST requests to ESP32
        self.redirect_to_esp32()

    def filter_headers(self):
        # Filter headers to include only those compatible with ESP-IDF's HTTP server
        compatible_headers = ["Content-Type", "Content-Length", "Authorization", "User-Agent", "Accept", "Host"]
        filtered_headers = {}
        for key, value in self.headers.items():
            if key in compatible_headers:
                filtered_headers[key] = value
        return filtered_headers

    def redirect_to_esp32(self):
        try:
            url = f"{ESP32_URL}{self.path}"
            headers = self.filter_headers()  # Use filtered headers

            if self.command == "GET":
                response = requests.get(url, headers=headers)
            elif self.command == "POST":
                content_length = int(self.headers.get('Content-Length', 0))
                post_data = self.rfile.read(content_length)
                response = requests.post(url, headers=headers, data=post_data)
            else:
                self.send_error(405, "Method Not Allowed")
                return

            self.send_response(response.status_code)
            for key, value in response.headers.items():
                if key.lower() != 'transfer-encoding':  # Exclude unsupported headers
                    self.send_header(key, value)
            self.end_headers()
            self.wfile.write(response.content)
        except Exception as e:
            self.send_error(500, f"Error redirecting to ESP32: {e}")

if __name__ == "__main__":
    PORT = 8080
    server_address = ("", PORT)
    httpd = HTTPServer(server_address, DebugHTTPRequestHandler)
    print(f"Serving files from {SPIFFS_DIR} on port {PORT}...")
    httpd.serve_forever()
