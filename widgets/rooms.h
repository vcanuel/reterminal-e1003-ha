// Rooms (climate.*): one row per room with temperature, target, humidity and heating indicator.
// Touching a row opens the room sub-page: target -/+ and hvac mode.
#pragma once
#include "ui.h"

namespace ui {

struct Room {
  const char *key;  // widget id (sub-page key)
  const char *name;
  sensor::Sensor *cur, *target, *hum, *min, *max;
  text_sensor::TextSensor *action, *mode;  // hvac_action, hvac mode (entity state)
  const char *modes;                       // modes offered on the sub-page, e.g. "auto,heat,off"
  float step;
  float &set;        // target being changed on the sub-page, sent after a short delay (NAN = none)
  std::string &msg;  // command feedback
  void (*send_target)();
  void (*send_mode)(const std::string &mode);
};

inline const char *hvac_label(const std::string &m) {
  const Strings &t = tr();
  return m == "off" ? t.mode_off : m == "heat" ? t.mode_heat : m == "cool" ? t.mode_cool
       : m == "heat_cool" ? t.mode_heat_cool : m == "auto" ? t.mode_auto : m.c_str();
}

// Target shown on the sub-page: the one being changed, otherwise HA's.
inline float room_target(const Room &room) { return std::isnan(room.set) ? room.target->state : room.set; }

inline void room_page(Display &it, const Room &room) {
  const Strings &t = tr();
  page_header(it, room.name);

  // Measured temperature, humidity, heating.
  txt(it, 48, 200, f_txt, GRAY, TextAlign::TOP_LEFT, t.temperature);
  txt(it, 48, 260, f_xl, BLACK, TextAlign::TOP_LEFT, fmt(room.cur->state) + "°");
  if (!std::isnan(room.hum->state))
    txt(it, 48, 500, f_txt, GRAY, TextAlign::TOP_LEFT, t.humidity + fmt(room.hum->state, 0) + " %");
  const std::string a = room.action->state;
  if (a == "heating") {
    txt(it, 48, 570, i_s, BLACK, TextAlign::TOP_LEFT, "\U000F0238");
    txt(it, 118, 578, f_txt_b, BLACK, TextAlign::TOP_LEFT, t.heating);
  } else if (a == "idle" || a == "off") {
    txt(it, 48, 578, f_txt, GRAY, TextAlign::TOP_LEFT, a == "idle" ? t.idle : t.mode_off);
  }

  // Target with -/+.
  const float lo = std::isnan(room.min->state) ? 5 : room.min->state;
  const float hi = std::isnan(room.max->state) ? 30 : room.max->state;
  txt(it, 1392, 200, f_txt, GRAY, TextAlign::TOP_CENTER, t.target);
  txt(it, 1392, 260, f_xl, std::isnan(room.set) ? BLACK : GRAY, TextAlign::TOP_CENTER, fmt(room_target(room)) + "°");
  auto change = [room, lo, hi](float delta) {
    return [room, lo, hi, delta] {
      float v = room_target(room);
      if (std::isnan(v)) v = std::isnan(room.cur->state) ? 19 : std::round(room.cur->state);
      room.set = std::min(hi, std::max(lo, std::round((v + delta) / room.step) * room.step));
      room.msg = "";
      room.send_target();
    };
  };
  button(it, {980, 480, 360, 200}, "", false, change(-room.step));
  txt(it, 1160, 580, i_s, BLACK, TextAlign::CENTER, "\U000F1639");  // minus-thick
  button(it, {1444, 480, 360, 200}, "", false, change(room.step));
  txt(it, 1624, 580, i_s, BLACK, TextAlign::CENTER, "\U000F11EC");  // plus-thick
  if (!room.msg.empty()) txt(it, 1392, 710, f_txt, GRAY, TextAlign::TOP_CENTER, room.msg);

  // Hvac modes: the current one is filled.
  it.filled_rectangle(48, 800, 1776, 3, LIGHT);
  txt(it, 48, 830, f_h2, BLACK, TextAlign::TOP_LEFT, t.mode);
  auto modes = split(room.modes, ',');
  const int n = modes.size(), gap = 40, w = n > 0 ? (1776 - (n - 1) * gap) / n : 0;
  for (int i = 0; i < n; i++) {
    std::string m = modes[i];
    m.erase(0, m.find_first_not_of(' '));
    button(it, {(int16_t) (48 + i * (w + gap)), 950, (int16_t) w, 220}, hvac_label(m), m == room.mode->state,
           [room, m] {
             room.msg = tr().sent;
             room.send_mode(m);
           });
  }
}

// One row, 86 px high; touching it opens the sub-page.
inline void room_row(Display &it, Rect r, const Room &room) {
  txt(it, r.x, r.y + 14, f_txt, BLACK, TextAlign::TOP_LEFT, room.name);
  if (room.action->state == "heating")
    txt(it, r.x + 360, r.y + 8, i_s, BLACK, TextAlign::TOP_LEFT, "\U000F0238");
  txt(it, r.x2() - 134, r.y, f_h1, BLACK, TextAlign::TOP_RIGHT, fmt(room.cur->state) + "°");
  txt(it, r.x2(), r.y + 2, f_small, GRAY, TextAlign::TOP_RIGHT, fmt(room.target->state) + "°");
  txt(it, r.x2(), r.y + 38, f_small, GRAY, TextAlign::TOP_RIGHT, fmt(room.hum->state, 0) + " %");
  zone({r.x, (int16_t) (r.y - 8), r.w, r.h}, [room] {
    room.msg = "";
    open(room.key, [room](Display &it) { room_page(it, room); });
  });
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
