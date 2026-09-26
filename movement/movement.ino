#include <Wire.h>
#include <Arduino_RouterBridge.h> // Official App Lab RPC bridge

#define IMU_ADDR       0x6A  // Default I2C address for the LSM6DSOX IMU
#define OUTX_L_G       0x22  // First register for Gyroscope output data
#define OUTX_L_A       0x28  // First register for Accelerometer output data
#define CTRL1_XL       0x10  // Accelerometer Control Register
#define CTRL2_G        0x11  // Gyroscope Control Register

// Initialization function to wake up and configure the IMU
bool init_lsm6dsox() {
  // 1. Configure Accelerometer: 104 Hz, +/- 4g range
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(CTRL1_XL);
  Wire1.write(0x48); // 0x40 (104 Hz) | 0x08 (+/- 4g range)
  if (Wire1.endTransmission() != 0) return false;

  // 2. Configure Gyroscope: 104 Hz, 2000 dps full scale
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(CTRL2_G);
  Wire1.write(0x4C); // 0x40 (104 Hz) | 0x0C (2000 dps)
  if (Wire1.endTransmission() != 0) return false;

  return true;
}

// Helper to read 6 sequential bytes (3 axes * 2 bytes each) from a starting register
bool read_i2c_block(uint8_t start_reg, int16_t &raw_x, int16_t &raw_y, int16_t &raw_z) {
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(start_reg);
  if (Wire1.endTransmission() != 0) return false;

  Wire1.requestFrom(IMU_ADDR, 6);
  if (Wire1.available() == 6) {
    uint8_t buffer[6];
    for (int i = 0; i < 6; i++) {
      buffer[i] = Wire1.read();
    }
    // Reconstruct signed 16-bit values (Little Endian format: Low byte first)
    raw_x = (int16_t)((buffer[1] << 8) | buffer[0]);
    raw_y = (int16_t)((buffer[3] << 8) | buffer[2]);
    raw_z = (int16_t)((buffer[5] << 8) | buffer[4]);
    return true;
  }
  return false;
}

// RPC Function exposed to the Python runtime
auto get_motion_telemetry() {
  int16_t raw_ax = 0, raw_ay = 0, raw_az = 0;
  int16_t raw_gx = 0, raw_gy = 0, raw_gz = 0;
  bool success = false;

  // Read both registers blocks sequentially
  if (read_i2c_block(OUTX_L_A, raw_ax, raw_ay, raw_az) && 
      read_i2c_block(OUTX_L_G, raw_gx, raw_gy, raw_gz)) {
    success = true;
  }

  // Forward the raw signed 16-bit integer values across the RPC loop
  return std::make_tuple(success, raw_ax, raw_ay, raw_az, raw_gx, raw_gy, raw_gz);
}

void setup() {
  Serial.begin(115200);
  Wire1.begin(); // Target physical Qwiic bus
  
  // Wait for IMU to wake up and register successfully
  delay(10);
  if (init_lsm6dsox()) {
    Serial.println("LSM6DSOX Register Calibration Successful.");
  } else {
    Serial.println("Hardware Error: LSM6DSOX IMU failed to initialize.");
  }
  
  // Start the background App Lab RPC daemon
  Bridge.begin();
  Bridge.provide("get_motion_telemetry", get_motion_telemetry);
}

void loop() {
  // Intentional empty loop: App Lab handles execution automatically in the background
}
