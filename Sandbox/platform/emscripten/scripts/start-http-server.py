import http.server
import socketserver
import uuid
import os
import sys

run_uuid = str(uuid.uuid4())


class NoCacheHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        kwargs['directory'] = directory
        super().__init__(*args, **kwargs)

    def do_GET(self):
        if self.path == '/run_uuid':
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(run_uuid.encode())
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

    directory = os.path.abspath(sys.argv[1])

    PORT = 3002
    handler = NoCacheHTTPRequestHandler
    with socketserver.TCPServer(("", PORT), handler) as httpd:
        print("\nserver started at http://127.0.0.1:3002/#{}".format(run_uuid))
        httpd.serve_forever()
