#include "ColorCalibration.h"
#include "LedStrip.h"

static constexpr uint32_t COLOR_MAGIC = 0xC010CA1u;
static constexpr uint16_t COLOR_EEPROM_ADDR = 128;
static constexpr uint8_t COLOR_VERSION = 1;
static constexpr uint8_t CAPTURE_SAMPLES = 80;

struct ColorBlob {
  uint32_t magic;
  uint8_t version;
  ColorThreshold black;
  ColorThreshold blue;
  ColorThreshold red;
  ColorThreshold silver;
};

struct CaptureBounds {
  uint16_t ch0Min = 65535;
  uint16_t ch0Max = 0;
  uint16_t ch3Min = 65535;
  uint16_t ch3Max = 0;
};

static void waitButtonPressAndRelease(uint8_t buttonPin) {
  while (digitalRead(buttonPin) == HIGH) delay(5);
  delay(80);
  while (digitalRead(buttonPin) == LOW) delay(5);
  delay(80);
}

static uint16_t withMarginMin(uint16_t v) {
  const uint16_t margin = (v > 80) ? (uint16_t)(v * 0.08f) : 20;
  return (v > margin) ? (v - margin) : 0;
}

static uint16_t withMarginMax(uint16_t v) {
  const uint16_t margin = (v > 80) ? (uint16_t)(v * 0.08f) : 20;
  const uint32_t out = (uint32_t)v + margin;
  return (out > 65535u) ? 65535u : (uint16_t)out;
}

static ColorThreshold captureColor(ReflectancePlate& plate,
                                   uint8_t c9Idx,
                                   uint8_t auxIdx,
                                   bool ratioMode,
                                   float ratioScale) {
  CaptureBounds b;

  for (uint8_t i = 0; i < CAPTURE_SAMPLES; i++) {
    plate.read(1, 350);

    const uint16_t c9 = plate.value(c9Idx);
    const uint16_t aux = plate.value(auxIdx);

    if (ratioMode) {
      const float denom = (aux > 0) ? (float)aux : 1.0f;
      const float r = ((float)c9 / denom) * ratioScale;
      const uint16_t ratio = (uint16_t)(r + 0.5f);
      if (ratio < b.ch0Min) b.ch0Min = ratio;
      if (ratio > b.ch0Max) b.ch0Max = ratio;
    } else {
      if (c9 < b.ch0Min) b.ch0Min = c9;
      if (c9 > b.ch0Max) b.ch0Max = c9;
      if (aux < b.ch3Min) b.ch3Min = aux;
      if (aux > b.ch3Max) b.ch3Max = aux;
    }
  }

  ColorThreshold t;
  t.ch0_min = withMarginMin(b.ch0Min);
  t.ch0_max = withMarginMax(b.ch0Max);
  if (ratioMode) {
    t.ch3_min = 0;
    t.ch3_max = 0;
  } else {
    t.ch3_min = withMarginMin(b.ch3Min);
    t.ch3_max = withMarginMax(b.ch3Max);
  }
  t.maxAbsTiltDeg = 0.0f;
  return t;
}

void ColorCalibration::run(ReflectancePlate& plate,
                           uint8_t buttonPin,
                           LedStrip *ledStrip,
                           uint8_t c9Idx,
                           uint8_t auxIdx,
                           bool ratioMode,
                           float ratioScale) {
  pinMode(buttonPin, INPUT_PULLUP);

  plate.setDecisionChannels(c9Idx, auxIdx);
  plate.setMatchMode(ratioMode ? ReflectanceMatchMode::RATIO : ReflectanceMatchMode::ABSOLUTE);
  plate.setRatioThresholdScale(ratioScale);

  ColorBlob blob;
  blob.magic = COLOR_MAGIC;
  blob.version = COLOR_VERSION;

  if (ledStrip) {
    ledStrip->clear(); 
    ledStrip->update();
  }
  waitButtonPressAndRelease(buttonPin);
  blob.black = captureColor(plate, c9Idx, auxIdx, ratioMode, ratioScale);

  if (ledStrip) {
    ledStrip->setColor(LedSide::ALL, LedColor::BLUE); // Acende Azul
    ledStrip->update();
  }
  waitButtonPressAndRelease(buttonPin);
  blob.blue = captureColor(plate, c9Idx, auxIdx, ratioMode, ratioScale);

  if (ledStrip) {
    ledStrip->setColor(LedSide::ALL, LedColor::RED); // Acende Vermelho
    ledStrip->update();
  }
  waitButtonPressAndRelease(buttonPin);
  blob.red = captureColor(plate, c9Idx, auxIdx, ratioMode, ratioScale);

  if (ledStrip) {
    ledStrip->setColor(LedSide::ALL, LedColor::YELLOW); // Amarelo/Brilhante simulando Prata
    ledStrip->update();
  }
  waitButtonPressAndRelease(buttonPin);
  blob.silver = captureColor(plate, c9Idx, auxIdx, ratioMode, ratioScale);

  EEPROM.put(COLOR_EEPROM_ADDR, blob);

  plate.setBlackThreshold(blob.black);
  plate.setBlueThreshold(blob.blue);
  plate.setRedThreshold(blob.red);
  plate.setSilverThreshold(blob.silver);

  if (ledStrip) {
    ledStrip->clear();
    ledStrip->update();
  }
}

bool ColorCalibration::load(ReflectancePlate& plate) {
  ColorBlob blob;
  EEPROM.get(COLOR_EEPROM_ADDR, blob);
  if (blob.magic != COLOR_MAGIC || blob.version != COLOR_VERSION) return false;

  plate.setBlackThreshold(blob.black);
  plate.setBlueThreshold(blob.blue);
  plate.setRedThreshold(blob.red);
  plate.setSilverThreshold(blob.silver);
  return true;
}
