#include <Wire.h>

#define LSM6DSOX_ADDR 0x6A   // Try 0x6B if not detected

// Registers
#define CTRL1_XL  0x10
#define CTRL2_G   0x11
#define OUTX_L_G  0x22
#define OUTX_L_A  0x28

void writeRegister(uint8_t reg, uint8_t value) {
  Wire1.beginTransmission(LSM6DSOX_ADDR);
  Wire1.write(reg);
  Wire1.write(value);
  Wire1.endTransmission();
}

void readRegisters(uint8_t reg, uint8_t *buffer, uint8_t len) {
  Wire1.beginTransmission(LSM6DSOX_ADDR);
  Wire1.write(reg);
  Wire1.endTransmission(false);
  Wire1.requestFrom(LSM6DSOX_ADDR, len);

  for (int i = 0; i < len; i++) {
    buffer[i] = Wire1.read();
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Wire1.begin();              // ✅ Use Wire1 for Qwiic
  Wire1.setClock(100000);

  Serial.println("LSM6DSOX Test Start");

  // Accelerometer: 104 Hz, ±2g
  writeRegister(CTRL1_XL, 0x40);

  // Gyroscope: 104 Hz, 250 dps
  writeRegister(CTRL2_G, 0x40);

  delay(100);
}

void loop() {
  uint8_t accelData[6];
  uint8_t gyroData[6];

  // Read accelerometer (6 bytes)
  readRegisters(OUTX_L_A, accelData, 6);

  int16_t ax = (accelData[1] << 8) | accelData[0];
  int16_t ay = (accelData[3] << 8) | accelData[2];
  int16_t az = (accelData[5] << 8) | accelData[4];

  // Read gyroscope (6 bytes)
  readRegisters(OUTX_L_G, gyroData, 6);

  int16_t gx = (gyroData[1] << 8) | gyroData[0];
  int16_t gy = (gyroData[3] << 8) | gyroData[2];
  int16_t gz = (gyroData[5] << 8) | gyroData[4];

  Serial.print("Accel (raw): ");
  Serial.print(ax); Serial.print(", ");
  Serial.print(ay); Serial.print(", ");
  Serial.println(az);

  Serial.print("Gyro (raw): ");
  Serial.print(gx); Serial.print(", ");
  Serial.print(gy); Serial.print(", ");
  Serial.println(gz);

  Serial.println("------------------------");

  delay(500);
}