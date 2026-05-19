#pragma once
#include "VL53Mux12_FRAN.h"

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

  int16_t frontSkew() const {
    return (int16_t)s.mm[(uint8_t)ToFId::FL] -
           (int16_t)s.mm[(uint8_t)ToFId::FR];
  }

  int16_t leftSkew() const {
    return (int16_t)s.mm[(uint8_t)ToFId::LF] -
           (int16_t)s.mm[(uint8_t)ToFId::LB];
  }

  int16_t corridorError() const {
    return (int16_t)s.mm[(uint8_t)ToFId::LC] -
           (int16_t)s.mm[(uint8_t)ToFId::RC];
  }

private:
  uint16_t min3(ToFId a, ToFId b, ToFId c) const {
    uint16_t va = s.mm[(uint8_t)a];
    uint16_t vb = s.mm[(uint8_t)b];
    uint16_t vc = s.mm[(uint8_t)c];
    return min(va, min(vb, vc));
  }
};