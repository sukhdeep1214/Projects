#include <Wire.h>

#define HS3003_ADDR 0x44

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Wire1.begin();   
  Serial.println("HS3003 Test (Wire1)");
}

void loop() {

  Wire1.beginTransmission(HS3003_ADDR);

  if (Wire1.endTransmission() != 0) {
    Serial.println("Sensor not responding");
    delay(1000);
    return;
  }

  delay(20);

  Wire1.requestFrom(HS3003_ADDR, 4);

  if (Wire1.available() == 4) {
    uint8_t b1 = Wire1.read();
    uint8_t b2 = Wire1.read();
    uint8_t b3 = Wire1.read();
    uint8_t b4 = Wire1.read();

    uint16_t rawHumidity = ((b1 & 0x3F) << 8) | b2;
    uint16_t rawTemperature = ((b3 << 8) | b4) >> 2;

    float humidity = (rawHumidity * 100.0) / 16383.0;
    float temperature = (rawTemperature * 165.0) / 16383.0 - 40.0;

    Serial.print("Temp: "); Serial.println(temperature);
    Serial.print("Hum: "); Serial.println(humidity);
  }

  delay(1000);
}