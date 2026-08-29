#include <Wire.h>

#define HS3003_ADDR  0x44
#define BUZZER_ADDR  0x1E

// Alarm thresholds
#define HIGH_TEMP 35.0    // °C
#define HIGH_HUM  80.0    // %RH


// =================================================
// Send tone to Modulino Buzzer
// =================================================
void buzzerTone(uint32_t frequency, uint32_t duration) {

  Wire1.beginTransmission(BUZZER_ADDR);

  // Frequency - 4 bytes
  Wire1.write((uint8_t*)&frequency, 4);

  // Duration - 4 bytes
  Wire1.write((uint8_t*)&duration, 4);

  byte error = Wire1.endTransmission();

  if (error != 0) {
    Serial.print("Buzzer I2C error: ");
    Serial.println(error);
  }
}


// =================================================
// Stop buzzer
// =================================================
void buzzerStop() {

  uint32_t zero = 0;

  Wire1.beginTransmission(BUZZER_ADDR);

  Wire1.write((uint8_t*)&zero, 4);
  Wire1.write((uint8_t*)&zero, 4);

  Wire1.endTransmission();
}


// =================================================
// Setup
// =================================================
void setup() {

  Serial.begin(115200);
  while (!Serial);

  Wire1.begin();
  Wire1.setClock(100000);

  Serial.println();
  Serial.println("HS3003 + Modulino Buzzer");
  Serial.println("--------------------------------");

  // Test buzzer
  Serial.println("Testing buzzer...");

  buzzerTone(2000, 1000);

  delay(1200);

  buzzerStop();

  Serial.println("Buzzer test complete.");
}


// =================================================
// Main loop
// =================================================
void loop() {

  // -----------------------------------------------
  // Check HS3003
  // -----------------------------------------------

  Wire1.beginTransmission(HS3003_ADDR);

  if (Wire1.endTransmission() != 0) {

    Serial.println("HS3003 not responding");

    buzzerStop();

    delay(1000);
    return;
  }

  delay(20);


  // -----------------------------------------------
  // Read HS3003
  // -----------------------------------------------

  Wire1.requestFrom(HS3003_ADDR, 4);

  if (Wire1.available() != 4) {

    Serial.println("HS3003 read error");

    buzzerStop();

    delay(1000);
    return;
  }


  uint8_t b1 = Wire1.read();
  uint8_t b2 = Wire1.read();
  uint8_t b3 = Wire1.read();
  uint8_t b4 = Wire1.read();


  // -----------------------------------------------
  // Convert sensor data
  // -----------------------------------------------

  uint16_t rawHumidity =
    ((b1 & 0x3F) << 8) | b2;

  uint16_t rawTemperature =
    ((b3 << 8) | b4) >> 2;


  float humidity =
    (rawHumidity * 100.0) / 16383.0;

  float temperature =
    (rawTemperature * 165.0) / 16383.0 - 40.0;


  // -----------------------------------------------
  // Display readings
  // -----------------------------------------------

  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.print(" C    ");

  Serial.print("Humidity: ");
  Serial.print(humidity, 2);
  Serial.println(" %");


  // -----------------------------------------------
  // Temperature alarm
  // -----------------------------------------------

  if (temperature >= HIGH_TEMP) {

    Serial.println("*** HIGH TEMPERATURE ***");

    // 2 kHz beep for 500 ms
    buzzerTone(2000, 500);

    delay(600);
  }


  // -----------------------------------------------
  // Humidity alarm
  // -----------------------------------------------

  else if (humidity >= HIGH_HUM) {

    Serial.println("*** HIGH HUMIDITY ***");

    // 1.5 kHz beep for 500 ms
    buzzerTone(1500, 500);

    delay(600);
  }


  // -----------------------------------------------
  // Normal
  // -----------------------------------------------

  else {

    buzzerStop();
  }


  delay(400);
}
