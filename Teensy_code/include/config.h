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
// 🔴 MOTORES – SENTIDO DE GIRO
// ======================================================
//
// Robot::setLeftRight manda o MESMO valor para os dois motores de um lado.
// Para o lado andar junto, os dois tem que girar no mesmo sentido fisico -
// e para o robo andar para frente, o lado esquerdo e o direito tem que
// girar em sentidos fisicos OPOSTOS, porque estao montados espelhados.
// Estas quatro flags acertam isso.
//
// SEM ELAS o robo fica parado se debatendo: as rodas de um mesmo lado
// empurram para lados contrarios e se anulam. E o sintoma classico de nao
// haver inversao nenhuma configurada.
//
// Ja da para ver um desencontro so de olhar os pinos acima: M3 (esq/frente)
// e Ain1=2, Ain2=3, mas M4 (esq/tras) e Ain1=5, Ain2=4 - invertido. Com o
// mesmo PWM esses dois giram para lados opostos. Por isso RL comeca em true.
//
// Os valores abaixo sao HIPOTESE, deduzida dos pinos. Confirme com
// MOTOR_TEST_MODE: ele aciona um motor de cada vez, e cada um tem que
// girar no sentido de FRENTE. Os que girarem ao contrario, inverta aqui.
constexpr bool MOTOR_INV_FL = false;   // M3 - referencia
constexpr bool MOTOR_INV_RL = false;    // M4 - pinos trocados em relacao ao M3
constexpr bool MOTOR_INV_FR = false;    // M1 - lado espelhado
constexpr bool MOTOR_INV_RR = false;    // M2 - lado espelhado

// Teste de sentido dos motores. Com true a navegacao nao roda: o loop
// aciona FL, RL, FR e RR em sequencia, um de cada vez, anunciando cada um
// no serial. Rode com o robo APOIADO, rodas no ar. Voltar para false
// depois de acertar as quatro flags.
constexpr bool MOTOR_TEST_MODE = false;
constexpr int16_t  MOTOR_TEST_PWM    = 1800;
constexpr uint16_t MOTOR_TEST_STEP_MS = 1500;

// Modo de medicao dos ToF. Com true a navegacao nao roda e NENHUM motor e
// acionado: o loop so imprime as quatro distancias. E como se mede
// ALIGN_FRONT_TARGET_MM e ALIGN_BACK_TARGET_MM - ver o bloco de centragem
// longitudinal no fim deste arquivo.
constexpr bool TOF_PROBE_MODE = false;


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

// --- Sinal dos encoders ---
// FL e RR ficam em lados espelhados do chassi: dependendo da fiacao, andar
// para frente pode fazer um contar para cima e o outro para baixo. Marque
// como invertida a roda que conta NEGATIVO andando para FRENTE, para que os
// dois fiquem positivos = frente. Tanto traveledTicks() quanto o trim de
// equilibrio do main.cpp dependem disso.
constexpr bool ENC_FL_INVERTED = false;
constexpr bool ENC_RR_INVERTED = false;

// Maze / tile (navegacao)
constexpr uint16_t TILE_SIZE_MM = 300;               // tamanho nominal do ladrilho
constexpr uint16_t TILE_CENTER_TO_CENTER_MM = 280;   // medida real usada no robo
// ATENCAO: a ISR do Encoder passou a contar so na borda de subida de A (antes
// contava nas duas), entao a resolucao caiu pela metade. Este valor e o
// antigo 450 dividido por 2 e NAO foi medido.
constexpr int32_t ENCODER_TICKS_PER_TILE = 860;
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
constexpr int8_t TURN_SIGN = 1;

// ENC_BALANCE_SIGN: +1 se, quando a roda esquerda anda MAIS que a direita,
// a correcao precisa ser para a esquerda. Se o robo "abrir" em vez de
// corrigir durante a reta, troque para -1.
constexpr int8_t ENC_BALANCE_SIGN = -1;

// --- PID de guinada (yaw) - realimentado pelo BNO055 ---
// Entrada: erro em GRAUS. Saida: contagem de PWM de giro.
constexpr float YAW_KP     = 30.0f;
constexpr float YAW_KI     = 0.8f;
constexpr float YAW_KD     = 3.0f;
constexpr float YAW_I_MAX  = 250.0f;   // anti-windup do termo integral

// Erro aceito para considerar alinhado. Apertado de 2.5 para 1.5: o robo
// atravessa o ladrilho inteiro com o erro que sobrar daqui, e no fim ele
// vira desvio lateral. Se o ALIGN passar a estourar o watchdog sempre,
// afrouxe - a correcao pulsada tem um passo minimo, e exigir menos que ele
// e pedir o impossivel.
constexpr float YAW_TOL_DEG = 1.5f;

// Saturacao do comando de giro. Girando no lugar as QUATRO rodas arrastam
// de lado, o que pede mais torque que andar reto - entao este teto tem que
// ficar ACIMA de DRIVE_PWM, nao abaixo. Estava em 1800 (menos que os 2200
// do avanco) e o robo simplesmente nao girava: o log mostrava o comando
// saturado em 1800 por 2,5 s com o yaw parado.
constexpr float YAW_OUT_MAX = 4096.0f;

// Piso do comando de giro, aplicado SO com o robo parado (fases ALIGN e
// TURN) e SO enquanto o erro passa de YAW_TOL_DEG. Sem ele o comando cai
// junto com o erro - a 10 graus, YAW_KP*10 = 260 - fica abaixo do atrito
// estatico e o robo congela curto do alvo ate o watchdog da fase.
//
// COMO AJUSTAR: e o menor PWM que faz o robo girar mesmo. Suba ate o giro
// comecar sempre, sem hesitar.
//
// 2600 ainda ficava ABAIXO do arranque: o log mostrava o ALIGN comandando
// -2600 com o yaw parado em 6.1 graus tres ciclos seguidos, e o TURN
// arrastando de 11.4 para 12.9 antes de soltar e disparar 48 graus de uma
// vez. Esse stick-slip tambem explica o robo "girar pela metade": no PWM
// marginal o lado com menos atrito arranca primeiro e o robo pivota em
// torno do outro em vez de girar em torno do centro.
constexpr float TURN_MIN_PWM = 4000.0f;

static_assert(TURN_MIN_PWM < YAW_OUT_MAX,
              "o piso de giro tem que caber abaixo do teto");

// --- Correcao fina de rumo por pulsos ---
// Com atrito estatico alto o robo so sai do lugar acima de TURN_MIN_PWM, e
// nesse PWM, uma vez solto, ele gira dezenas de graus de uma vez. Ou seja:
// acionamento continuo NAO consegue corrigir 4 graus - ou nao move, ou
// passa longe do alvo.
//
// Abaixo de TURN_FINE_DEG a correcao passa a ser pulsada: o pulso da o
// empurrao que vence o atrito e acaba antes de o robo girar demais. Acima
// disso o acionamento e continuo, que e o que faz o giro de 90 graus.
//
// Para desligar os pulsos, ponha TURN_FINE_DEG = 0.
// O pulso encurtou junto com a subida do TURN_MIN_PWM para 4000: o passo
// de cada pulso e proporcional a PWM x tempo, e 60 ms a 4000 dao um salto
// grande demais para assentar dentro de YAW_TOL_DEG = 1.5 graus. Se o ALIGN
// ficar oscilando em torno do alvo, encurte mais (o minimo util e um ciclo
// de controle, 20 ms); se ele nao sair do lugar, alongue.
constexpr float    TURN_FINE_DEG     = 15.0f;
constexpr uint16_t TURN_PULSE_ON_MS  = 40;    // 2 ciclos de controle
constexpr uint16_t TURN_PULSE_OFF_MS = 200;   // 10 ciclos para assentar
// (o static_assert contra CONTROL_PERIOD_MS esta la embaixo, junto da
//  definicao dele)

// --- PID de equilibrio das rodas - realimentado pelos encoders ---
// Entrada: diferenca de ticks entre roda esquerda e direita no ladrilho.
// Termo de TRIM: corrige derivas que o yaw ainda nao acusou. Manter ganho
// baixo, o BNO e a referencia principal de rumo. Zerar ENC_KP desliga.
constexpr float ENC_KP     = 2.6f;
constexpr float ENC_KI     = 0.04f;
constexpr float ENC_KD     = 0.25f;
constexpr float ENC_I_MAX  = 150.0f;
constexpr float ENC_OUT_MAX = 500.0f;

// --- Centragem lateral entre paredes - realimentada pelos ToF ---
// Entrada: (distancia esquerda - distancia direita) em mm.
constexpr float CENTER_KP     = 2.7f;
constexpr float CENTER_OUT_MAX = 450.0f;
// So centraliza com parede dos DOIS lados dentro deste alcance.
constexpr uint16_t CENTER_VALID_MM = 300;

// --- Velocidades base ---
// Mais devagar que os 2200 anteriores: da tempo para as malhas de rumo e
// centragem agirem dentro do ladrilho, em vez de so constatarem o desvio
// no fim. Se o robo nao arrancar mais do repouso, subir de volta ate voltar
// a sair - o atrito estatico linear e o piso real deste numero.
constexpr int16_t DRIVE_PWM = 1900;

// --- Autoridade de curva durante o avanco ---
// No DRIVE_TILE o comando vai como move_tank(DRIVE_PWM, turn), ou seja
// left = DRIVE_PWM + turn e right = DRIVE_PWM - turn. Se turn puder chegar
// nos 4096 do YAW_OUT_MAX, um lado satura e o outro INVERTE: o robo para de
// andar e passa a girar no meio do ladrilho.
//
// Este teto separado mantem a correcao como direcao, nao como giro. Regra
// pratica: bem abaixo de DRIVE_PWM.
constexpr float DRIVE_TURN_MAX = 700.0f;

static_assert(DRIVE_TURN_MAX < (float)DRIVE_PWM,
              "com turn >= DRIVE_PWM uma das rodas inverte e o avanco vira giro");

// --- Periodo do laco de controle ---
// dt fixo mantem o PID previsivel (derivativo/integral estaveis).
constexpr uint16_t CONTROL_PERIOD_MS = 20;   // 50 Hz

static_assert(TURN_PULSE_ON_MS >= CONTROL_PERIOD_MS,
              "o pulso de giro tem que durar ao menos um ciclo de controle");
constexpr uint16_t IMU_PERIOD_MS     = 10;   // leitura do BNO055

// --- Watchdogs de fase (rede de seguranca) ---
// Com IMU e encoders reais a saida normal e por erro/ticks. Estes limites
// so evitam travamento se um sensor morrer no meio da fase.
// O ALIGN precisa de mais folga desde que a correcao fina virou pulsada:
// cada pulso gasta TURN_PULSE_ON_MS + TURN_PULSE_OFF_MS e corrige pouco.
constexpr uint16_t PHASE_ALIGN_MAX_MS    = 3000;
constexpr uint16_t PHASE_TURN_MAX_MS     = 2500;
// Subiu junto com a queda do DRIVE_PWM: mais devagar, mesmo ladrilho, mais
// tempo. Se este watchdog disparar antes dos ticks, o ladrilho vira aborto.
constexpr uint32_t PHASE_DRIVE_MAX_MS    = 6000;
constexpr uint16_t PHASE_CENTER_MAX_MS   = 1800;
// Parede frontal. No inicio do ladrilho ABORTA (e marca a celula da frente
// como bloqueada); passados TILE_COMPLETE_PCT do percurso conta como fim
// normal de beco. Manter abaixo de TOF_WALL_CLEAR_MM.
constexpr uint16_t FRONT_STOP_MM = 90;

// ======================================================
// 📍 CENTRAGEM LONGITUDINAL NO LADRILHO (fase CENTER)
// ======================================================
//
// A fase roda DUAS vezes por ladrilho: depois de andar e depois de girar.
// Ela posiciona o robo no eixo em que ele esta olhando, avancando OU
// recuando ate a parede da frente ficar em ALIGN_FRONT_TARGET_MM.
//
// Por que as duas vezes cobrem os dois eixos: girar 90 graus troca o eixo
// longitudinal pelo lateral. Centrar no eixo A, girar, e centrar no eixo B
// deixa o robo no centro do ladrilho nos dois - o que a traccao diferencial
// nao consegue fazer de uma vez so, ja que o robo nao anda de lado.
//
// ####################################################################
// ##  ESTES DOIS DECIDEM ONDE O ROBO PARA. Nao e ENCODER_TICKS_PER_  ##
// ##  TILE, nem TILE_SIZE_MM: com parede a frente (ou atras) dentro  ##
// ##  do alcance, a fase CENTER DESCARTA os encoders e posiciona o   ##
// ##  robo so por estes alvos.                                       ##
// ####################################################################
//
// DEFINICAO: quanto o ToF le com o robo no centro EXATO de um ladrilho que
// tem parede naquela borda. NAO e folga de giro - essa era a definicao de
// quando esta fase era so um recuo, e e o erro que deixa o robo parado em
// cima da divisao entre dois ladrilhos.
//
// COMO MEDIR: ligue TOF_PROBE_MODE (abaixo). Marque com fita o centro de um
// ladrilho, ponha o robo ali a mao com parede a frente e leia F: no serial.
// Repita com parede atras para o B:.
//
// Os dois valores so sao iguais se o chassi for simetrico - e a traseira
// tem 2 sensores contra 3 da frente. Meça os dois.
//
// ORDEM DE GRANDEZA: e o meio-ladrilho MENOS o quanto o sensor avanca em
// relacao ao centro do robo. Com ladrilho de 280 mm o meio e 140 mm; se o
// sensor fica 80 mm a frente do centro, a leitura no centro e ~60 mm.
constexpr uint16_t ALIGN_FRONT_TARGET_MM = 109;  // MEDIR
constexpr uint16_t ALIGN_BACK_TARGET_MM  = 130;  // MEDIR
constexpr uint16_t ALIGN_BACK_MIN_MM     = 70;   // nunca recuar alem disso

// Alcance em que uma parede ainda conta como borda DESTE ladrilho.
//
// Derivado, nao chutado: uma parede um ladrilho a frente le
// ALIGN_FRONT_TARGET_MM + TILE_CENTER_TO_CENTER_MM. Estava fixo em 400
// contra 109 + 280 = 389 - onze milimetros de margem. Bastava ruido, ou o
// robo estar um pouco atras do centro, para ele aceitar a parede ERRADA
// como referencia e andar um ladrilho inteiro "centralizando" nela,
// terminando em cima da divisao seguinte.
//
// Meio ladrilho de margem separa os dois casos com folga: aceita a borda
// deste ladrilho mesmo com o robo bem atrasado, e rejeita a do proximo.
constexpr uint16_t CENTER_REF_VALID_MM =
    ALIGN_FRONT_TARGET_MM + TILE_CENTER_TO_CENTER_MM / 2;

static_assert(CENTER_REF_VALID_MM < ALIGN_FRONT_TARGET_MM + TILE_CENTER_TO_CENTER_MM,
              "o alcance valido chega na parede do ladrilho SEGUINTE: o robo "
              "vai andar um ladrilho inteiro tentando se centrar nela");

// Banda morta. Apertar demais faz o robo ficar caçando o alvo, porque o
// arranque linear tambem tem atrito estatico.
constexpr uint16_t CENTER_TOL_MM = 10;

// Ganho e limites do avanco/recuo de centragem. Entrada em mm, saida em PWM.
constexpr float   CENTER_POS_KP      = 14.0f;
constexpr int16_t CENTER_POS_MIN_PWM = 1500;   // piso de arranque linear
constexpr int16_t CENTER_POS_MAX_PWM = 1900;   // teto: e ajuste fino, nao viagem

// Recuar so faz sentido se o alvo for maior que a distancia em que o robo
// para: iguais, a fase CENTER nunca chega a acionar motor.
static_assert(ALIGN_FRONT_TARGET_MM > FRONT_STOP_MM,
              "ALIGN_FRONT_TARGET_MM tem que ser maior que FRONT_STOP_MM");
static_assert(ALIGN_BACK_MIN_MM > 0,
              "ALIGN_BACK_MIN_MM zero desliga a protecao de re");
static_assert(CENTER_POS_MIN_PWM <= CENTER_POS_MAX_PWM,
              "o piso de centragem tem que caber abaixo do teto");
static_assert(ALIGN_FRONT_TARGET_MM < CENTER_REF_VALID_MM,
              "o alvo tem que estar dentro do alcance considerado valido");


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