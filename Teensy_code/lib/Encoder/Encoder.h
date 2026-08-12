#pragma once
#include <Arduino.h>

/*
  Encoder incremental em quadratura, decodificado por interrupcao na borda
  de SUBIDA do canal A (o nivel de B nessa borda da o sentido).

  SINAL: FL e RR ficam em lados espelhados do chassi. Com a mesma fiacao,
  andar para frente faz um contar para cima e o outro para baixo. Use
  setInverted() para normalizar: depois disso os DOIS contam positivo
  andando para frente, que e o que RobotControl::traveledTicks() e o trim
  de equilibrio do main.cpp assumem.
*/
class Encoder {
public:
  explicit Encoder(uint8_t pinA, uint8_t pinB);

  void begin(bool pullup = true);

  // Marque como invertida a roda que conta NEGATIVO andando para frente.
  // Afeta apenas read()/readAndReset(); readRaw() continua sem o sinal.
  void setInverted(bool inverted) { _sign = inverted ? -1 : 1; }
  bool inverted() const { return _sign < 0; }

  void reset();
  int32_t read() const;
  int32_t readAndReset();

  // Mesma contagem de read(), mas SEM o sinal de setInverted(): mostra o
  // que a fiacao realmente entrega. Para diagnosticar ENC_*_INVERTED.
  int32_t readRaw() const;

private:
  uint8_t _pinA;
  uint8_t _pinB;

  // _ticks: escrito exclusivamente pela ISR.
  // _base:  escrito exclusivamente pelo contexto principal (reset()).
  // Sem variavel compartilhada em leitura-modificacao-escrita, nao ha
  // necessidade de desabilitar interrupcao em lugar nenhum.
  volatile int32_t _ticks;
  int32_t _base = 0;
  int8_t _sign = 1;

  void handleISR();

  static void isr0();
  static void isr1();

  static Encoder* _instance0;
  static Encoder* _instance1;
};
