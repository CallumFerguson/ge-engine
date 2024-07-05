import http.server
import socketserver
import uuid
import os
import sys


class NoCacheHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    run_hash = str(uuid.uuid4())

    def __init__(self, *args, **kwargs):
        kwargs['directory'] = directory
        super().__init__(*args, **kwargs)

    def do_GET(self):
        if self.path == '/runhash':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(self.run_hash.encode())
        else:
            super().do_GET()

    def end_headers(self):
        self.send_header('Cache-Control', 'no-store, no-cache, must-revalidate')
        self.send_header('Pragma', 'no-cache')
        self.send_header('Expires', '0')
        super().end_headers()

    def log_message(self, log_format, *args):
        pass


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python start-http-server.py <directory>")
        sys.exit(1)

    directory = os.path.join(os.path.abspath(sys.argv[1]), "dist")

    PORT = 3002
    handler = NoCacheHTTPRequestHandler
    with socketserver.TCPServer(("", PORT), handler) as httpd:
        print("\nserver started at http://127.0.0.1:3002")
        httpd.serve_forever()
