#include <Arduino.h>
#include <Wire.h>

#define IMU_ADDR       0x6A
#define LED_ADDR       0x39

#define CTRL1_XL       0x10
#define CTRL2_G        0x11
#define ACCEL_REG      0x28

#define MOVE_THRESHOLD 2000
#define FRAME_SIZE     12

int16_t oldAx = 0;
int16_t oldAy = 0;
int16_t oldAz = 0;


// ============================================================
// LED MATRIX
// ============================================================

void ledSend(const uint8_t *frame)
{
  Wire1.beginTransmission(LED_ADDR);
  Wire1.write(frame, FRAME_SIZE);
  Wire1.endTransmission();
}

void ledOn()
{
  uint8_t frame[12];
  memset(frame, 0xFF, sizeof(frame));
  ledSend(frame);
}

void ledOff()
{
  uint8_t frame[12] = {};
  ledSend(frame);
}

bool ledInit()
{
  uint8_t frame[12] =
    {'M','O','N',0,0,0,0,0,0,0,0,0};

  Wire1.beginTransmission(LED_ADDR);
  Wire1.write(frame, FRAME_SIZE);

  return Wire1.endTransmission() == 0;
}


// ============================================================
// IMU
// ============================================================

bool imuWrite(uint8_t reg, uint8_t value)
{
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(reg);
  Wire1.write(value);

  return Wire1.endTransmission() == 0;
}

bool imuRead(uint8_t reg, uint8_t *buf, uint8_t len)
{
  Wire1.beginTransmission(IMU_ADDR);
  Wire1.write(reg);

  if (Wire1.endTransmission(false) != 0)
    return false;

  if (Wire1.requestFrom(IMU_ADDR, len) != len)
    return false;

  for (uint8_t i = 0; i < len; i++)
    buf[i] = Wire1.read();

  return true;
}


// ============================================================
// MOVEMENT DETECTION
// ============================================================

bool movementDetected()
{
  uint8_t a[6];

  if (!imuRead(ACCEL_REG, a, 6))
    return false;

  int16_t ax = (a[1] << 8) | a[0];
  int16_t ay = (a[3] << 8) | a[2];
  int16_t az = (a[5] << 8) | a[4];

  int32_t dx = ax - oldAx;
  int32_t dy = ay - oldAy;
  int32_t dz = az - oldAz;

  oldAx = ax;
  oldAy = ay;
  oldAz = az;

  return abs(dx) > MOVE_THRESHOLD ||
         abs(dy) > MOVE_THRESHOLD ||
         abs(dz) > MOVE_THRESHOLD;
}


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  Wire1.begin();
  Wire1.setClock(400000);

  // Initialize IMU
  if (!imuWrite(CTRL1_XL, 0x40) ||
      !imuWrite(CTRL2_G, 0x40))
  {
    Serial.println("IMU FAIL");
    while (1);
  }

  // Initialize LED Matrix
  if (!ledInit())
  {
    Serial.println("LED MATRIX FAIL");
    while (1);
  }

  ledOff();

  Serial.println("MOVEMENT + LED TEST PASS");
}


// ============================================================
// LOOP
// ============================================================

void loop()
{
  bool movement = movementDetected();

  if (movement)
  {
    ledOn();
    Serial.println("MOVEMENT: YES  LED: ON");
  }
  else
  {
    ledOff();
    Serial.println("MOVEMENT: NO   LED: OFF");
  }

  delay(100);
}