// Tuile voiture électrique : batterie, autonomie, branchée / en charge.
#pragma once
#include "ui.h"

namespace ui {

struct Car {
  const char *name;
  sensor::Sensor *battery, *range, *charger_power;
  binary_sensor::BinarySensor *plugged;
};

// Zone conseillée : 370 x 220.
inline void car(Display &it, Rect r, const Car &c) {
  const bool charging = c.charger_power->state > 0;
  tile(it, r, "\U000F0B6C", c.name, fmt(c.battery->state, 0) + " %",
       fmt(c.range->state, 0) + " km" + (charging ? " · en charge" : c.plugged->state ? " · branchée" : ""));
}

}  // namespace ui
