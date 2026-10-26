// Header: today's date, last update time, power source (mains or battery %).
// With `calendar` (id of a widgets/agenda.yaml), touching the date opens its month view.
#pragma once
#include "ui.h"

namespace ui {

inline void header(Display &it, Rect r, const char *calendar = nullptr) {
  const Strings &t = tr();
  auto now = ha_time->now();
  if (now.is_valid()) {
    const std::string day = t.days[now.day_of_week - 1], num = std::to_string(now.day_of_month),
                      month = t.months[now.month - 1];
    txt(it, r.x, r.y, f_h1, BLACK, TextAlign::TOP_LEFT,
        t.day_before_month ? day + " " + num + " " + month : day + ", " + month + " " + num);
    txt(it, r.x2() - 264, r.y + 22, f_small, GRAY, TextAlign::TOP_RIGHT, t.updated + now.strftime("%H:%M"));
  }
  if (calendar) zone({(int16_t) (r.x - 20), (int16_t) (r.y - 20), 1000, (int16_t) (r.h + 20)}, [calendar] { show(calendar); });
  const bool mains = on_mains->value();
  const std::string power_txt = mains ? std::string(t.mains) : fmt(battery_level->state, 0) + " %";
  int bx, by, bw, bh;
  it.get_text_bounds(r.x2(), r.y + 22, power_txt.c_str(), f_small, TextAlign::TOP_RIGHT, &bx, &by, &bw, &bh);
  txt(it, r.x2(), r.y + 22, f_small, GRAY, TextAlign::TOP_RIGHT, power_txt);
  txt(it, bx - 8, r.y + 10, i_s, mains ? BLACK : GRAY, TextAlign::TOP_RIGHT, mains ? "\U000F06A5" : "\U000F0079");
}

}  // namespace ui
