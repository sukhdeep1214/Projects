import time
import math
from enum import Enum
from arduino.app_utils import Bridge, App

# Sensor Sensitivity Scale Factors based on register choices (+/-4g and 2000dps)
ACCEL_SCALE = (0.122 / 1000.0) * 9.80665   # Converts raw LSB directly to m/s²
GYRO_SCALE  = 70.0 / 1000.0                 # 70 mdps/LSB converted to degrees per second

# Threshold configurations (Converted thresholds to m/s²)
G_CONSTANT = 9.80665
IMPACT_THRESHOLD = 1.8 * G_CONSTANT   # ~17.65 m/s² spike to register a knock
TILT_THRESHOLD   = 45.0                # Estimated tilt limit in degrees
MOTION_THRESHOLD = 15.0                # Rotation threshold in degrees/sec

# 100ms loops for fluid motion tracking
SAMPLING_INTERVAL = 0.1  

class MotionState(Enum):
    REST = 0
    MOVING = 1
    TILTED = 2
    IMPACT_ALERT = 3

current_state = MotionState.REST
impact_cooldown = 0.0

def evaluate_motion_state(ax, ay, az, gx, gy, gz):
    """Processes acceleration and rotation vectors to manage alert states."""
    global current_state, impact_cooldown
    
    # Calculate net acceleration magnitude (Total m/s²)
    total_ms2 = math.sqrt(ax**2 + ay**2 + az**2)
    
    # Calculate net rotational magnitude (Total angular speed)
    total_gyro = math.sqrt(gx**2 + gy**2 + gz**2)
    
    # Cooldown logic for impact states
    if current_state == MotionState.IMPACT_ALERT:
        if time.time() > impact_cooldown:
            current_state = MotionState.REST
        return total_ms2
        
    # 1. Highest Priority: Sudden Impact / Knock
    if total_ms2 > IMPACT_THRESHOLD:
        current_state = MotionState.IMPACT_ALERT
        impact_cooldown = time.time() + 1.5
        return total_ms2
        
    # 2. Second Priority: Angular Tilt Check
    if total_ms2 > 1.0: # Protect from zero division
        pitch_est = abs(math.degrees(math.atan2(ax, math.sqrt(ay**2 + az**2))))
        roll_est  = abs(math.degrees(math.atan2(ay, math.sqrt(ax**2 + az**2))))
        
        if pitch_est > TILT_THRESHOLD or roll_est > TILT_THRESHOLD:
            current_state = MotionState.TILTED
            return total_ms2

    # 3. Third Priority: Active Handling / Moving Check
    if total_gyro > MOTION_THRESHOLD:
        current_state = MotionState.MOVING
    else:
        current_state = MotionState.REST
        
    return total_ms2

def main():
    print("=" * 60)
    print("  IMU Motion App (Acceleration in m/s²) Active")
    print("=" * 60)
    
    last_run_time = 0.0

    while True:
        current_time = time.time()
        
        if current_time - last_run_time >= SAMPLING_INTERVAL:
            last_run_time = current_time
            
            try:
                # Query the raw binary registers over the RPC bridge link
                success, r_ax, r_ay, r_az, r_gx, r_gy, r_gz = Bridge.call("get_motion_telemetry")
                
                if success:
                    # Convert raw values into Standard real-world units
                    ax, ay, az = r_ax * ACCEL_SCALE, r_ay * ACCEL_SCALE, r_az * ACCEL_SCALE
                    gx, gy, gz = r_gx * GYRO_SCALE, r_gy * GYRO_SCALE, r_gz * GYRO_SCALE
                    
                    # Evaluate tracking math
                    net_ms2 = evaluate_motion_state(ax, ay, az, gx, gy, gz)
                    
                    # Format state tracking logs with custom tags
                    if current_state == MotionState.REST:
                        state_label = "NORMAL REST"
                    elif current_state == MotionState.MOVING:
                        state_label = "ACTIVE MOVEMENT"
                    elif current_state == MotionState.TILTED:
                        state_label = "WARNING (TILTED)"
                    elif current_state == MotionState.IMPACT_ALERT:
                        state_label = "ALERT (SUDDEN IMPACT)"
                        
                    print(f"[{state_label}] | Net Accel: {net_ms2:.2f} m/s² | Gyro: {math.sqrt(gx**2+gy**2+gz**2):.1f}°/s")
                else:
                    print("[WARNING] Hardware fetch returned false. Check hardware address connection.")
                    
            except Exception as rpc_error:
                print(f"[ERROR] Bridge execution failed: {rpc_error}")
                
        time.sleep(0.01)

if __name__ == "__main__":
    main()
    App.run()
