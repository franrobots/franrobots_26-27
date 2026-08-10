#pragma once
#include <Arduino.h>

/*
=========================================================
                TEENSY 4.0 – ROBO MAZE
=========================================================
Autor: Calebe Almeida
Plataforma: Teensy 4.0 (i.MX RT1062)
Ambiente: PlatformIO
=========================================================
*/

// ======================================================
// 🔵 I2C BUS
// ======================================================

constexpr uint8_t I2C_SDA = 18;
constexpr uint8_t I2C_SCL = 19;
constexpr uint32_t I2C_FREQUENCY = 400000;   // 400 kHz 
// ======================================================
// 🔴 MOTORES – DIREÇÃO (COMPARTILHADA POR PARES)
// ======================================================

// Direito Frente (M1)
constexpr uint8_t Ain1_M1 = 6;
constexpr uint8_t Ain2_M1 = 7;
// Direito tras (M2)
constexpr uint8_t Ain1_M2 = 8;
constexpr uint8_t Ain2_M2 = 9;
// Esquerdo Frente (M1)
constexpr uint8_t Ain1_M3 = 2; // 2 era 24
constexpr uint8_t Ain2_M3 = 3; // 3 era 28
// Esquerdo tras (M2)
constexpr uint8_t Ain1_M4 = 5; // 5  era 25
constexpr uint8_t Ain2_M4 = 4; // 4 era 29


// ======================================================
// 🔴 MOTORES – PWM INDIVIDUAL
// ======================================================

//constexpr uint8_t PWM_M1 = 2;
//constexpr uint8_t PWM_M2 = 5;
//constexpr uint8_t PWM_M3 = 4;
//constexpr uint8_t PWM_M4 = 5;

// NÃO USADO MAIS, AGORA PELO DRIVER MUDAR,O PWM É COMPARTILHADO PELO PAR DE MOTORES (M1 e M1, M2 e M2)

// ======================================================
// 🟢 ENCODERS (1 CANAL POR MOTOR)
// ======================================================

constexpr uint8_t ENC1_A = 21;
constexpr uint8_t ENC1_B = 22;

constexpr uint8_t ENC2_A = 26;
constexpr uint8_t ENC2_B = 20;


// ======================================================
// 🟡 SERVO
// ======================================================

constexpr uint8_t SERVO_PIN = 10;


// ======================================================
// 🟣 FITA LED (WS2812)
// ======================================================

constexpr uint8_t LED_LEFT = 32;
constexpr uint8_t LED_MIDDLE = 33;
constexpr uint8_t LED_RIGHT = 11;


// ======================================================
// ⚫ BOTÕES E SWITCHES
// ======================================================

constexpr uint8_t BUTTON_PIN = 23;   // suporta interrupção
constexpr uint8_t SW_RIGHT   = 12;   // switch button right
constexpr uint8_t SW_LEFT    = 13;   // switch button Left


// ======================================================
// 🟤 SENSORES ANALÓGICOS (ADC)
// ======================================================

constexpr uint16_t C9_PIN    = A2;   // pin 16
constexpr uint16_t LDR_RED   = A1;   // pin 15
constexpr uint16_t LDR_GREEN = A3;   // pin 17
constexpr uint16_t LDR_BLUE  = A0;   // pin 14


// ======================================================
// ⚙️ CONFIGURAÇÕES GERAIS DO SISTEMA
// ======================================================

// PWM
constexpr uint16_t PWM_FREQUENCY = 30000;   // 30 kHz (ideal para motor)
constexpr uint8_t  PWM_RESOLUTION = 12;      // 12 bita (ideal para motor)

// ADC
constexpr uint8_t  ADC_RESOLUTION = 12;     // 0–4095
constexpr uint16_t ADC_MAX_VALUE  = (1 << ADC_RESOLUTION) - 1;

// Servo

// SERVO SEM COISO, ENTÃO NADA DISSO MUDA NADA, NÃO É USADO, MAS DEIXEI AQUI PRA NÃO ESQUECER, SE PRECISAR DEPOIS É SÓ AJUSTAR

constexpr uint16_t SERVO_MIN_US = 1000;
constexpr uint16_t SERVO_MAX_US = 2000;
//constexpr uint8_t SERVO_CENTER_DEG = 90;
//constexpr uint8_t SERVO_LEFT_DROP_DEG = 35;
//constexpr uint8_t SERVO_RIGHT_DROP_DEG = 145;
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
constexpr uint8_t OPENMVL = 0x12;
constexpr uint8_t OPENMVR = 0x13;
constexpr uint8_t BNO055_ADDRESS = 0x28;