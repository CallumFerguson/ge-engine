import http.server
import socketserver
import sys
import os

class QuietHttpRequestHandler(http.server.SimpleHTTPRequestHandler):
    def log_message(self, format, *args):
        # Override to prevent logging to the console
        pass

if __name__ == '__main__':
    os.chdir(sys.argv[1])  # Change to the desired directory
    port = 9229  # You can choose another port by modifying this line
    with socketserver.TCPServer(("", port), QuietHttpRequestHandler) as httpd:
        print(f"Starting server at http://127.0.0.1:{port}")
        httpd.serve_forever()
