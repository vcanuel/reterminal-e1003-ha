// Weather: current conditions (icon, temperature, humidity, wind) + 5-day forecast.
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
  text_sensor::TextSensor *state;
  sensor::Sensor *temp, *hum, *wind;
  const std::string &forecast;  // one line per day: condition|max|min|precipitation|weekday (0 = Monday)
  font::Font *icon_xl, *icon_m, *temp_xl;
};

// Suggested area: 980 x 620.
inline void weather(Display &it, Rect r, const Weather &w) {
  const Strings &t = tr();
  const std::string cond = w.state->state;
  txt(it, r.x, r.y + 10, w.icon_xl, BLACK, TextAlign::TOP_LEFT, weather_icon(cond));
  txt(it, r.x + 340, r.y, w.temp_xl, BLACK, TextAlign::TOP_LEFT, fmt(w.temp->state) + "°");
  txt(it, r.x + 352, r.y + 250, f_h2, BLACK, TextAlign::TOP_LEFT, weather_label(cond));
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
}

}  // namespace ui
