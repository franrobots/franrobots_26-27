#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

// ================================
// CONFIG
// ================================
#define TCA1_ADDR 0x70
#define TCA2_ADDR 0x71

#define NUM_SENSORS 11

// ================================
// MAPA DOS SENSORES
// ================================
struct SensorMap {
  uint8_t mux;
  uint8_t channel;
  const char* name;
};

SensorMap sensors[NUM_SENSORS] = {
  {TCA1_ADDR, 0, "S1"},
  {TCA1_ADDR, 1, "S2"},
  {TCA1_ADDR, 2, "S3"},
  {TCA1_ADDR, 3, "S4"},
  {TCA1_ADDR, 4, "S5"},
  {TCA1_ADDR, 5, "S6"},
  {TCA1_ADDR, 7, "S7"},
  {TCA2_ADDR, 0, "S8"},
  {TCA2_ADDR, 2, "S9"},
  {TCA2_ADDR, 7, "S10"},
  {TCA2_ADDR, 3, "S11"},
};

// ================================
// OBJETOS DOS SENSORES
// ================================
VL53L0X lox[NUM_SENSORS];
bool ok[NUM_SENSORS];

// ================================
// MUX SELECT
// ================================
void tcaSelect(uint8_t addr, uint8_t ch) {
  Wire.beginTransmission(addr);
  Wire.write(1 << ch);
  Wire.endTransmission();
}

// ================================
// DESLIGA TODOS
// ================================
void tcaDisable(uint8_t addr) {
  Wire.beginTransmission(addr);
  Wire.write(0);
  Wire.endTransmission();
}

// ================================
// INIT SENSORES
// ================================
void initSensors() {

  Serial.println("\nInicializando sensores...");

  tcaDisable(TCA1_ADDR);
  tcaDisable(TCA2_ADDR);
  delay(10);

  for (int i = 0; i < NUM_SENSORS; i++) {

  tcaDisable(TCA1_ADDR);
  tcaDisable(TCA2_ADDR);

  delayMicroseconds(100);

    Serial.print("Init ");
    Serial.print(sensors[i].name);
    Serial.print(" -> MUX 0x");
    Serial.print(sensors[i].mux, HEX);
    Serial.print(" CH ");
    Serial.println(sensors[i].channel);

    tcaSelect(sensors[i].mux, sensors[i].channel);
    delayMicroseconds(100);

    lox[i].setTimeout(50);

    if (!lox[i].init()) {
      Serial.println("❌ Falha");
      ok[i] = false;

      tcaDisable(TCA1_ADDR);
      tcaDisable(TCA2_ADDR);

      continue;
    }

    // Configuração otimizada
    lox[i].setMeasurementTimingBudget(20000); // 20ms (rápido)
    lox[i].startContinuous();

    ok[i] = true;
    Serial.println("✅ OK");

    tcaDisable(TCA1_ADDR);
    tcaDisable(TCA2_ADDR);
    delay(10);

  }

  tcaDisable(TCA1_ADDR);
  tcaDisable(TCA2_ADDR);
}

// ================================
// LEITURA
// ================================
uint16_t readSensor(uint8_t i) {

  if (!ok[i]) return 0;

  tcaDisable(TCA1_ADDR);
  tcaDisable(TCA2_ADDR);

  delayMicroseconds(100);

  tcaSelect(sensors[i].mux, sensors[i].channel);

    delayMicroseconds(100);

  uint16_t d = lox[i].readRangeContinuousMillimeters();

  if (lox[i].timeoutOccurred()) {
    return 0;
  }

  return d;
}

// ================================
// LEITURA DE TODOS
// ================================
void readAll(uint16_t *dist) {
  for (int i = 0; i < NUM_SENSORS; i++) {
    dist[i] = readSensor(i);
  }
}

// ================================
// DEBUG PRINT
// ================================
void printAll(uint16_t *dist) {
  Serial.println("\n===== DISTANCIAS =====");

  for (int i = 0; i < NUM_SENSORS; i++) {
    Serial.print(sensors[i].name);
    Serial.print(" Mux: ");
    Serial.print(sensors[i].mux);
    Serial.print(" Channel: ");
    Serial.print(sensors[i].channel);
    Serial.print(" Distance: ");
    

    if (dist[i] == 0) {
      Serial.println("ERRO");
    } else {
      Serial.print(dist[i]);
      Serial.println(" mm");
    }
  }
}

// ================================
// SETUP
// ================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin();
  Wire.setClock(400000);

  initSensors();
}

// ================================
// LOOP
// ================================
uint16_t dist[NUM_SENSORS];

void loop() {

  readAll(dist);

  printAll(dist);

  delay(10);
}