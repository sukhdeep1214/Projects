#include <Wire.h>

#define HS3003_ADDR 0x44

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;

  Wire.begin();  // Initialize I²C

  Serial.println("HS3003 Low-Level I2C Test");
}

void loop() {

  // Request a measurement by initiating communication
  Wire.beginTransmission(HS3003_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("Sensor not responding");
    delay(1000);
    return;
  }

  delay(20);  // Wait for measurement

  // Read 4 bytes returned by the sensor
  Wire.requestFrom(HS3003_ADDR, 4);

  if (Wire.available() == 4) {

    uint8_t b1 = Wire.read();
    uint8_t b2 = Wire.read();
    uint8_t b3 = Wire.read();
    uint8_t b4 = Wire.read();

    // Decode humidity (14 bits)
    uint16_t rawHumidity =
      ((b1 & 0x3F) << 8) | b2;

    // Decode temperature (14 bits)
    uint16_t rawTemperature =
      ((b3 << 8) | b4) >> 2;

    float humidity =
      (rawHumidity * 100.0) / 16383.0;

    float temperature =
      (rawTemperature * 165.0) / 16383.0 - 40.0;

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.println("---------------------");
  }

  delay(1000);
}
