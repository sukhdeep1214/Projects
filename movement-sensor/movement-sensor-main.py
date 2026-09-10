from arduino.app_utils import Bridge, App
import requests
import time


WINDOWS_SERVER = "http://192.168.1.8:5000/motion"

COOLDOWN_SECONDS = 5.0

last_motion_time = 0.0


def motion_detected():
    global last_motion_time

    now = time.monotonic()

    if now - last_motion_time < COOLDOWN_SECONDS:
        return

    last_motion_time = now

    print("================================", flush=True)
    print("MOTION DETECTED!", flush=True)
    print("Sending to Windows...", flush=True)

    try:
        response = requests.post(
            WINDOWS_SERVER,
            json={
                "event": "motion_detected"
            },
            timeout=3
        )

        print(
            f"Windows response: {response.status_code}",
            flush=True
        )

        if response.ok:
            print("Motion sent to Windows!", flush=True)

    except requests.RequestException as error:
        print(
            f"Windows connection error: {error}",
            flush=True
        )


# Arduino calls this function when motion occurs
Bridge.provide(
    "motion_detected",
    motion_detected
)


print("======================================")
print("UNO Q Motion Wi-Fi Bridge")
print("======================================")
print(f"Windows PC: {WINDOWS_SERVER}")
print("Waiting for LSM6DSOX motion...")
print()

App.run()

