#include <Wire.h>

// I2C Device Addresses
#define HS3003_ADDR  0x44
#define BUZZER_ADDR  0x1E

// Alarm thresholds
#define HIGH_TEMP       35.0   // °C
#define HIGH_HUM        80.0   // %RH
#define HYSTERESIS      0.5    

// Timing intervals (in milliseconds)
#define SENSOR_INTERVAL 2000   
#define BEEP_INTERVAL    500   // How often the buzzer chirps during an alert

// System state tracking
unsigned long lastSensorRead = 0;
unsigned long lastBuzzerBeep = 0; // Tracks the repeating alarm beep timing

enum SystemStatus { NORMAL, ALERT_TEMP, ALERT_HUM };
SystemStatus currentStatus = NORMAL;

// =================================================
// Send 4-byte variables safely over I2C
// =================================================
void sendUint32I2C(uint32_t value) {
  for (int i = 0; i < 4; i++) {
    Wire1.write((uint8_t)(value >> (i * 8)));
  }
}

// =================================================
// Send tone to Modulino Buzzer
// =================================================
void buzzerTone(uint32_t frequency, uint32_t duration) {
  Wire1.beginTransmission(BUZZER_ADDR);
  sendUint32I2C(frequency);
  sendUint32I2C(duration);
  Wire1.endTransmission();
}

// =================================================
// Stop buzzer
// =================================================
void buzzerStop() {
  Wire1.beginTransmission(BUZZER_ADDR);
  sendUint32I2C(0);
  sendUint32I2C(0);
  Wire1.endTransmission();
}

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
  
  Serial.println("SensorsApp Initialized Successfully.");
}

// =================================================
// Loop
// =================================================
void loop() {
  unsigned long currentMillis = millis();

  // 1. Non-blocking Sensor Read Timing
  if (currentMillis - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = currentMillis;
    
    // --- Step A: Trigger HS3003 Sensor Measurement ---
    Wire1.beginTransmission(HS3003_ADDR);
    Wire1.endTransmission(); 
    delay(40); 

    // --- Step B: Request 4 bytes back from HS3003 ---
    Wire1.requestFrom(HS3003_ADDR, 4);
    if (Wire1.available() == 4) {
      uint8_t b1 = Wire1.read();
      uint8_t b2 = Wire1.read();
      uint8_t b3 = Wire1.read();
      uint8_t b4 = Wire1.read();

      // Mask status bits and shift raw ranges
      uint16_t rawHum  = ((b1 & 0x3F) << 8) | b2;
      uint16_t rawTemp = (b3 << 6) | (b4 >> 2);

      // Formulas converting raw data ranges into real metrics
      float humidity    = (float)rawHum * 100.0 / 16383.0;
      float temperature = ((float)rawTemp * 165.0 / 16383.0) - 40.0;

      // --- Step C: Output Real Values ---
      Serial.print("Temperature: ");
      Serial.print(temperature, 2);
      Serial.print(" °C | Humidity: ");
      Serial.print(humidity, 2);
      Serial.print(" %RH");

      // --- Step D: Evaluate Thresholds, Set States & Print Alerts ---
      if (temperature > HIGH_TEMP) {
        currentStatus = ALERT_TEMP;
        
        Serial.println(" [ALERT - HIGH TEMP!]"); 
      } 
      else if (temperature < (HIGH_TEMP - HYSTERESIS)) {
        if (currentStatus == ALERT_TEMP) {
          currentStatus = NORMAL;
          buzzerStop(); // Silence immediately
          Serial.println(" [STATUS - RETURNED TO NORMAL]");
        } else {
          Serial.println(); // Just end the line normally if conditions are safe
        }
      } else {
        Serial.println(); // End the line if in the middle of the hysteresis gap
      }
      
    } else {
      Serial.println("[ERROR] Failed to read data payload from HS3003 sensor.");
    }
  }

  // 2. Non-blocking Continuous Alarm Beeping Logic
  if (currentStatus == ALERT_TEMP) {
    if (currentMillis - lastBuzzerBeep >= BEEP_INTERVAL) {
      lastBuzzerBeep = currentMillis;
      
      // Chirp a 1000Hz tone for 150ms every 500ms
      buzzerTone(1000, 150); 
    }
  }
}
