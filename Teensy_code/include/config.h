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
// 🎛️ CONTROLE EM MALHA FECHADA (PID)
// ======================================================

// --- Convencoes de sinal (AJUSTAR NO PRIMEIRO TESTE) ---
// TURN_SIGN: +1 se move_tank(0, +turn) gira para a DIREITA, ou seja,
// no sentido em que o yaw do BNO055 AUMENTA. Se o robo girar para o lado
// errado / entrar em fuga, troque para -1. Este e o unico ponto de ajuste.
constexpr int8_t TURN_SIGN = +1;

// ENC_BALANCE_SIGN: +1 se, quando a roda esquerda anda MAIS que a direita,
// a correcao precisa ser para a esquerda. Se o robo "abrir" em vez de
// corrigir durante a reta, troque para -1.
constexpr int8_t ENC_BALANCE_SIGN = -1;

// --- PID de guinada (yaw) - realimentado pelo BNO055 ---
// Entrada: erro em GRAUS. Saida: contagem de PWM de giro.
constexpr float YAW_KP     = 26.0f;
constexpr float YAW_KI     = 0.8f;
constexpr float YAW_KD     = 3.0f;
constexpr float YAW_I_MAX  = 250.0f;   // anti-windup do termo integral
constexpr float YAW_OUT_MAX = 1800.0f; // saturacao do comando de giro
constexpr float YAW_TOL_DEG = 2.5f;    // erro aceito para considerar alinhado

// --- PID de equilibrio das rodas - realimentado pelos encoders ---
// Entrada: diferenca de ticks entre roda esquerda e direita no ladrilho.
// Termo de TRIM: corrige derivas que o yaw ainda nao acusou. Manter ganho
// baixo, o BNO e a referencia principal de rumo. Zerar ENC_KP desliga.
constexpr float ENC_KP     = 1.4f;
constexpr float ENC_KI     = 0.04f;
constexpr float ENC_KD     = 0.25f;
constexpr float ENC_I_MAX  = 150.0f;
constexpr float ENC_OUT_MAX = 500.0f;

// --- Centragem lateral entre paredes - realimentada pelos ToF ---
// Entrada: (distancia esquerda - distancia direita) em mm.
constexpr float CENTER_KP     = 2.2f;
constexpr float CENTER_OUT_MAX = 450.0f;
// So centraliza com parede dos DOIS lados dentro deste alcance.
constexpr uint16_t CENTER_VALID_MM = 300;

// --- Velocidades base ---
constexpr int16_t DRIVE_PWM = 2200;   // avanco durante DRIVE_TILE
constexpr int16_t ALIGN_PWM = 1200;   // avanco lento durante PRE_ALIGN

// --- Periodo do laco de controle ---
// dt fixo mantem o PID previsivel (derivativo/integral estaveis).
constexpr uint16_t CONTROL_PERIOD_MS = 20;   // 50 Hz
constexpr uint16_t IMU_PERIOD_MS     = 10;   // leitura do BNO055

// --- Watchdogs de fase (rede de seguranca) ---
// Com IMU e encoders reais a saida normal e por erro/ticks. Estes limites
// so evitam travamento se um sensor morrer no meio da fase.
constexpr uint16_t PHASE_PREALIGN_MAX_MS = 1500;
constexpr uint16_t PHASE_TURN_MAX_MS     = 2500;
constexpr uint32_t PHASE_DRIVE_MAX_MS    = 4000;
constexpr uint16_t FRONT_STOP_MM         = 90;  // parede frontal: aborta o ladrilho


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