// Electric car tile: battery, range, plugged in / charging.
#pragma once
#include "ui.h"

namespace ui {

struct Car {
  const char *name;
  sensor::Sensor *battery, *range, *charger_power;
  binary_sensor::BinarySensor *plugged;
};

// Suggested area: 370 x 220.
inline void car(Display &it, Rect r, const Car &c) {
  const bool charging = c.charger_power->state > 0;
  tile(it, r, "\U000F0B6C", c.name, fmt(c.battery->state, 0) + " %",
       fmt(c.range->state, 0) + " km" + (charging ? tr().charging : c.plugged->state ? tr().plugged : ""));
}

}  // namespace ui
