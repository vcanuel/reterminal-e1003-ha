// Calendar: next events from one or more calendars; touching it (or the header date) opens a month view.
#pragma once
#include <ctime>
#include "ui.h"

namespace ui {

// events: one line per event, days_from_today|weekday (0 = Monday)|day_of_month|time|title
// (see widgets/agenda.yaml).
// ---------- Month view ----------
struct Month {
  const char *key;      // widget id (sub-page key)
  int &offset;          // months from the current one
  std::string &events;  // one line per event: start|end (exclusive)|time|title, dates as YYYY-MM-DD
  bool &loading;        // waiting for calendar.get_events
  void (*fetch)();      // fetches the events of the displayed grid
};

// Grid of the displayed month: Monday of the first week, number of weeks (local noon, safe for DST).
struct MonthGrid {
  int year, month;  // displayed month (1-12)
  struct tm first;  // first cell (a Monday)
  int weeks;
};
inline MonthGrid month_grid(int offset) {
  auto now = ha_time->now();
  int y = now.is_valid() ? now.year : 2026, m = (now.is_valid() ? now.month : 1) - 1 + offset;
  y += m >= 0 ? m / 12 : (m - 11) / 12;
  m = ((m % 12) + 12) % 12;
  struct tm t = {};
  t.tm_year = y - 1900;
  t.tm_mon = m;
  t.tm_mday = 1;
  t.tm_hour = 12;
  t.tm_isdst = -1;
  mktime(&t);
  const int wd = (t.tm_wday + 6) % 7;  // Monday = 0
  struct tm last = t;
  last.tm_mon++;
  last.tm_mday = 0;
  mktime(&last);
  struct tm first = t;
  first.tm_mday = 1 - wd;
  mktime(&first);
  return {y, m + 1, first, (wd + last.tm_mday + 6) / 7};
}
inline struct tm cell_day(const MonthGrid &g, int i) {
  struct tm d = g.first;
  d.tm_mday += i;
  d.tm_isdst = -1;
  mktime(&d);
  return d;
}
inline std::string ymd(const struct tm &d) {
  char buf[16];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &d);
  return buf;
}
// "YYYY-MM-DD 00:00:00" of the first cell and of the day after the last one, for calendar.get_events.
inline std::string month_start(int offset) { return ymd(month_grid(offset).first) + " 00:00:00"; }
inline std::string month_end(int offset) {
  const MonthGrid g = month_grid(offset);
  return ymd(cell_day(g, g.weeks * 7)) + " 00:00:00";
}

// s cut (with "...") to fit in w pixels.
inline std::string clip(Display &it, std::string s, font::Font *f, int w) {
  int x1, y1, tw, th;
  it.get_text_bounds(0, 0, s.c_str(), f, TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
  if (tw <= w) return s;
  while (!s.empty()) {
    do s.pop_back();
    while (!s.empty() && (s.back() & 0xC0) == 0x80);  // whole UTF-8 characters
    it.get_text_bounds(0, 0, (s + "...").c_str(), f, TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
    if (tw <= w) break;
  }
  return s + "...";
}

inline void month_page(Display &it, const Month &m);

// Opens the month view on the current month; the screen is redrawn when the events arrive.
inline void month_open(const Month &m) {
  m.offset = 0;
  m.loading = true;
  m.fetch();
  open(m.key, [m](Display &it) { month_page(it, m); });
  hold();
}

inline void month_page(Display &it, const Month &m) {
  const Strings &t = tr();
  const MonthGrid g = month_grid(m.offset);
  auto go = [m](int delta) {
    return [m, delta] {
      m.offset = delta == 0 ? 0 : m.offset + delta;
      m.loading = true;
      m.fetch();
      hold();
    };
  };

  // Header: Back, month (touch: back to the current month), previous / next.
  button(it, {48, 30, 300, 100}, t.back, false, ui::close);
  std::string title = std::string(t.months[g.month - 1]) + " " + std::to_string(g.year);
  title[0] = toupper(title[0]);
  txt(it, 836, 50, f_h1, BLACK, TextAlign::TOP_CENTER, title);
  if (m.offset != 0) zone({400, 20, 870, 120}, go(0));
  if (m.loading) txt(it, 836, 125, f_small, GRAY, TextAlign::TOP_CENTER, t.loading);
  button(it, {1324, 30, 240, 100}, "", false, go(-1));
  txt(it, 1444, 80, i_s, BLACK, TextAlign::CENTER, "\U000F0141");
  button(it, {1584, 30, 240, 100}, "", false, go(1));
  txt(it, 1704, 80, i_s, BLACK, TextAlign::CENTER, "\U000F0142");
  it.filled_rectangle(48, 160, 1776, 4, BLACK);

  // Events of the grid.
  struct Ev {
    std::string start, end, time, title;
  };
  std::vector<Ev> evs;
  for (auto &line : split(m.events, '\n')) {
    auto f = split(line, '|');
    if (f.size() >= 4) evs.push_back({f[0], f[1], f[2], f[3]});
  }

  // Grid: weekday names, then one row per week.
  const int cw = 1776 / 7, top = 235, ch = (1395 - top) / g.weeks, lines = (ch - 62) / 40;
  for (int c = 0; c < 7; c++)
    txt(it, 48 + c * cw + cw / 2, 185, f_small, GRAY, TextAlign::TOP_CENTER, t.days_short[c]);
  for (int w = 0; w <= g.weeks; w++) it.filled_rectangle(48, top + w * ch, 1776, 1, LIGHT);
  for (int c = 1; c < 7; c++) it.filled_rectangle(48 + c * cw, top, 1, g.weeks * ch, LIGHT);
  auto now = ha_time->now();
  for (int i = 0; i < g.weeks * 7; i++) {
    const struct tm d = cell_day(g, i);
    const std::string day = ymd(d);
    const int x = 48 + (i % 7) * cw, y = top + (i / 7) * ch;
    const bool in_month = d.tm_mon + 1 == g.month;
    const bool today = now.is_valid() && d.tm_year + 1900 == now.year && d.tm_mon + 1 == now.month &&
                       d.tm_mday == now.day_of_month;
    const std::string num = std::to_string(d.tm_mday);
    if (today) {
      it.filled_rectangle(x + 6, y + 6, 66, 50, BLACK);
      it.print(x + 39, y + 8, f_txt_b, WHITE, TextAlign::TOP_CENTER, num.c_str(), BLACK);
    } else {
      txt(it, x + 12, y + 8, in_month ? f_txt_b : f_txt, in_month ? BLACK : LIGHT, TextAlign::TOP_LEFT, num);
    }
    // Events covering this day: all-day ones on a gray band, then timed ones with their time.
    std::vector<const Ev *> day_evs;
    for (auto &e : evs)
      if (e.start <= day && day < e.end) day_evs.push_back(&e);
    std::stable_sort(day_evs.begin(), day_evs.end(), [](const Ev *a, const Ev *b) { return a->time < b->time; });
    const int n = day_evs.size(), shown = n > lines ? lines - 1 : n;
    for (int k = 0; k < shown; k++) {
      const Ev &e = *day_evs[k];
      const int ey = y + 62 + k * 40;
      if (e.time.empty()) {
        it.filled_rectangle(x + 6, ey, cw - 12, 38, LIGHT);
        it.print(x + 12, ey + 2, f_small, BLACK, TextAlign::TOP_LEFT, clip(it, e.title, f_small, cw - 24).c_str(), LIGHT);
      } else {
        txt(it, x + 10, ey + 2, f_small, in_month ? BLACK : GRAY, TextAlign::TOP_LEFT,
            clip(it, e.time + " " + e.title, f_small, cw - 18));
      }
    }
    if (n > shown) txt(it, x + 10, y + 62 + shown * 40 + 2, f_small, GRAY, TextAlign::TOP_LEFT, "+" + std::to_string(n - shown));
  }
}

// Suggested area: ~950 wide, 85 + 66 x max_events high; touching it opens the month view (with `month`).
inline void agenda(Display &it, Rect r, const char *label, const std::string &events, int max_events,
                   const Month *month = nullptr) {
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
  if (month) {
    const Month m = *month;
    can_open(m.key, [m] { month_open(m); });
    zone(r, [m] { month_open(m); });
  }
}

}  // namespace ui
