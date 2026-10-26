// Weather: current conditions (icon, temperature, humidity, wind) + 5-day forecast.
// Touching it opens a sub-page with more details, the next 12 hours and 7 days.
#pragma once
#include "ui.h"

namespace ui {

struct WeatherCond {
  const char *key;
  const char *icon;
};

// Order matches Strings::conditions.
static const WeatherCond WEATHER_CONDS[] = {
    {"clear-night", "\U000F0594"},  {"cloudy", "\U000F0590"},        {"fog", "\U000F0591"},
    {"hail", "\U000F0592"},         {"lightning", "\U000F0593"},     {"lightning-rainy", "\U000F067E"},
    {"partlycloudy", "\U000F0595"}, {"pouring", "\U000F0596"},       {"rainy", "\U000F0597"},
    {"snowy", "\U000F0598"},        {"snowy-rainy", "\U000F067F"},   {"sunny", "\U000F0599"},
    {"windy", "\U000F059D"},        {"windy-variant", "\U000F059E"},
};

inline int weather_cond(const std::string &key) {
  for (int i = 0; i < 14; i++)
    if (key == WEATHER_CONDS[i].key) return i;
  return -1;
}
inline const char *weather_label(const std::string &key) {
  const int i = weather_cond(key);
  return i >= 0 ? tr().conditions[i] : "--";
}
inline const char *weather_icon(const std::string &key) {
  const int i = weather_cond(key);
  return i >= 0 ? WEATHER_CONDS[i].icon : "\U000F05D6";  // alert-circle-outline
}

struct Weather {
  const char *key;  // widget id (sub-page key)
  text_sensor::TextSensor *state;
  sensor::Sensor *temp, *hum, *wind;
  // One line per day: condition|max|min|precipitation|weekday (0 = Monday)|precipitation probability|wind
  const std::string &forecast;
  // Current details, formatted with their unit (empty when unknown):
  // feels like|dew point|pressure|wind|wind bearing (degrees)|gusts|UV index|cloud cover|visibility
  const std::string &details;
  const std::string &hourly;  // one line per hour: condition|temperature|precipitation probability|precipitation|hour
  font::Font *icon_xl, *icon_m, *temp_xl;
};

// "3.5 mm · 80 %", empty when no precipitation is expected.
inline std::string weather_precip(const std::string &mm, const std::string &prob) {
  std::string s;
  if (atof(mm.c_str()) > 0) s = mm + " mm";
  if (atoi(prob.c_str()) > 0) s += (s.empty() ? "" : " · ") + prob + " %";
  return s;
}

// Big icon, temperature and condition (dashboard and sub-page).
inline void weather_now(Display &it, int x, int y, const Weather &w) {
  const std::string cond = w.state->state;
  txt(it, x, y + 10, w.icon_xl, BLACK, TextAlign::TOP_LEFT, weather_icon(cond));
  txt(it, x + 340, y, w.temp_xl, BLACK, TextAlign::TOP_LEFT, fmt(w.temp->state) + "°");
  txt(it, x + 352, y + 250, f_h2, BLACK, TextAlign::TOP_LEFT, weather_label(cond));
}

// Full-screen sub-page: current details, next 12 hours, next 7 days.
inline void weather_page(Display &it, const Weather &w) {
  const Strings &t = tr();
  page_header(it, t.weather_title);
  weather_now(it, 48, 190, w);

  // Details: two columns of label / value, unknown values skipped.
  auto d = split(w.details, '|');
  d.resize(9);
  std::string wind = d[3];
  if (!d[4].empty()) wind += " " + std::string(t.compass[(int) lround(atof(d[4].c_str()) / 45) % 8]);
  const std::pair<const char *, std::string> rows[] = {
      {t.feels_like, d[0]}, {t.humidity, std::isnan(w.hum->state) ? "" : fmt(w.hum->state, 0) + " %"},
      {t.wind, wind},       {t.gusts, d[5]},
      {t.pressure, d[2]},   {t.uv_index, d[6]},
      {t.clouds, d[7]},     {t.visibility, d[8]},
      {t.dew_point, d[1]},
  };
  int n = 0;
  for (auto &row : rows) {
    if (row.second.empty() || n == 8) continue;
    const int x = 1080 + (n % 2) * 400, y = 210 + (n / 2) * 75;
    txt(it, x, y, f_small, GRAY, TextAlign::TOP_LEFT, row.first);
    txt(it, x + 350, y - 4, f_txt_b, BLACK, TextAlign::TOP_RIGHT, row.second);
    n++;
  }

  // Next hours: 12 columns.
  auto hours = split(w.hourly, '\n');
  if (!hours.empty()) {
    it.filled_rectangle(48, 525, 1776, 3, LIGHT);
    txt(it, 48, 545, f_h2, BLACK, TextAlign::TOP_LEFT, t.next_hours);
    for (size_t i = 0; i < hours.size() && i < 12; i++) {
      auto f = split(hours[i], '|');
      if (f.size() < 5) continue;
      const int cx = 48 + i * 148 + 74, y0 = 620;
      char hh[8];
      snprintf(hh, sizeof(hh), "%02d:00", atoi(f[4].c_str()));
      txt(it, cx, y0, f_txt_b, BLACK, TextAlign::TOP_CENTER, hh);
      txt(it, cx, y0 + 48, w.icon_m, BLACK, TextAlign::TOP_CENTER, weather_icon(f[0]));
      txt(it, cx, y0 + 155, f_txt_b, BLACK, TextAlign::TOP_CENTER, f[1] + "°");
      if (atoi(f[2].c_str()) > 0) txt(it, cx, y0 + 202, f_small, GRAY, TextAlign::TOP_CENTER, f[2] + " %");
      if (atof(f[3].c_str()) > 0) txt(it, cx, y0 + 240, f_small, GRAY, TextAlign::TOP_CENTER, f[3] + " mm");
    }
  }

  // Next days: 7 columns.
  it.filled_rectangle(48, 915, 1776, 3, LIGHT);
  txt(it, 48, 935, f_h2, BLACK, TextAlign::TOP_LEFT, t.next_days);
  auto days = split(w.forecast, '\n');
  for (size_t i = 0; i < days.size() && i < 7; i++) {
    auto f = split(days[i], '|');
    if (f.size() < 5) continue;
    f.resize(7);
    const int cx = 48 + i * 253 + 126, y0 = 1015;
    txt(it, cx, y0, f_txt_b, BLACK, TextAlign::TOP_CENTER, i == 0 ? t.today : t.days_short[atoi(f[4].c_str()) % 7]);
    txt(it, cx, y0 + 48, w.icon_m, BLACK, TextAlign::TOP_CENTER, weather_icon(f[0]));
    txt(it, cx - 6, y0 + 155, f_txt_b, BLACK, TextAlign::TOP_RIGHT, f[1] + "°");
    txt(it, cx + 6, y0 + 155, f_txt, GRAY, TextAlign::TOP_LEFT, f[2] + "°");
    txt(it, cx, y0 + 202, f_small, GRAY, TextAlign::TOP_CENTER, weather_precip(f[3], f[5]));
    txt(it, cx, y0 + 242, f_small, GRAY, TextAlign::TOP_CENTER, f[6]);
  }
}

// Suggested area: 980 x 620; touching it opens the sub-page.
inline void weather(Display &it, Rect r, const Weather &w) {
  const Strings &t = tr();
  weather_now(it, r.x, r.y, w);
  txt(it, r.x + 352, r.y + 310, f_small, GRAY, TextAlign::TOP_LEFT,
      t.humidity + fmt(w.hum->state, 0) + " %   ·   " + t.wind + fmt(w.wind->state, 0) + " km/h");

  auto days = split(w.forecast, '\n');
  for (size_t i = 0; i < days.size() && i < 5; i++) {
    auto f = split(days[i], '|');
    if (f.size() < 5) continue;
    const int cx = r.x + 8 + i * 190 + 95, y0 = r.y + 375;
    int wd = atoi(f[4].c_str());
    txt(it, cx, y0, f_txt_b, BLACK, TextAlign::TOP_CENTER, i == 0 ? t.today : t.days_short[wd % 7]);
    txt(it, cx, y0 + 48, w.icon_m, BLACK, TextAlign::TOP_CENTER, weather_icon(f[0]));
    txt(it, cx - 6, y0 + 155, f_txt_b, BLACK, TextAlign::TOP_RIGHT, f[1] + "°");
    txt(it, cx + 6, y0 + 155, f_txt, GRAY, TextAlign::TOP_LEFT, f[2] + "°");
    if (atof(f[3].c_str()) > 0)
      txt(it, cx, y0 + 200, f_small, GRAY, TextAlign::TOP_CENTER, f[3] + " mm");
  }
  zone(r, [w] { open(w.key, [w](Display &it) { weather_page(it, w); }); });
}

}  // namespace ui
