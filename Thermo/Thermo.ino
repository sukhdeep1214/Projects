#include <Wire.h>
#include <Arduino_RouterBridge.h> 

#define HS3003_ADDR  0x44

// RPC Function exposed to Python. 
auto get_raw_sensor_data() {
  uint16_t rawTemp = 0;
  uint16_t rawHum = 0;
  bool success = false;

  // Step 1: Send a measurement wake-up request
  Wire1.beginTransmission(HS3003_ADDR);
  if (Wire1.endTransmission() == 0) {
    // Step 2: Mandatory hardware conversion delay
    delay(40);

    // Step 3: Request 4 structural registry bytes from the device address
    Wire1.requestFrom(HS3003_ADDR, 4);
    if (Wire1.available() == 4) {
      uint8_t buffer[4]; // Defined as array matrix
      for (int i = 0; i < 4; i++) {
        buffer[i] = Wire1.read();
      }

      // Isolate diagnostic status bits from the upper half of byte 0 (00 = valid data)
      uint8_t readStatus = (buffer[0] & 0xC0) >> 6;
      if (readStatus == 0) {
        // Isolate 14-bit payload fragment blocks
        rawHum = ((buffer[0] & 0x3F) << 8) | buffer[1];
        rawTemp = (buffer[2] << 6) | (buffer[3] >> 2);
        success = true; 
      }
    }
  }
  
  // Package and return the results using native standard C++ namespaces
  return std::make_tuple(success, rawTemp, rawHum);
}

void setup() {
  Serial.begin(115200);
  Wire1.begin();

  // Establish active interface bindings to the background MPU RPC daemon
  Bridge.begin();
  
  // Bind your physical retrieval method to a callable text keyword for Python
  Bridge.provide("get_raw_sensor_data", get_raw_sensor_data);

  Serial.println("MCU HS3003 State Driver Attached and Ready.");
}

void loop() {
  // Intentional empty loop: App Lab handles execution automatically in the background
}
