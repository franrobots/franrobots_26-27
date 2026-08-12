#include "Encoder.h"

Encoder* Encoder::_instance0 = nullptr;
Encoder* Encoder::_instance1 = nullptr;

Encoder::Encoder(uint8_t pinA, uint8_t pinB) : _pinA(pinA), _pinB(pinB), _ticks(0) {}

void Encoder::begin(bool pullup) {
  pinMode(_pinA, pullup ? INPUT_PULLUP : INPUT);
  pinMode(_pinB, pullup ? INPUT_PULLUP : INPUT);

  // RISING, nao CHANGE: a mesma borda amostra B e conta o tick.
  //
  // Antes a interrupcao era CHANGE e o codigo contava nas DUAS bordas de A,
  // mas so reavaliava o sentido na subida. Metade dos ticks saia com uma
  // direcao amostrada meio periodo antes, e o primeiro tick depois de uma
  // inversao de sentido saia com o sinal trocado.
  //
  // Efeito colateral: a resolucao caiu pela metade. ENCODER_TICKS_PER_TILE
  // precisa ser medido de novo (ver ENCODER_TEST_MODE em config.h).
  //
  // A ordem de begin() importa: o primeiro pega a ISR 0, o segundo a ISR 1.
  if (_instance0 == nullptr) {
    _instance0 = this;
    attachInterrupt(digitalPinToInterrupt(_pinA), Encoder::isr0, RISING);
  }
  else if (_instance1 == nullptr) {
    _instance1 = this;
    attachInterrupt(digitalPinToInterrupt(_pinA), Encoder::isr1, RISING);
  }
}

// Nenhuma destas precisa desabilitar interrupcao.
//
// _ticks e escrito SO pela ISR; _base e escrito SO pelo contexto principal.
// Nao ha leitura-modificacao-escrita compartilhada, e acesso de 32 bits
// alinhado e atomico no Cortex-M7 - entao as duas partes nunca disputam a
// mesma variavel. Zerar e feito movendo _base, nao _ticks.
//
// A versao anterior chamava noInterrupts()/interrupts(), que reabilita a
// interrupcao incondicionalmente: quebraria qualquer secao critica externa
// que envolvesse a chamada. E o reset() perdia os ticks que chegassem
// durante a propria janela critica.
void Encoder::reset() {
  _base = _ticks;
}

int32_t Encoder::read() const {
  return ((int32_t)_ticks - _base) * _sign;
}

int32_t Encoder::readAndReset() {
  const int32_t t = _ticks;      // uma unica amostra, usada nas duas contas
  const int32_t v = t - _base;
  _base = t;
  return v * _sign;
}

int32_t Encoder::readRaw() const {
  return (int32_t)_ticks - _base;
}

void Encoder::handleISR() {
  // Na subida de A, o nivel de B define o sentido (quadratura).
  // Convencao mantida da versao anterior: B em nivel baixo incrementa.
  if (digitalRead(_pinB)) {
    _ticks--;
  } else {
    _ticks++;
  }
}

void Encoder::isr0() {
  if (_instance0) _instance0->handleISR();
}

void Encoder::isr1() {
  if (_instance1) _instance1->handleISR();
}
