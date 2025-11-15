// LED and sensor pin definitions
int redLED = 8;
int yellowLED = 9;
int greenLED = 10;
int carSensor = 7; // Digital pin for the IR sensor

// Timing variables (in milliseconds)
int redDuration = 10000;
int greenDuration = 15000;
int yellowDuration = 3000;

// Function to simulate a single traffic light cycle
void trafficLightCycle() {
  // Red light phase
  digitalWrite(redLED, HIGH);
  delay(redDuration);
  digitalWrite(redLED, LOW);

  // Green light phase with sensor-based extension
  digitalWrite(greenLED, HIGH);
  long startTime = millis(); // Record the start time
  while (millis() - startTime < greenDuration) {
    // If a car is detected, extend the green light duration by a little
    if (digitalRead(carSensor) == HIGH) {
      greenDuration += 2000; // Add 2 seconds
      startTime = millis(); // Reset the timer
      delay(100); // Small delay to avoid rapid re-triggering
    }
    delay(100);
  }
  digitalWrite(greenLED, LOW);

  // Yellow light phase
  digitalWrite(yellowLED, HIGH);
  delay(yellowDuration);
  digitalWrite(yellowLED, LOW);
}

// Setup function runs once
void setup() {
  pinMode(redLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(carSensor, INPUT); // Set the sensor pin as an input
}

// Loop function runs continuously
void loop() {
  trafficLightCycle();
}