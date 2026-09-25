#include <Wire.h>

// I2C Device Addresses
#define HS3003_ADDR  0x44

// Alarm thresholds
#define HIGH_TEMP       35.0   // °C
#define HIGH_HUM        80.0   // %RH
#define HYSTERESIS      0.5    

// Timing intervals (in milliseconds)
#define SENSOR_INTERVAL 2000   

// System state tracking
unsigned long lastSensorRead = 0;

enum SystemStatus { NORMAL, ALERT_TEMP, ALERT_HUM };
SystemStatus currentStatus = NORMAL;

// =================================================
// Setup
// =================================================
void setup() {
  Serial.begin(115200);
  
  // Initialize the specific I2C bus (Wire1) for Modulino / Qwiic hardware
  Wire1.begin();

  // Wait up to 3 seconds for Serial Monitor connection
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    // Deliberately empty loop while waiting
  }
  
  Serial.println("ThermoApp Initialized Successfully.");
}

// =================================================
// Loop
// =================================================
void loop() {
  unsigned long currentMillis = millis();

  // 1 & 2. Read and Log HS3003 Data at SENSOR_INTERVAL
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;

    float temperature = 0.0;
    float humidity = 0.0;

    if (readHS3003(temperature, humidity)) {
      Serial.print("Temp: ");
      Serial.print(temperature, 2);
      Serial.print(" °C | Hum: ");
      Serial.print(humidity, 2);
      Serial.print(" %RH | Status: ");

      // 3. State machine handling with Hysteresis
      updateSystemStatus(temperature, humidity);

      switch (currentStatus) {
        case NORMAL:     Serial.println("NORMAL"); break;
        case ALERT_TEMP: Serial.println("ALERT (HIGH TEMP)"); break;
        case ALERT_HUM:  Serial.println("ALERT (HIGH HUMIDITY)"); break;
      }
    } else {
      Serial.println("Error: Failed to read from HS3003 sensor.");
    }
  }
}

// =================================================
// Native I2C Communication for HS3003
// =================================================
bool readHS3003(float &temp, float &hum) {
  // Step 1: Send a measurement request (wake up sensor)
  Wire1.beginTransmission(HS3003_ADDR);
  if (Wire1.endTransmission() != 0) {
    return false; // Sensor not responding
  }

  // Step 2: Wait for measurement completion (typically ~33-40ms)
  delay(40);

  // Step 3: Request 4 bytes of data (2 bytes Humidity, 2 bytes Temperature)
  Wire1.requestFrom(HS3003_ADDR, 4);
  if (Wire1.available() == 4) {
    uint8_t data[4];
    for (int i = 0; i < 4; i++) {
      data[i] = Wire1.read();
    }

    // First two bits of byte 0 are status bits (00 = valid data)
    uint8_t status = (data[0] & 0xC0) >> 6;
    if (status != 0) {
      return false; // Data is stale or sensor is in command mode
    }

    // Parse Humidity (14-bit data from bytes 0 and 1)
    uint16_t rawHum = ((data[0] & 0x3F) << 8) | data[1];
    hum = ((float)rawHum / 16383.0) * 100.0;

    // Parse Temperature (14-bit data from bytes 2 and 3 shifted right by 2)
    uint16_t rawTemp = (data[2] << 6) | (data[3] >> 2);
    temp = ((float)rawTemp / 16383.0) * 165.0 - 40.0;

    return true;
  }
  
  return false; 
}

// =================================================
// Hysteresis-Driven State Management
// =================================================
void updateSystemStatus(float t, float h) {
  if (currentStatus == NORMAL) {
    // Check for breach of normal thresholds
    if (t >= HIGH_TEMP) {
      currentStatus = ALERT_TEMP;
    } else if (h >= HIGH_HUM) {
      currentStatus = ALERT_HUM;
    }
  } 
  else if (currentStatus == ALERT_TEMP) {
    // Must fall below (Threshold - Hysteresis) to recover
    if (t < (HIGH_TEMP - HYSTERESIS)) {
      // Check if it drops straight into a humidity breach
      if (h >= HIGH_HUM) {
        currentStatus = ALERT_HUM;
      } else {
        currentStatus = NORMAL;
      }
    }
  } 
  else if (currentStatus == ALERT_HUM) {
    // Must fall below (Threshold - Hysteresis) to recover
    if (h < (HIGH_HUM - HYSTERESIS)) {
      // Check if it drops straight into a temperature breach
      if (t >= HIGH_TEMP) {
        currentStatus = ALERT_TEMP;
      } else {
        currentStatus = NORMAL;
      }
    }
  }
}
