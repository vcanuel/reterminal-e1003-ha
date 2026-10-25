// Rooms (climate.*): one row per room with temperature, target, humidity and heating indicator.
#pragma once
#include "ui.h"

namespace ui {

struct Room {
  const char *name;
  sensor::Sensor *cur, *target, *hum;
  text_sensor::TextSensor *action;
};

// One row, 86 px high.
inline void room_row(Display &it, Rect r, const Room &room) {
  txt(it, r.x, r.y + 14, f_txt, BLACK, TextAlign::TOP_LEFT, room.name);
  if (room.action->state == "heating")
    txt(it, r.x + 360, r.y + 8, i_s, BLACK, TextAlign::TOP_LEFT, "\U000F0238");
  txt(it, r.x2() - 134, r.y, f_h1, BLACK, TextAlign::TOP_RIGHT, fmt(room.cur->state) + "°");
  txt(it, r.x2(), r.y + 2, f_small, GRAY, TextAlign::TOP_RIGHT, fmt(room.target->state) + "°");
  txt(it, r.x2(), r.y + 38, f_small, GRAY, TextAlign::TOP_RIGHT, fmt(room.hum->state, 0) + " %");
}

// Title + list of rooms (each one is the global exposed by widgets/room.yaml).
// Suggested area: ~760 wide, 80 + 86 x number of rooms high.
inline void rooms(Display &it, Rect r, const char *label, std::initializer_list<Draw> list) {
  title(it, r.x, r.y, "\U000F0F54", label);
  int i = 0;
  for (auto &room : list) {
    const int y = r.y + 80 + i * 86;
    if (i > 0) it.filled_rectangle(r.x, y - 8, r.w, 1, LIGHT);
    room(it, {r.x, (int16_t) y, r.w, 86});
    i++;
  }
}

}  // namespace ui
