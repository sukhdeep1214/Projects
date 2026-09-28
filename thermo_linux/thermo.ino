#include <Wire.h>
#include <Arduino_RouterBridge.h> // Native UNO Q Router library

#define HS3003_ADDR 0x44

float liveTemperature = 0.0;
float liveHumidity = 0.0;
bool sensorError = false;

// Functions bound directly to the local routing registry map
float get_temperature() { return liveTemperature; }
float get_humidity() { return liveHumidity; }
bool is_sensor_faulty() { return sensorError; }

void setup() {
  Wire1.begin();
  Bridge.begin();
  
  // Register endpoints on the system router map
  Bridge.provide("get_temperature", get_temperature);
  Bridge.provide("get_humidity", get_humidity);
  Bridge.provide("is_sensor_faulty", is_sensor_faulty);
}

bool readHS3003(float &temperature, float &humidity) {
  Wire1.beginTransmission(HS3003_ADDR);
  if (Wire1.endTransmission() != 0) return false;

  delay(40); // Mandated HS3003 conversion window

  if (Wire1.requestFrom(HS3003_ADDR, 4) != 4) return false;

  uint8_t data[4] = {0}; 
  for (int i = 0; i < 4; i++) {
    data[i] = Wire1.read();
  }

  if ((data[0] & 0xC0) != 0x00) return false;

  uint16_t raw_hum = ((data[0] & 0x3F) << 8) | data[1];
  humidity = ((float)raw_hum / 16383.0) * 100.0;

  uint16_t raw_temp = (data[2] << 6) | ((data[3] & 0xFC) >> 2);
  temperature = ((float)raw_temp / 16383.0) * 165.0 - 40.0;

  return true;
}

void loop() {
  float t = 0.0, h = 0.0;
  if (readHS3003(t, h)) {
    liveTemperature = t;
    liveHumidity = h;
    sensorError = false;
  } else {
    sensorError = true;
  }
  delay(2000); 
}
