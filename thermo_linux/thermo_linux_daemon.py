# -*- coding: utf-8 -*-
import socket
import time
import msgpack
import sys

# Official UNO Q native Unix socket file path
SOCKET_PATH = "/var/run/arduino-router.sock"

# Alarm thresholds from original design
HIGH_TEMP = 35.0   # °C
HIGH_HUM = 80.0    # %RH
HYSTERESIS = 0.5   

class SystemStatus:
    NORMAL = "NORMAL"
    ALERT_TEMP = "ALERT (HIGH TEMP)"
    ALERT_HUM = "ALERT (HIGH HUMIDITY)"

# System state tracking
current_status = SystemStatus.NORMAL

def update_system_status(temperature, humidity):
    """Evaluates the system state machine utilizing a strict hysteresis window."""
    global current_status
    if current_status == SystemStatus.NORMAL:
        if temperature > HIGH_TEMP:
            current_status = SystemStatus.ALERT_TEMP
        elif humidity > HIGH_HUM:
            current_status = SystemStatus.ALERT_HUM
    elif current_status == SystemStatus.ALERT_TEMP:
        if temperature <= (HIGH_TEMP - HYSTERESIS):
            current_status = SystemStatus.NORMAL
    elif current_status == SystemStatus.ALERT_HUM:
        if humidity <= (HIGH_HUM - HYSTERESIS):
            current_status = SystemStatus.NORMAL

def query_mcu(method_name, msg_id=1):
    """Sends a MessagePack-RPC request and buffers fragmented stream bytes

    until a complete, well-formed response payload arrives.
    """
    # RPC format specification: [type=0 (Request), msg_id, method, params=[]]
    request = [0, msg_id, method_name, []]
    
    sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    try:
        sock.settimeout(2.0)
        sock.connect(SOCKET_PATH)
        sock.sendall(msgpack.packb(request))
        
        # Stream unpacker to isolate chunked/split network frames
        unpacker = msgpack.Unpacker()
        
        while True:
            data = sock.recv(4096)
            if not data:
                raise ConnectionError("Socket connection closed prematurely by router.")
                
            unpacker.feed(data)
            
            # Extract complete unpacked objects from the stream buffer
            for response in unpacker:
                # RPC Response format specification: [type=1, msg_id, error, result]
                if len(response) < 4:
                    raise ValueError(f"Malformed payload tracking format: {response}")
                
                error = response[2]
                result = response[3]
                
                if error is not None:
                    raise RuntimeError(f"Microcontroller error response: {error}")
                    
                return result
    finally:
        sock.close()

def main():
    print("Starting Stream-Buffered Unix-Socket Daemon on UNO Q...")
    print(f"Targeting active socket layer path: {SOCKET_PATH}")
    sys.stdout.flush()

    while True:
        try:
            # 1. Evaluate microcontroller diagnostic registers
            is_faulty = query_mcu("is_sensor_faulty")
            if is_faulty:
                print("Warning: Microcontroller reporting HS3003 sensor fault.", file=sys.stderr)
                time.sleep(2)
                continue

            # 2. Extract live telemetry values from your Arduino functions
            temperature = float(query_mcu("get_temperature"))
            humidity = float(query_mcu("get_humidity"))

            # 3. Process status machine threshold transitions
            update_system_status(temperature, humidity)
            print(f"Temp: {temperature:.2f} C | Hum: {humidity:.2f} %RH | Status: {current_status}")
            sys.stdout.flush()

        except Exception as e:
            # Filter out and silence common transient framing noise to ensure pure metrics logging
            if "Data is not enough" not in str(e):
                print(f"System Notification: ({e})", file=sys.stderr)
                sys.stderr.flush()
            time.sleep(1)
            continue

        # Maintain the 2-second sensing interval defined in the system parameters
        time.sleep(2)

if __name__ == "__main__":
    main()
