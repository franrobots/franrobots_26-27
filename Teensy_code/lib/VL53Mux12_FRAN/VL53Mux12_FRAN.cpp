#include "VL53Mux12_FRAN.h"

namespace {
// constexpr uint8_t kSensorCount = (uint8_t)ToFId::COUNT;
constexpr uint16_t kTimeoutMs = 50;
constexpr uint32_t kTimingBudgetUs = 20000;
// const char* const kSensorNames[kSensorCount] = {
//   "FL", "FC", "FR",
//   "LF", "LC", "LB",
//   "BL",       "BR",
//   "RF", "RC", "RB"
// };
}
// Derivado do enum, nao hardcoded: se ToFId ganhar/perder um sensor,
// a tabela abaixo quebra na compilacao em vez de silenciosamente
// desalinhar os indices.
constexpr uint8_t kSensorCount = (uint8_t)ToFId::COUNT;

struct SensorMap {
  uint8_t mux;
  uint8_t channel;
  const char* name;
};

#define TCA1_ADDR 0x70
#define TCA2_ADDR 0x71

// ATENCAO: a ordem desta tabela DEVE casar com enum class ToFId (VL53Mux12_FRAN.h).
// readSensorByIndex(i) le o canal de kSensorNames[i] e grava em _mm[i], entao
// o indice i tem que ser igual a (uint8_t)ToFId. Nao reordene um sem o outro.
SensorMap kSensorNames[kSensorCount] = {
  {TCA1_ADDR, 7, "FL"}, // idx 0  -> ToFId::FL   (era S7)
  {TCA1_ADDR, 5, "FC"}, // idx 1  -> ToFId::FC   (era S6)
  {TCA1_ADDR, 4, "FR"}, // idx 2  -> ToFId::FR   (era S5)
  {TCA2_ADDR, 0, "LF"}, // idx 3  -> ToFId::LF   (era S8)
  {TCA2_ADDR, 2, "LC"}, // idx 4  -> ToFId::LC   (era S9)
  {TCA2_ADDR, 4, "LB"}, // idx 5  -> ToFId::LB   (era S10)
  {TCA2_ADDR, 1, "BL"}, // idx 6  -> ToFId::BL   (era S11)
  {TCA1_ADDR, 0, "BR"}, // idx 7  -> ToFId::BR   (era S1)
  {TCA1_ADDR, 3, "RF"}, // idx 8  -> ToFId::RF   (era S4)
  {TCA1_ADDR, 2, "RC"}, // idx 9  -> ToFId::RC   (era S3)
  {TCA1_ADDR, 1, "RB"}  // idx 10 -> ToFId::RB   (era S2)
};

static_assert(sizeof(kSensorNames) / sizeof(kSensorNames[0]) == (size_t)ToFId::COUNT,
              "kSensorNames precisa ter uma entrada por ToFId, na mesma ordem do enum");

VL53Mux12_FRAN::VL53Mux12_FRAN(uint8_t addrA, uint8_t addrB)
: _addrA(addrA), _addrB(addrB) {}

void VL53Mux12_FRAN::printAll(const uint16_t* dist) const {
  Serial.println("\n===== DISTANCIAS =====");

  for (int i = 0; i < kSensorCount; i++) {
    Serial.print(kSensorNames[i].name);
    Serial.print(" Mux: ");
    Serial.print(kSensorNames[i].mux);
    Serial.print(" Channel: ");
    Serial.print(kSensorNames[i].channel);
    Serial.print(" Distance: ");
    

    if (dist[i] == 0) {
      Serial.println("ERRO");
    } else {
      Serial.print(dist[i]);
      Serial.println(" mm");
    }
  }
}


void VL53Mux12_FRAN::tcaDisable(uint8_t addr) {
  if (!_wire) return;
  _wire->beginTransmission(addr);
  _wire->write(0);
  _wire->endTransmission();
}

void VL53Mux12_FRAN::tcaDisableAll() {
  tcaDisable(_addrA);
  tcaDisable(_addrB);
}

void VL53Mux12_FRAN::tcaSelect(uint8_t addr, uint8_t ch) {
  tcaDisableAll();
  delayMicroseconds(100);

  _wire->beginTransmission(addr);
  _wire->write(1u << ch);
  _wire->endTransmission();

  delayMicroseconds(100);
}

bool VL53Mux12_FRAN::begin(TwoWire& wire, uint32_t clk, uint16_t periodMs) {
  _wire = &wire;
  _wire->begin();
  _wire->setClock(clk);
  _period = periodMs;
  _rr = 0;

  for (uint8_t i = 0; i < kSensorCount; i++) {
    _raw[i] = 0;
    _cal[i] = 0;
    _mm[i] = 0;
    _ok[i] = false;
    _last[i] = 0;
    _sensorOk[i] = false;
    _offset[i] = 0;
    _ema[i] = 0.0f;
    _emaInit[i] = false;
  }

  bool allOk = true;

  tcaDisableAll();
  delay(10);

  for (uint8_t i = 0; i < kSensorCount; i++) {
    const uint8_t mux = kSensorNames[i].mux;
    const uint8_t ch = kSensorNames[i].channel;

    tcaDisableAll();
    delayMicroseconds(100);

    tcaSelect(mux, ch);
    delayMicroseconds(100);

    _sensor[i].setTimeout(kTimeoutMs);

    if (!_sensor[i].init()) {
      _sensorOk[i] = false;
      allOk = false;
      tcaDisableAll();
      delayMicroseconds(100);
      continue;
    }

    _sensor[i].setMeasurementTimingBudget(kTimingBudgetUs);
    if (_period == 0) _sensor[i].startContinuous();
    else _sensor[i].startContinuous(_period);

    _sensorOk[i] = true;
  }

  tcaDisableAll();
  return allOk;
}

void VL53Mux12_FRAN::getSensorOk() const {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    Serial.print(kSensorNames[i].name);
    Serial.print(": ");
    Serial.println(_sensorOk[i] ? "OK" : "FAIL");
  }
}

bool VL53Mux12_FRAN::readSensorByIndex(uint8_t i) {
  if (i >= kSensorCount || !_sensorOk[i]) {
    if (i < kSensorCount) {
      _raw[i] = 0;
      _cal[i] = 0;
      _mm[i] = 0;
      _ok[i] = false;
    }
    return false;
  }

  const uint8_t mux = kSensorNames[i].mux;
  //const uint8_t ch = (i < 7) ? i : (uint8_t)(i - 7);
  const uint8_t ch = kSensorNames[i].channel;

  tcaSelect(mux, ch);

  const uint16_t raw = _sensor[i].readRangeContinuousMillimeters();
  if (_sensor[i].timeoutOccurred()) {
    _raw[i] = 0;
    _cal[i] = 0;
    _mm[i] = 0;
    _ok[i] = false;
    return false;
  }

  _raw[i] = raw;
  _cal[i] = (raw > _offset[i]) ? (uint16_t)(raw - _offset[i]) : 0;

  if (!_emaInit[i]) {
    _ema[i] = _cal[i];
    _emaInit[i] = true;
  } else {
    _ema[i] += _alpha * (_cal[i] - _ema[i]);
  }

  _mm[i] = (uint16_t)(_ema[i] + 0.5f);
  _ok[i] = true;
  _last[i] = millis();
  return true;
}

uint16_t VL53Mux12_FRAN::readSensor(ToFId id) {
  const uint8_t i = (uint8_t)id;
  if (!readSensorByIndex(i)) return 0;
  return _mm[i];
}

void VL53Mux12_FRAN::readAll() {
  for (uint8_t i = 0; i < kSensorCount; i++) {
    readSensorByIndex(i);
  }
}

bool VL53Mux12_FRAN::update() {
  const uint8_t i = _rr;
  _rr = (uint8_t)((_rr + 1) % kSensorCount);
  return readSensorByIndex(i);
}

void VL53Mux12_FRAN::snapshot(ScanToF12& out) const {
  const uint32_t now = millis();
  for (uint8_t i = 0; i < kSensorCount; i++) {
    out.mm[i] = _mm[i];
    out.ok[i] = _ok[i];
    out.age_ms[i] = _last[i] ? (now - _last[i]) : 0;
  }
}

void VL53Mux12_FRAN::setOffset(ToFId id, int16_t off) {
  _offset[(uint8_t)id] = off;
}

void VL53Mux12_FRAN::setEmaAlpha(float a) {
  if (a < 0.05f) a = 0.05f;
  if (a > 1.0f) a = 1.0f;
  _alpha = a;
}

uint16_t VL53Mux12_FRAN::getRaw(ToFId id) const { return _raw[(uint8_t)id]; }
uint16_t VL53Mux12_FRAN::getCal(ToFId id) const { return _cal[(uint8_t)id]; }
uint16_t VL53Mux12_FRAN::get(ToFId id) const { return _mm[(uint8_t)id]; }
bool VL53Mux12_FRAN::valid(ToFId id) const { return _ok[(uint8_t)id]; }
bool VL53Mux12_FRAN::sensorInitOk(ToFId id) const { return _sensorOk[(uint8_t)id]; }
