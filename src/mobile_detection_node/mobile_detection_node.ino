/*
  Mobile Phone Detection System - ESP32 Detection Node

  Hardware:
    ESP32
    HMC5883L-compatible magnetometer
    nRF24L01
    16x2 parallel LCD
    Buzzer

  Detection logic follows the methodology described in the
  associated project paper:
    - 100 samples for baseline calibration
    - 75 Hz monitoring target
    - Euclidean magnetic-field deviation
    - 25 mG threshold (configurable)
    - 5 consecutive detections for debounce

  Sensor:
  A direct-I2C HMC5883L driver is included (no external library).
  If your "GY-271" board is actually a QMC5883L clone (I2C address
  0x0D), initMagnetometer() will report an ID mismatch; in that case
  replace initMagnetometer()/readMagnetometer() only. The detection
  logic does not depend on the sensor driver.

  Board package: ESP32 Arduino core 2.x or 3.x (buzzer code handles both).
*/

#include <Wire.h>
#include <SPI.h>
#include <RF24.h>
#include <LiquidCrystal.h>
#include <math.h>

// -------------------- Pins --------------------
#define HMC_SDA 21
#define HMC_SCL 22

#define NRF_CE  4
#define NRF_CSN 5

#define BUZZER_PIN 27

// Parallel 16x2 LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(14, 13, 26, 25, 33, 32);

RF24 radio(NRF_CE, NRF_CSN);

// -------------------- Radio --------------------
const uint8_t NODE_ID = 1;

// Must match the corresponding receiver pipe. Valid IDs: 1..6.
// nRF24L01 pipes 2-5 share their upper 4 address bytes with pipe 1 and
// differ only in the FIRST byte (RF24 stores the LSB first), so the
// addresses must differ in their first character, not their last.
const byte RADIO_ADDRESSES[6][6] = {
  "1NODE", "2NODE", "3NODE", "4NODE", "5NODE", "6NODE"
};

// -------------------- Detection --------------------
const int CALIBRATION_SAMPLES = 100;
const int REQUIRED_HITS = 5;
const float THRESHOLD_MG = 25.0f;

// Baseline magnetic vector.
float baseX = 0.0f;
float baseY = 0.0f;
float baseZ = 0.0f;

int consecutiveHits = 0;

struct __attribute__((packed)) AlertPacket {
  uint8_t nodeId;
  uint8_t detected;
  uint32_t timestampMs;
  float deltaB;
};

// -------------------- HMC5883L (direct I2C) --------------------
const uint8_t HMC_ADDR = 0x1E;
const float   HMC_MG_PER_LSB = 0.92f;   // gain 1.3 Ga -> 1090 LSB/Gauss
const uint32_t CALIBRATION_TIMEOUT_MS = 5000;

void hmcWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(HMC_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

bool initMagnetometer() {
  // Identification registers 0x0A..0x0C must read 'H','4','3'.
  Wire.beginTransmission(HMC_ADDR);
  Wire.write(0x0A);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom((int)HMC_ADDR, 3) != 3) return false;
  const char a = Wire.read();
  const char b = Wire.read();
  const char c = Wire.read();
  if (a != 'H' || b != '4' || c != '3') return false;

  hmcWrite(0x00, 0x78);  // 8-sample averaging, 75 Hz output rate, normal bias
  hmcWrite(0x01, 0x20);  // gain: +/-1.3 Ga (1090 LSB/Gauss)
  hmcWrite(0x02, 0x00);  // continuous-measurement mode
  delay(10);
  return true;
}

// Returns true only when a new, valid sample is available. Output in mG.
bool readMagnetometer(float &x, float &y, float &z) {
  // Status register: bit 0 = data ready
  Wire.beginTransmission(HMC_ADDR);
  Wire.write(0x09);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom((int)HMC_ADDR, 1) != 1) return false;
  if (!(Wire.read() & 0x01)) return false;

  // Data registers start at 0x03 in the order X, Z, Y (not X, Y, Z).
  Wire.beginTransmission(HMC_ADDR);
  Wire.write(0x03);
  if (Wire.endTransmission() != 0) return false;
  if (Wire.requestFrom((int)HMC_ADDR, 6) != 6) return false;

  const int16_t rx = (int16_t)((Wire.read() << 8) | Wire.read());
  const int16_t rz = (int16_t)((Wire.read() << 8) | Wire.read());
  const int16_t ry = (int16_t)((Wire.read() << 8) | Wire.read());

  // -4096 marks ADC overflow/saturation: discard the sample.
  if (rx == -4096 || ry == -4096 || rz == -4096) return false;

  x = rx * HMC_MG_PER_LSB;
  y = ry * HMC_MG_PER_LSB;
  z = rz * HMC_MG_PER_LSB;
  return true;
}

void showStatus(const char *line1, const char *line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void haltWithError(const char *line1, const char *line2) {
  showStatus(line1, line2);
  while (true) delay(1000);
}

void calibrateBaseline() {
  showStatus("Calibrating...", "Keep area clear");

  float sumX = 0.0f;
  float sumY = 0.0f;
  float sumZ = 0.0f;
  int validSamples = 0;

  const uint32_t calStart = millis();

  while (validSamples < CALIBRATION_SAMPLES) {
    float x, y, z;

    if (readMagnetometer(x, y, z)) {
      sumX += x;
      sumY += y;
      sumZ += z;
      validSamples++;
    }

    if (millis() - calStart > CALIBRATION_TIMEOUT_MS) {
      Serial.println("Calibration timed out: no valid magnetometer data.");
      haltWithError("SENSOR ERROR", "No data");
    }

    delay(13); // Approximately 75 Hz target sampling interval.
  }

  baseX = sumX / CALIBRATION_SAMPLES;
  baseY = sumY / CALIBRATION_SAMPLES;
  baseZ = sumZ / CALIBRATION_SAMPLES;

  showStatus("Calibration OK", "Monitoring...");
  delay(800);
}

void sendAlert(float deltaB) {
  AlertPacket packet;
  packet.nodeId = NODE_ID;
  packet.detected = 1;
  packet.timestampMs = millis();
  packet.deltaB = deltaB;

  const bool ok = radio.write(&packet, sizeof(packet));
  Serial.println(ok ? "Alert sent (ACK received)." : "Alert send failed (no ACK).");
}

void beep() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  tone(BUZZER_PIN, 2200, 500);
#else
  ledcWriteTone(0, 2200);   // core 2.x: LEDC channel 0
  delay(500);
  ledcWriteTone(0, 0);
#endif
}

void triggerLocalAlert(float deltaB) {
  showStatus("MOBILE DETECTED", "Alert sent");

  beep();
  sendAlert(deltaB);

  delay(700);
  showStatus("Monitoring...", "Status: READY");
}

void setup() {
  Serial.begin(115200);

  if (NODE_ID < 1 || NODE_ID > 6) {
    Serial.println("NODE_ID must be 1..6.");
    while (true) delay(1000);
  }

  pinMode(BUZZER_PIN, OUTPUT);
#if !(defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3))
  ledcSetup(0, 2000, 8);
  ledcAttachPin(BUZZER_PIN, 0);
#endif

  lcd.begin(16, 2);
  showStatus("Mobile Detector", "Starting...");

  Wire.begin(HMC_SDA, HMC_SCL);

  if (!initMagnetometer()) {
    Serial.println("HMC5883L not found (check wiring / QMC5883L clone?).");
    haltWithError("SENSOR ERROR", "Check HMC5883L");
  }

  if (!radio.begin()) {
    showStatus("NRF24 ERROR", "Check wiring");
    Serial.println("nRF24L01 initialization failed.");
    while (true) {
      delay(1000);
    }
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_1MBPS);
  radio.setPayloadSize(sizeof(AlertPacket));
  radio.openWritingPipe(RADIO_ADDRESSES[NODE_ID - 1]);
  radio.stopListening();

  calibrateBaseline();
}

void loop() {
  const uint32_t sampleStart = millis();

  float x, y, z;

  if (readMagnetometer(x, y, z)) {
    const float dx = x - baseX;
    const float dy = y - baseY;
    const float dz = z - baseZ;

    const float deltaB = sqrtf(dx * dx + dy * dy + dz * dz);

    Serial.print("Delta B: ");
    Serial.print(deltaB, 2);
    Serial.println(" mG");

    if (deltaB > THRESHOLD_MG) {
      consecutiveHits++;

      if (consecutiveHits >= REQUIRED_HITS) {
        triggerLocalAlert(deltaB);
        consecutiveHits = 0;
      }
    } else {
      consecutiveHits = 0;
    }
  }

  // Target about 75 samples per second.
  const uint32_t elapsed = millis() - sampleStart;
  if (elapsed < 13) {
    delay(13 - elapsed);
  }
}
