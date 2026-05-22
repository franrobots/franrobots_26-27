#pragma once
#include <Arduino.h>

/*
=========================================================
                TEENSY 4.0 – ROBO MAZE
=========================================================
Autor: Breno
Plataforma: Teensy 4.0 (i.MX RT1062)
Ambiente: PlatformIO
=========================================================
*/

// ======================================================
// 🔵 I2C BUS
// ======================================================

constexpr uint8_t I2C_SDA = 18;
constexpr uint8_t I2C_SCL = 19;


// ======================================================
// 🔴 MOTORES – DIREÇÃO (COMPARTILHADA POR PARES)
// ======================================================
// Motores 1 e 2 → IN00
// Motores 3 e 4 → IN01

constexpr uint8_t IN00A = 6;
constexpr uint8_t IN00B = 7;

constexpr uint8_t IN01A = 8;
constexpr uint8_t IN01B = 9;


// ======================================================
// 🔴 MOTORES – PWM INDIVIDUAL
// ======================================================

constexpr uint8_t PWM_M1 = 2;
constexpr uint8_t PWM_M2 = 3;
constexpr uint8_t PWM_M3 = 4;
constexpr uint8_t PWM_M4 = 5;


// ======================================================
// 🟢 ENCODERS (1 CANAL POR MOTOR)
// ======================================================

constexpr uint8_t ENC1 = 20;
constexpr uint8_t ENC2 = 21;


// ======================================================
// 🟡 SERVO
// ======================================================

constexpr uint8_t SERVO_PIN = 10;


// ======================================================
// 🟣 FITA LED (WS2812)
// ======================================================

constexpr uint8_t LED_STRIP = 11;


// ======================================================
// ⚫ BOTÕES E SWITCHES
// ======================================================

constexpr uint8_t BUTTON_PIN = 23;   // suporta interrupção
constexpr uint8_t SW_RIGHT   = 12;
constexpr uint8_t SW_LEFT    = 13;   // LED interno onboard


// ======================================================
// 🟤 SENSORES ANALÓGICOS (ADC)
// ======================================================

constexpr uint16_t C9_PIN    = A0;   // pin 14
constexpr uint16_t LDR_BLUE  = A1;   // pin 15
constexpr uint16_t LDR_RED   = A2;   // pin 16
constexpr uint16_t LDR_GREEN = A3;   // pin 17


// ======================================================
// ⚙️ CONFIGURAÇÕES GERAIS DO SISTEMA
// ======================================================

// PWM
constexpr uint16_t PWM_FREQUENCY = 20000;   // 20 kHz (ideal para motor)
constexpr uint8_t  PWM_RESOLUTION = 12;      // 0–255

// ADC
constexpr uint8_t  ADC_RESOLUTION = 12;     // 0–4095
constexpr uint16_t ADC_MAX_VALUE  = (1 << ADC_RESOLUTION) - 1;

// Servo
constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;
constexpr uint8_t SERVO_CENTER_DEG = 90;
constexpr uint8_t SERVO_LEFT_DROP_DEG = 35;
constexpr uint8_t SERVO_RIGHT_DROP_DEG = 145;
constexpr uint16_t SERVO_MOVE_DELAY_MS = 250;
constexpr uint16_t SERVO_BETWEEN_KITS_MS = 220;

// Encoder
constexpr uint16_t ENCODER_PPR = 600;  // ajustar conforme modelo

// Maze / tile (navegacao)
constexpr uint16_t TILE_SIZE_MM = 300;               // tamanho nominal do ladrilho
constexpr uint16_t TILE_CENTER_TO_CENTER_MM = 280;   // medida real usada no robo
constexpr int32_t ENCODER_TICKS_PER_TILE = 860;      // calibrar em pista real
constexpr uint16_t TOF_WALL_CLEAR_MM = 120;          // limiar para considerar caminho livre

// Vítimas / resgate
constexpr uint8_t VICTIM_CONFIDENCE_MIN = 65;
constexpr uint32_t VICTIM_ACTION_COOLDOWN_MS = 2500;
constexpr uint16_t VICTIM_WALL_CONFIRM_MM = 220;  // parede lateral maxima para validar vitima
constexpr uint16_t PAUSE_LED_INTERVAL_MS = 220;

// Switch avoidance (obstaculos)
constexpr int16_t SWITCH_BACK_PWM = 220;
constexpr int16_t SWITCH_TURN_PWM = 240;
constexpr uint16_t SWITCH_BACK_MS = 120;
constexpr uint16_t SWITCH_TURN_MS = 180;


// ======================================================
// 🧠 FLAGS DO SISTEMA
// ======================================================

constexpr bool USE_BUTTON_INTERRUPT = true;
constexpr bool DEBUG_SERIAL         = true;



// ======================================================
// 🧠 ENDEREÇOS I2C
// ======================================================
constexpr uint8_t TCA_A = 0x70;
constexpr uint8_t TCA_B = 0x71;
constexpr uint8_t OPENMVL = 0x10;
constexpr uint8_t OPENMVR = 0x11;

