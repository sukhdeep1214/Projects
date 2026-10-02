#include <SPI.h>
#include <SD.h>
#include <ESP_I2S.h>

// ---------------- Pins ----------------
#define SD_CS    5
#define SD_SCK   18
#define SD_MISO  19
#define SD_MOSI  23

#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25

// ---------------- Settings ----------------
#define WAV_FILE "/test.wav"
#define VOLUME   0.5f          // 0.0 .. 1.0 (start low)
#define LOOP_PLAYBACK true     // replay forever

I2SClass i2s;

struct WavInfo {
  uint16_t channels;
  uint32_t sampleRate;
  uint16_t bits;
  uint32_t dataStart;
  uint32_t dataSize;
};

static uint32_t rd32(const uint8_t *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t rd16(const uint8_t *p) { return p[0] | (p[1] << 8); }

// Walks the RIFF chunks to find "fmt " and "data"
bool parseWav(File &f, WavInfo &w) {
  uint8_t h[12];
  if (f.read(h, 12) != 12) return false;
  if (memcmp(h, "RIFF", 4) != 0 || memcmp(h + 8, "WAVE", 4) != 0) {
    Serial.println("Not a RIFF/WAVE file");
    return false;
  }

  bool haveFmt = false;
  uint16_t format = 0;

  while (f.available()) {
    uint8_t ch[8];
    if (f.read(ch, 8) != 8) return false;
    uint32_t size = rd32(ch + 4);

    if (memcmp(ch, "fmt ", 4) == 0) {
      uint8_t fmt[16];
      if (size < 16 || f.read(fmt, 16) != 16) return false;
      format       = rd16(fmt);
      w.channels   = rd16(fmt + 2);
      w.sampleRate = rd32(fmt + 4);
      w.bits       = rd16(fmt + 14);
      haveFmt = true;
      if (size > 16) f.seek(f.position() + (size - 16));
    } else if (memcmp(ch, "data", 4) == 0) {
      if (!haveFmt) return false;
      w.dataStart = f.position();
      w.dataSize  = size;
      // PCM = 1, WAVE_FORMAT_EXTENSIBLE = 0xFFFE (accepted if 16-bit)
      return (format == 1 || format == 0xFFFE);
    } else {
      f.seek(f.position() + size + (size & 1));   // skip other chunks (padded to even)
    }
  }
  return false;
}

bool startI2S(uint32_t rate) {
  i2s.end();
  i2s.setPins(I2S_BCLK, I2S_LRC, I2S_DOUT);
  // Always stereo on the bus; mono files are duplicated in software
  return i2s.begin(I2S_MODE_STD, rate, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO);
}

void playFile() {
  File f = SD.open(WAV_FILE, FILE_READ);
  if (!f) {
    Serial.println("Cannot open " WAV_FILE " (is it in the card's root folder?)");
    return;
  }

  WavInfo w;
  if (!parseWav(f, w)) {
    Serial.println("Unsupported or invalid WAV (need PCM)");
    f.close();
    return;
  }
  Serial.printf("WAV: %u Hz, %u-bit, %u ch, %u bytes of audio\n",
                (unsigned)w.sampleRate, w.bits, w.channels, (unsigned)w.dataSize);

  if (w.bits != 16 || (w.channels != 1 && w.channels != 2)) {
    Serial.println("Only 16-bit mono/stereo WAV is supported");
    f.close();
    return;
  }
  if (w.sampleRate < 8000 || w.sampleRate > 96000) {
    Serial.println("Warning: MAX98357A supports 8000-96000 Hz");
  }
  if (!startI2S(w.sampleRate)) {
    Serial.println("I2S start failed");
    f.close();
    return;
  }

  f.seek(w.dataStart);
  uint32_t remaining = w.dataSize;

  static uint8_t  inBuf[1024];
  static int16_t  outBuf[1024];     // up to 512 stereo frames = 1024 samples

  Serial.println("Playing...");
  while (remaining > 0) {
    size_t want = remaining < sizeof(inBuf) ? remaining : sizeof(inBuf);
    size_t got = f.read(inBuf, want);
    if (got == 0) break;
    remaining -= got;

    int16_t *in = (int16_t *)inBuf;
    size_t outSamples = 0;

    if (w.channels == 2) {
      size_t n = got / 2;                       // samples (L,R,L,R...)
      for (size_t i = 0; i < n; i++)
        outBuf[outSamples++] = (int16_t)(in[i] * VOLUME);
    } else {
      size_t n = got / 2;                       // mono samples
      for (size_t i = 0; i < n; i++) {
        int16_t s = (int16_t)(in[i] * VOLUME);
        outBuf[outSamples++] = s;               // left
        outBuf[outSamples++] = s;               // right
      }
    }
    i2s.write((uint8_t *)outBuf, outSamples * sizeof(int16_t));
  }

  // Flush a little silence so the last samples are heard and the amp doesn't pop
  memset(outBuf, 0, sizeof(outBuf));
  for (int i = 0; i < 8; i++) i2s.write((uint8_t *)outBuf, sizeof(outBuf));

  f.close();
  Serial.println("Done");
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== SD WAV player ===");

  SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SPI, 4000000)) {
    Serial.println("SD mount failed. Check wiring, power voltage and FAT32 format.");
    while (true) delay(1000);
  }
  Serial.printf("SD OK, card size %llu MB\n", SD.cardSize() / (1024ULL * 1024ULL));

  File root = SD.open("/");
  Serial.println("Files in root:");
  while (File e = root.openNextFile()) {
    Serial.printf("  %s  (%u bytes)\n", e.name(), (unsigned)e.size());
    e.close();
  }
  root.close();
}

void loop() {
  playFile();
  if (!LOOP_PLAYBACK) while (true) delay(1000);
  delay(1000);
}