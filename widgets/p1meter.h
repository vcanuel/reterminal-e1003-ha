// Electricity from a HomeWizard P1 meter: the power tile (power.h); touching it opens a sub-page with the
// current rate, the capacity tariff (quarter-hour average vs. peak of the month), the last 24 hours and 14 days.
#pragma once
#include "power.h"

namespace ui {

struct P1Meter {
  const char *key;  // widget id (sub-page key)
  const char *title, *sub;
  sensor::Sensor *power, *average, *peak;  // W
  text_sensor::TextSensor *tariff;         // "1" = day, "2" = night / weekend
  const std::string &hours;  // last 24 hours: hour:kWh,hour:kWh,...
  const std::string &days;   // last 14 days, one line per day: weekday (0 = Monday)|day kWh|night kWh
};

inline std::string watts(float w) {
  if (std::isnan(w)) return "--";
  return w >= 1000 ? fmt(w / 1000, 2) + " kW" : fmt(w, 0) + " W";
}

// Bars in r (bottom-aligned, scaled to max), one per value: dark part, then light part on top.
inline void bars(Display &it, Rect r, const std::vector<std::pair<float, float>> &v, float max, int gap) {
  if (v.empty() || max <= 0) return;
  const int n = v.size(), w = (r.w - (n - 1) * gap) / n;
  for (int i = 0; i < n; i++) {
    const int x = r.x + i * (w + gap), h1 = (int) (r.h * v[i].first / max), h2 = (int) (r.h * v[i].second / max);
    if (h1 > 0) it.filled_rectangle(x, r.y + r.h - h1, w, h1, BLACK);
    if (h2 > 0) it.filled_rectangle(x, r.y + r.h - h1 - h2, w, h2, GRAY);
  }
  it.filled_rectangle(r.x, r.y + r.h, r.w, 2, BLACK);
}

inline void p1_page(Display &it, const P1Meter &m) {
  const Strings &t = tr();
  page_header(it, m.title);

  // Now: power and rate.
  txt(it, 48, 190, f_xl, BLACK, TextAlign::TOP_LEFT, watts(m.power->state));
  const bool night = m.tariff->state == "2";
  if (m.tariff->state == "1" || night) {
    txt(it, 52, 405, i_s, BLACK, TextAlign::TOP_LEFT, night ? "\U000F0594" : "\U000F05A8");
    txt(it, 122, 410, f_h2, BLACK, TextAlign::TOP_LEFT, night ? t.rate_night : t.rate_day);
  }

  // Capacity tariff: quarter-hour average vs. peak of the month.
  const int cx = 1000, cw = 824;
  txt(it, cx, 190, f_h2, BLACK, TextAlign::TOP_LEFT, t.capacity);
  kv(it, cx, 265, cw, t.quarter, watts(m.average->state));
  kv(it, cx, 327, cw, t.month_peak, watts(m.peak->state));
  const int gy = 410, gh = 44;
  for (int b = 0; b < 3; b++) it.rectangle(cx + b, gy + b, cw - 2 * b, gh - 2 * b, BLACK);
  if (!std::isnan(m.average->state) && m.peak->state > 0)
    it.filled_rectangle(cx + 6, gy + 6, (int) ((cw - 12) * std::min(1.f, m.average->state / m.peak->state)), gh - 12,
                        m.average->state > 0.8f * m.peak->state ? BLACK : GRAY);

  // Last 24 hours.
  it.filled_rectangle(48, 500, 1776, 3, LIGHT);
  txt(it, 48, 520, f_h2, BLACK, TextAlign::TOP_LEFT, t.last_24h);
  std::vector<std::pair<float, float>> hv;
  std::vector<int> hh;
  float hmax = 0;
  for (auto &p : split(m.hours, ',')) {
    auto f = split(p, ':');
    if (f.size() < 2) continue;
    hh.push_back(atoi(f[0].c_str()));
    hv.push_back({std::max(0.f, (float) atof(f[1].c_str())), 0});
    hmax = std::max(hmax, hv.back().first);
  }
  const Rect hr{48, 600, 1776, 230};
  if (hmax > 0) {
    txt(it, 1824, 528, f_small, GRAY, TextAlign::TOP_RIGHT, "max " + fmt(hmax, 2) + " kWh");
    bars(it, hr, hv, hmax, 10);
    const int n = hv.size(), w = (hr.w - (n - 1) * 10) / n;
    for (int i = 0; i < n; i++)
      if (hh[i] % 3 == 0) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%02dh", hh[i]);
        txt(it, hr.x + i * (w + 10) + w / 2, 842, f_small, GRAY, TextAlign::TOP_CENTER, buf);
      }
  }

  // Last 14 days: day / night, totals.
  it.filled_rectangle(48, 900, 1776, 3, LIGHT);
  txt(it, 48, 920, f_h2, BLACK, TextAlign::TOP_LEFT, t.last_14d);
  it.filled_rectangle(1420, 932, 28, 28, BLACK);
  txt(it, 1460, 926, f_small, GRAY, TextAlign::TOP_LEFT, t.day);
  it.filled_rectangle(1620, 932, 28, 28, GRAY);
  txt(it, 1660, 926, f_small, GRAY, TextAlign::TOP_LEFT, t.night);
  std::vector<std::pair<float, float>> dv;
  std::vector<int> wd;
  float dmax = 0;
  for (auto &line : split(m.days, '\n')) {
    auto f = split(line, '|');
    if (f.size() < 3) continue;
    wd.push_back(atoi(f[0].c_str()));
    dv.push_back({std::max(0.f, (float) atof(f[1].c_str())), std::max(0.f, (float) atof(f[2].c_str()))});
    dmax = std::max(dmax, dv.back().first + dv.back().second);
  }
  const Rect dr{48, 1000, 1776, 220};
  if (dmax > 0) {
    bars(it, dr, dv, dmax, 24);
    const int n = dv.size(), w = (dr.w - (n - 1) * 24) / n;
    for (int i = 0; i < n; i++)
      txt(it, dr.x + i * (w + 24) + w / 2, 1232, f_small, GRAY, TextAlign::TOP_CENTER, t.days_short[wd[i] % 7]);
    auto total = [&](int i) { return dv[i].first + dv[i].second; };
    float sum = 0;
    for (int i = 0; i + 1 < n; i++) sum += total(i);  // full days only
    std::string line = std::string(t.today_long) + " " + fmt(total(n - 1)) + " kWh";
    if (n >= 2) line += "   ·   " + std::string(t.yesterday) + " " + fmt(total(n - 2)) + " kWh";
    if (n >= 2) line += "   ·   " + std::string(t.average) + " " + fmt(sum / (n - 1)) + " kWh" + t.per_day;
    txt(it, 48, 1300, f_txt, BLACK, TextAlign::TOP_LEFT, line);
  }
}

// Dashboard tile (suggested area: 370 x 220); touching it opens the sub-page.
inline void p1_tile(Display &it, Rect r, const P1Meter &m) {
  power(it, r, m.title, m.sub, m.power);
  auto show_page = [m] { open(m.key, [m](Display &it) { p1_page(it, m); }); };
  can_open(m.key, show_page);
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)}, show_page);
}

}  // namespace ui
