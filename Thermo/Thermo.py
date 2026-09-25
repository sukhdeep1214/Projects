import time
from enum import Enum
from arduino.app_utils import Bridge, App

# Alarm Threshold Configs
HIGH_TEMP = 35.0   # °C
HIGH_HUM = 80.0    # %RH
HYSTERESIS = 0.5   # Gap required to drop out of alert status

# Timing Configuration (2 seconds)
SAMPLING_INTERVAL = 2.0  

class SystemStatus(Enum):
    NORMAL = 0
    ALERT_TEMP = 1
    ALERT_HUM = 2

# Global State tracker
current_status = SystemStatus.NORMAL

def update_system_status(t, h):
    """Hysteresis-Driven State Management State Machine."""
    global current_status
    
    if current_status == SystemStatus.NORMAL:
        if t >= HIGH_TEMP:
            current_status = SystemStatus.ALERT_TEMP
        elif h >= HIGH_HUM:
            current_status = SystemStatus.ALERT_HUM
            
    elif current_status == SystemStatus.ALERT_TEMP:
        # Must fall below (Threshold - Hysteresis) to recover
        if t < (HIGH_TEMP - HYSTERESIS):
            if h >= HIGH_HUM:
                current_status = SystemStatus.ALERT_HUM
            else:
                current_status = SystemStatus.NORMAL
                
    elif current_status == SystemStatus.ALERT_HUM:
        # Must fall below (Threshold - Hysteresis) to recover
        if h < (HIGH_HUM - HYSTERESIS):
            if t >= HIGH_TEMP:
                current_status = SystemStatus.ALERT_TEMP
            else:
                current_status = SystemStatus.NORMAL

def main():
    print("=" * 50)
    print("  HS3003 App with State Machine Initialized (Python)")
    print("=" * 50)
    
    last_run_time = 0.0

    while True:
        current_time = time.time()
        
        if current_time - last_run_time >= SAMPLING_INTERVAL:
            last_run_time = current_time
            
            try:
                # RPC request to the microcontroller
                success, raw_temp, raw_hum = Bridge.call("get_raw_sensor_data")
                
                if success:
                    # Execute exact 14-bit data parsing calculations
                    humidity = (raw_hum / 16383.0) * 100.0
                    temperature = (raw_temp / 16383.0) * 165.0 - 40.0
                    
                    # Update state machine logic
                    update_system_status(temperature, humidity)
                    
                    # Print formatted console stream out with status flags
                    status_str = "NORMAL"
                    if current_status == SystemStatus.ALERT_TEMP:
                        status_str = "ALERT (HIGH TEMP)"
                    elif current_status == SystemStatus.ALERT_HUM:
                        status_str = "ALERT (HIGH HUMIDITY)"
                        
                    print(f"[DATA] Temp: {temperature:.2f} °C | Hum: {humidity:.2f} %RH | State: {status_str}")
                else:
                    print("[WARNING] Hardware fetch returned false. Check Qwiic cable.")
                    
            except Exception as error:
                print(f"[ERROR] Communication failure on RPC router: {error}")
        
        time.sleep(0.05)

if __name__ == "__main__":
    main()
    App.run()
