#include <Arduino.h>
#include <Wire.h>

// I2C Addresses
#define IMU_ADDR       0x6A 
#define LED_ADDR       0x39

// LSM6DSOX Register Definitions
#define CTRL1_XL       0x10  
#define CTRL3_C        0x12  
#define ACCEL_REG      0x28  

// Configuration Settings
#define MOVE_THRESHOLD 3000  
#define FRAME_SIZE     12

// Global Variables for History Tracking
int16_t oldAx = 0;
int16_t oldAy = 0;
int16_t oldAz = 0;
bool wasMoving = false; // Tracks state transitions for printing logs

// ==========================================
// UTILITY FUNCTIONS
// ==========================================

// Helper function to log messages with structural timestamps
void logEvent(const char* message) {
  unsigned long ms = millis();
  unsigned long seconds = ms / 1000;
  unsigned long fractionalMs = ms % 1000;
  
  Serial.print("[");
  if (seconds < 10) Serial.print("0"); // Pad formatting
  Serial.print(seconds);
  Serial.print(".");
  if (fractionalMs < 100) Serial.print("0");
  if (fractionalMs < 10) Serial.print("0");
  Serial.print(fractionalMs);
  Serial.print("] ");
  Serial.println(message);
}

// ==========================================
// LED MATRIX FUNCTIONS
// ==========================================

void ledSend(const uint8_t* frame) {
  Wire1.beginTransmission(LED_ADDR);
  Wire1.write(frame, FRAME_SIZE);
  Wire1.endTransmission();
}

void ledOn() {
  uint8_t frame[FRAME_SIZE];
  memset(frame, 0xFF, sizeof(frame));
  ledSend(frame);
}

void ledOff() {
  uint8_t frame[FRAME_SIZE];
  memset(frame, 0x00, sizeof(frame));
  ledSend(frame);
}

bool ledInit() {
  uint8_t initFrame[FRAME_SIZE] = {0x00}; 
  initFrame[0] = 0x01; 
  
  Wire1.beginTransmission(LED_ADDR);
  Wire1.write(initFrame, FRAME_SIZE);
  return (Wire1.endTransmission() == 0);
}

// ==========================================
// IMU FUNCTIONS
// ==========================================

bool imuWrite(uint8_t reg, uint8_t value) {
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(reg);
  Wire1.write(value);
  return (Wire1.endTransmission() == 0);
}

bool imuRead(uint8_t reg, uint8_t* buf, uint8_t len) {
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(reg);
  if (Wire1.endTransmission(false) != 0) { 
    return false; 
  }
  
  if (Wire1.requestFrom((uint8_t)IMU_ADDR, len) != len) {
    return false;
  }
  
  for (uint8_t i = 0; i < len; i++) {
    buf[i] = Wire1.read();
  }
  return true;
}

// ==========================================
// MOVEMENT DETECTION
// ==========================================

bool movementDetected() {
  uint8_t accelData[6];
  if (!imuRead(ACCEL_REG, accelData, 6)) {
    return false;
  }

  // Reconstruct 16-bit signed integers from low/high bytes
  int16_t ax = (int16_t)((accelData[1] << 8) | accelData[0]);
  int16_t ay = (int16_t)((accelData[3] << 8) | accelData[2]);
  int16_t az = (int16_t)((accelData[5] << 8) | accelData[4]);

  // Calculate delta variance
  int32_t dx = ax - oldAx;
  int32_t dy = ay - oldAy;
  int32_t dz = az - oldAz;

  // Store coordinates for next loop
  oldAx = ax;
  oldAy = ay;
  oldAz = az;

  // Manhattan metric approximation to avoid calculation overheads/overflows
  int32_t totalMotion = abs(dx) + abs(dy) + abs(dz);

  return (totalMotion > MOVE_THRESHOLD);
}

// ==========================================
// MAIN ARDUINO ENTRY POINTS
// ==========================================

void setup() {
  Serial.begin(115200);
  while(!Serial); // Wait for the Serial Monitor window to fully clear open
  
  Wire1.begin();
  delay(100); 
  
  logEvent("System booting up...");

  // Configure LSM6DSOX 
  if (imuWrite(CTRL3_C, 0x44)) {
    logEvent("LSM6DSOX Bus protocols configured.");
  } else {
    logEvent("ALERT: Communication error setting up LSM6DSOX configurations.");
  }

  // Power up Accelerometer (104 Hz performance mode, +/- 2g scale)
  if (imuWrite(CTRL1_XL, 0x40)) {
    logEvent("LSM6DSOX Accelerometer streaming initialized successfully.");
  } else {
    logEvent("ALERT: Critical error hardware processing IMU sensor stream!");
  }

  if (ledInit()) {
    logEvent("Matrix structural configuration accepted.");
  } else {
    logEvent("ALERT: External Matrix configuration connection down.");
  }
  
  ledOff();
  logEvent("System deployment state operational.");
}

void loop() {
  bool isCurrentlyMoving = movementDetected();

  // Print logs strictly during state updates to avoid terminal clogging
  if (isCurrentlyMoving && !wasMoving) {
    logEvent("ALERT: Movement detected! Turning LED Matrix ON.");
    ledOn();
    wasMoving = true;
  } 
  else if (!isCurrentlyMoving && wasMoving) {
    logEvent("STATUS: Device stabilized. Turning LED Matrix OFF.");
    ledOff();
    wasMoving = false;
  }
  
  delay(40); // Standard polling window matching hardware refresh output
}
