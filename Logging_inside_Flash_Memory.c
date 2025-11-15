#include <SPI.h>
#include <Adafruit_SPIFlash.h>

// Define a structure for sensor data (using small data types for efficiency)
struct SensorData {
    uint32_t timestamp;  // 4 bytes: Store time in seconds/milliseconds
    int16_t temperature; // 2 bytes: Store in 0.01 degree increments (e.g., 2550 for 25.50C)
    uint16_t humidity;   // 2 bytes: Store in 0.01% increments
    uint8_t battery;     // 1 byte: Store percentage (0-100)
    // Total size: 9 bytes per entry (highly compressed compared to strings or floats)
};

// Use the default external flash object (connect to standard SPI pins)
// You might need to adjust this depending on your specific board/chip
Adafruit_SPIFlash flash;

#define FLASH_SS 10 // Chip select pin for the flash chip

uint32_t logAddress = 0; // Start address for logging

void setup() {
    Serial.begin(115200);
    Serial.println("Flash Memory Logger Starting...");

    // Initialize flash memory
    if (!flash.begin(FLASH_SS)) {
        Serial.println("Error: Could not find or initialize flash chip.");
        while (1);
    }
    Serial.print("Flash chip size: ");
    Serial.print(flash.size() / 1024);
    Serial.println(" KB");

    // Start logging from the beginning of the memory
    logAddress = 0;
    // Optional: Erase the entire chip (be careful, this takes time)
    // Serial.println("Erasing flash chip...");
    // flash.eraseChip();
    // Serial.println("Erase complete.");
}

void loop() {
    // 1. Simulate sensor data
    SensorData data;
    data.timestamp = millis() / 1000; // Time in seconds
    data.temperature = analogRead(A0); // Read analog, scale/convert later
    data.humidity = analogRead(A1);    // Read analog, scale/convert later
    data.battery = 95;                 // Example value

    // 2. Log data to flash
    if (logAddress + sizeof(data) < flash.size()) {
        // Write the data struct directly to flash
        if (flash.write(logAddress, (uint8_t*)&data, sizeof(data))) {
            Serial.print("Logged data to address: 0x");
            Serial.println(logAddress, HEX);
            logAddress += sizeof(data); // Move to the next log position
        } else {
            Serial.println("Error writing to flash!");
        }
    } else {
        Serial.println("Flash memory full! Stop logging or implement wear leveling/wrap-around.");
        while (1); // Stop on full for this simple example
    }

    // 3. Smart Technique: Compression & Data Filtering
    // Only log if the data has changed significantly to save space
    // (This requires storing the last value and comparing before logging)
    
    // 4. Implement a read function to verify
    // readAndVerify(logAddress - sizeof(data));

    delay(5000); // Log every 5 seconds
}

void readAndVerify(uint32_t address) {
    SensorData readData;
    if (flash.read(address, (uint8_t*)&readData, sizeof(readData))) {
        Serial.print("Verified Read from 0x");
        Serial.print(address, HEX);
        Serial.print(": Time=");
        Serial.print(readData.timestamp);
        Serial.print(", Temp=");
        Serial.print(readData.temperature);
        Serial.println("...");
    }
}