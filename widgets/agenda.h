// Calendar: next events from one or more calendars.
#pragma once
#include "ui.h"

namespace ui {

// events: one line per event, days_from_today|weekday (0 = Monday)|day_of_month|time|title
// (see widgets/agenda.yaml).
inline void agenda(Display &it, Rect r, const char *label, const std::string &events, int max_events) {
  const Strings &t = tr();
  title(it, r.x, r.y, "\U000F0E18", label);
  auto lines = split(events, '\n');
  if (lines.empty())
    txt(it, r.x, r.y + 85, f_txt, GRAY, TextAlign::TOP_LEFT, t.no_events);
  // Day labels first, to size the day column (at least 202 px).
  std::vector<std::vector<std::string>> rows;
  int day_w = 202 - 16;
  for (size_t i = 0; i < lines.size() && (int) i < max_events; i++) {
    auto e = split(lines[i], '|');
    if (e.size() < 5) continue;
    const int delta = atoi(e[0].c_str());
    e[0] = delta <= 0   ? t.today
           : delta == 1 ? t.tomorrow
                        : std::string(t.days_short[atoi(e[1].c_str()) % 7]) + " " + e[2];
    int bx, by, bw, bh;
    it.get_text_bounds(0, 0, e[0].c_str(), f_txt_b, TextAlign::TOP_LEFT, &bx, &by, &bw, &bh);
    day_w = std::max(day_w, bw);
    rows.push_back(e);
  }
  for (size_t i = 0; i < rows.size(); i++) {
    const auto &e = rows[i];
    const int y = r.y + 85 + i * 66, time_x = r.x + day_w + 16;
    txt(it, r.x, y, f_txt_b, BLACK, TextAlign::TOP_LEFT, e[0]);
    txt(it, time_x, y, f_txt, GRAY, TextAlign::TOP_LEFT, e[3]);
    txt(it, time_x + 130, y, f_txt, BLACK, TextAlign::TOP_LEFT, e[4]);
  }
}

}  // namespace ui
