// Agenda : prochains événements d'un ou plusieurs calendriers.
#pragma once
#include "ui.h"

namespace ui {

// events : une ligne par événement, jour|heure|titre (voir widgets/agenda.yaml).
inline void agenda(Display &it, Rect r, const char *label, const std::string &events, int max_events) {
  title(it, r.x, r.y, "\U000F0E18", label);
  auto lines = split(events, '\n');
  if (lines.empty())
    txt(it, r.x, r.y + 85, f_txt, GRAY, TextAlign::TOP_LEFT, "Rien de prévu cette semaine");
  for (size_t i = 0; i < lines.size() && (int) i < max_events; i++) {
    auto e = split(lines[i], '|');
    if (e.size() < 3) continue;
    const int y = r.y + 85 + i * 66;
    txt(it, r.x, y, f_txt_b, BLACK, TextAlign::TOP_LEFT, e[0]);
    txt(it, r.x + 202, y, f_txt, GRAY, TextAlign::TOP_LEFT, e[1]);
    txt(it, r.x + 332, y, f_txt, BLACK, TextAlign::TOP_LEFT, e[2]);
  }
}

}  // namespace ui
