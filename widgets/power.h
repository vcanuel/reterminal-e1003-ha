// Tuile électricité : puissance instantanée (W, ou kW au-delà de 1000 W).
#pragma once
#include "ui.h"

namespace ui {

// Zone conseillée : 370 x 220.
inline void power(Display &it, Rect r, const char *label, const char *sub, sensor::Sensor *watts) {
  const float p = watts->state;
  tile(it, r, "\U000F0241", label, std::isnan(p) ? "--" : (p >= 1000 ? fmt(p / 1000, 2) + " kW" : fmt(p, 0) + " W"),
       sub);
}

}  // namespace ui
