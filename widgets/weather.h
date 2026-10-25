// Météo : conditions actuelles (icône, température, humidité, vent) + prévisions sur 5 jours.
#pragma once
#include "ui.h"

namespace ui {

struct WeatherCond {
  const char *key;
  const char *label;
  const char *icon;
};

static const WeatherCond WEATHER_CONDS[] = {
    {"clear-night", "Nuit claire", "\U000F0594"},
    {"cloudy", "Nuageux", "\U000F0590"},
    {"fog", "Brouillard", "\U000F0591"},
    {"hail", "Grêle", "\U000F0592"},
    {"lightning", "Orage", "\U000F0593"},
    {"lightning-rainy", "Orage et pluie", "\U000F067E"},
    {"partlycloudy", "Éclaircies", "\U000F0595"},
    {"pouring", "Averses", "\U000F0596"},
    {"rainy", "Pluie", "\U000F0597"},
    {"snowy", "Neige", "\U000F0598"},
    {"snowy-rainy", "Neige fondue", "\U000F067F"},
    {"sunny", "Ensoleillé", "\U000F0599"},
    {"windy", "Venteux", "\U000F059D"},
    {"windy-variant", "Venteux", "\U000F059E"},
};

inline const WeatherCond *weather_cond(const std::string &key) {
  for (const auto &c : WEATHER_CONDS)
    if (key == c.key) return &c;
  return nullptr;
}
inline const char *weather_label(const std::string &key) {
  auto *c = weather_cond(key);
  return c ? c->label : "--";
}
inline const char *weather_icon(const std::string &key) {
  auto *c = weather_cond(key);
  return c ? c->icon : "\U000F05D6";  // alert-circle-outline
}

struct Weather {
  text_sensor::TextSensor *state;
  sensor::Sensor *temp, *hum, *wind;
  const std::string &forecast;  // une ligne par jour : condition|max|min|précipitations|jour (0 = lundi)
  font::Font *icon_xl, *icon_m, *temp_xl;
};

// Zone conseillée : 980 x 620.
inline void weather(Display &it, Rect r, const Weather &w) {
  const std::string cond = w.state->state;
  txt(it, r.x, r.y + 10, w.icon_xl, BLACK, TextAlign::TOP_LEFT, weather_icon(cond));
  txt(it, r.x + 340, r.y, w.temp_xl, BLACK, TextAlign::TOP_LEFT, fmt(w.temp->state) + "°");
  txt(it, r.x + 352, r.y + 250, f_h2, BLACK, TextAlign::TOP_LEFT, weather_label(cond));
  txt(it, r.x + 352, r.y + 310, f_small, GRAY, TextAlign::TOP_LEFT,
      "Humidité " + fmt(w.hum->state, 0) + " %   ·   Vent " + fmt(w.wind->state, 0) + " km/h");

  auto days = split(w.forecast, '\n');
  for (size_t i = 0; i < days.size() && i < 5; i++) {
    auto f = split(days[i], '|');
    if (f.size() < 5) continue;
    const int cx = r.x + 8 + i * 190 + 95, y0 = r.y + 375;
    int wd = atoi(f[4].c_str());
    txt(it, cx, y0, f_txt_b, BLACK, TextAlign::TOP_CENTER, i == 0 ? "Auj." : JOURS_COURTS[wd % 7]);
    txt(it, cx, y0 + 48, w.icon_m, BLACK, TextAlign::TOP_CENTER, weather_icon(f[0]));
    txt(it, cx - 6, y0 + 155, f_txt_b, BLACK, TextAlign::TOP_RIGHT, f[1] + "°");
    txt(it, cx + 6, y0 + 155, f_txt, GRAY, TextAlign::TOP_LEFT, f[2] + "°");
    if (atof(f[3].c_str()) > 0)
      txt(it, cx, y0 + 200, f_small, GRAY, TextAlign::TOP_CENTER, f[3] + " mm");
  }
}

}  // namespace ui
