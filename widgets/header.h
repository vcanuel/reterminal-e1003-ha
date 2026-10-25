// En-tête : date du jour, heure de mise à jour, alimentation (secteur ou % batterie).
#pragma once
#include "ui.h"

namespace ui {

inline void header(Display &it, Rect r) {
  auto now = ha_time->now();
  if (now.is_valid()) {
    txt(it, r.x, r.y, f_h1, BLACK, TextAlign::TOP_LEFT,
        std::string(JOURS[now.day_of_week - 1]) + " " + std::to_string(now.day_of_month) + " " + MOIS[now.month - 1]);
    txt(it, r.x2() - 264, r.y + 22, f_small, GRAY, TextAlign::TOP_RIGHT, "Mis à jour " + now.strftime("%H:%M"));
  }
  const bool mains = on_mains->value();
  const std::string power_txt = mains ? std::string("Secteur") : fmt(battery_level->state, 0) + " %";
  int bx, by, bw, bh;
  it.get_text_bounds(r.x2(), r.y + 22, power_txt.c_str(), f_small, TextAlign::TOP_RIGHT, &bx, &by, &bw, &bh);
  txt(it, r.x2(), r.y + 22, f_small, GRAY, TextAlign::TOP_RIGHT, power_txt);
  txt(it, bx - 8, r.y + 10, i_s, mains ? BLACK : GRAY, TextAlign::TOP_RIGHT, mains ? "\U000F06A5" : "\U000F0079");
}

}  // namespace ui
