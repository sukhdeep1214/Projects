// ESP32: stream /test.wav from an SD card to a MAX98357A using the Arduino ESP_I2S library.
// Requires ESP32 Arduino core 3.x. Assumes a canonical 44-byte WAV header (16-bit PCM).

#include "SD.h"
#include "ESP_I2S.h"

#define SD_CS           5
#define I2S_BCLK        27
#define I2S_LRC         26
#define I2S_DOUT        25

#define WAV_FILE        "/test.wav"
#define WAV_HEADER_SIZE 44
#define BUFFER_SIZE     512

I2SClass i2s;
File audioFile;
uint8_t audioBuffer[BUFFER_SIZE];

static void halt(const char *msg) {
  Serial.println(msg);
  while (true) delay(1000);
}

void setup() {
  Serial.begin(115200);

  if (!SD.begin(SD_CS)) halt("SD card mount failed");

  audioFile = SD.open(WAV_FILE, FILE_READ);
  if (!audioFile) halt("Failed to open " WAV_FILE);

  // Read channel count and sample rate from the WAV header
  uint8_t header[WAV_HEADER_SIZE];
  if (audioFile.read(header, WAV_HEADER_SIZE) != WAV_HEADER_SIZE) halt("WAV header too short");
  uint16_t channels = header[22] | (header[23] << 8);
  uint32_t rate = header[24] | (header[25] << 8) | (header[26] << 16) | ((uint32_t)header[27] << 24);

  i2s.setPins(I2S_BCLK, I2S_LRC, I2S_DOUT);
  i2s_slot_mode_t slots = (channels == 2) ? I2S_SLOT_MODE_STEREO : I2S_SLOT_MODE_MONO;
  if (!i2s.begin(I2S_MODE_STD, rate, I2S_DATA_BIT_WIDTH_16BIT, slots, I2S_STD_SLOT_BOTH))
    halt("I2S init failed");

  Serial.printf("Streaming %s: %u Hz, %u channel(s)\n", WAV_FILE, (unsigned)rate, channels);
}

void loop() {
  size_t n = audioFile.read(audioBuffer, BUFFER_SIZE);
  if (n > 0) {
    i2s.write(audioBuffer, n);        // blocks until DMA has room, which paces the loop
    return;
  }

  Serial.println("Track ended, restarting");
  audioFile.seek(WAV_HEADER_SIZE);    // back to the start of the audio data
  delay(1000);
}
