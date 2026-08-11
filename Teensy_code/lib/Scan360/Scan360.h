#pragma once
#include "VL53Mux12_FRAN.h"

/*
  Agrupa os 11 ToF por lado e resume cada lado num unico valor.

  Convencao dos nomes (ver kSensorNames em VL53Mux12_FRAN.cpp):
  primeira letra = lado, segunda = posicao naquele lado.
    FL/FC/FR -> frente (esquerda, centro, direita)
    LF/LC/LB -> lado esquerdo (frente, centro, tras)
    RF/RC/RB -> lado direito (frente, centro, tras)
    BL/BR    -> traseira (esquerda, direita) - so 2 sensores

  TOLERANCIA A FALHA: um sensor com timeout grava mm=0 e ok=false
  (VL53Mux12_FRAN::readSensorByIndex). Como zero seria sempre o menor valor,
  um unico sensor ruim derrubaria o grupo inteiro e criaria uma parede
  fantasma. Por isso as leituras invalidas sao IGNORADAS e o minimo sai
  apenas dos sensores que responderam.

  RETORNO 0 = "sem leitura confiavel neste lado" (todos os sensores do grupo
  invalidos), nao "parede encostada". Quem consome deve testar > 0 antes de
  usar o valor como distancia. Como frontFree/leftFree/etc comparam com
  TOF_WALL_CLEAR_MM, o zero naturalmente vira "bloqueado" - conservador de
  proposito: sem informacao, nao avanca.
*/
struct Scan360 {
  ScanToF12 s;

  uint16_t minFront() const {
    return min3(ToFId::FL, ToFId::FC, ToFId::FR);
  }

  uint16_t minLeft() const {
    return min3(ToFId::LF, ToFId::LC, ToFId::LB);
  }

  uint16_t minRight() const {
    return min3(ToFId::RF, ToFId::RC, ToFId::RB);
  }

  uint16_t minBack() const {
    return min2(ToFId::BL, ToFId::BR);
  }

  // Quantos sensores de cada lado responderam nesta amostra.
  // Util para diagnostico: 0 significa que o lado esta cego.
  uint8_t validFront() const { return count3(ToFId::FL, ToFId::FC, ToFId::FR); }
  uint8_t validLeft()  const { return count3(ToFId::LF, ToFId::LC, ToFId::LB); }
  uint8_t validRight() const { return count3(ToFId::RF, ToFId::RC, ToFId::RB); }
  uint8_t validBack()  const { return (uint8_t)(isValid(ToFId::BL) + isValid(ToFId::BR)); }

  // Skews: diferenca entre dois sensores do mesmo lado, para estimar
  // desalinhamento. Retornam 0 se qualquer um dos dois estiver invalido -
  // sem os dois valores nao ha diferenca que faca sentido.
  int16_t frontSkew() const    { return diff(ToFId::FL, ToFId::FR); }
  int16_t leftSkew() const     { return diff(ToFId::LF, ToFId::LB); }
  int16_t backSkew() const     { return diff(ToFId::BL, ToFId::BR); }
  int16_t corridorError() const { return diff(ToFId::LC, ToFId::RC); }

private:
  static constexpr uint16_t NO_READING = 0xFFFF;

  // Um sensor so conta se respondeu (ok) e devolveu valor nao-nulo.
  bool isValid(ToFId id) const {
    const uint8_t i = (uint8_t)id;
    return s.ok[i] && s.mm[i] > 0;
  }

  void accumulate(ToFId id, uint16_t& best) const {
    if (!isValid(id)) return;
    const uint16_t v = s.mm[(uint8_t)id];
    if (v < best) best = v;
  }

  uint16_t min3(ToFId a, ToFId b, ToFId c) const {
    uint16_t best = NO_READING;
    accumulate(a, best);
    accumulate(b, best);
    accumulate(c, best);
    return (best == NO_READING) ? 0 : best;
  }

  uint16_t min2(ToFId a, ToFId b) const {
    uint16_t best = NO_READING;
    accumulate(a, best);
    accumulate(b, best);
    return (best == NO_READING) ? 0 : best;
  }

  uint8_t count3(ToFId a, ToFId b, ToFId c) const {
    return (uint8_t)(isValid(a) + isValid(b) + isValid(c));
  }

  int16_t diff(ToFId a, ToFId b) const {
    if (!isValid(a) || !isValid(b)) return 0;
    return (int16_t)s.mm[(uint8_t)a] - (int16_t)s.mm[(uint8_t)b];
  }
};
