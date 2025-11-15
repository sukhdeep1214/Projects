// Define pins
const int tempSensorPin = A0; // Analog pin for LM35 temperature sensor
const int fanPin = 9;         // Digital pin for fan control (must be PWM capable)

// Define temperature thresholds and corresponding fan speeds
const int tempThresholdLow = 25; // Temperature below which fan is off
const int tempThresholdMid = 28; // Temperature for medium fan speed
const int tempThresholdHigh = 32; // Temperature for high fan speed

void setup() {
  pinMode(fanPin, OUTPUT); // Set fan pin as output
  Serial.begin(9600);      // Initialize serial communication for debugging
}

void loop() {
  // Read temperature from LM35 sensor
  int sensorValue = analogRead(tempSensorPin);
  float voltage = sensorValue * (5.0 / 1023.0); // Convert raw reading to voltage
  float temperatureC = (voltage - 0.5) * 100.0; // Convert voltage to Celsius for LM35

  // Print temperature to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.println(" °C");

  // Control fan speed based on temperature
  if (temperatureC < tempThresholdLow) {
    analogWrite(fanPin, 0); // Fan OFF
    Serial.println("Fan OFF");
  } else if (temperatureC < tempThresholdMid) {
    analogWrite(fanPin, 100); // Low speed (e.g., 40% PWM)
    Serial.println("Fan Speed: Low");
  } else if (temperatureC < tempThresholdHigh) {
    analogWrite(fanPin, 180); // Medium speed (e.g., 70% PWM)
    Serial.println("Fan Speed: Medium");
  } else {
    analogWrite(fanPin, 255); // Full speed (100% PWM)
    Serial.println("Fan Speed: High");
  }

  delay(1000); // Wait for 1 second before next reading
}
