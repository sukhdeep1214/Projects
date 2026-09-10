from http.server import BaseHTTPRequestHandler, HTTPServer
import json
import subprocess


HOST = "0.0.0.0"
PORT = 5000


class MotionHandler(BaseHTTPRequestHandler):

    def do_POST(self):

        if self.path != "/motion":
            self.send_response(404)
            self.end_headers()
            return

        length = int(self.headers.get("Content-Length", 0))

        body = self.rfile.read(length)

        print()
        print("================================")
        print("MOTION DETECTED!")
        print("Received from UNO Q:")
        print(body.decode("utf-8"))
        print("================================")

        # Windows text-to-speech
        powershell_command = (
            "Add-Type -AssemblyName System.Speech; "
            "$s=New-Object System.Speech.Synthesis.SpeechSynthesizer; "
            "$s.Speak('Motion detected')"
        )

        subprocess.Popen(
            [
                "powershell",
                "-NoProfile",
                "-Command",
                powershell_command
            ]
        )

        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()

        self.wfile.write(b"OK")

    def log_message(self, format, *args):
        return


print("================================")
print("UNO Q Motion Listener")
print("Listening on port 5000...")
print("Waiting for UNO Q...")
print("================================")

server = HTTPServer((HOST, PORT), MotionHandler)

server.serve_forever()
