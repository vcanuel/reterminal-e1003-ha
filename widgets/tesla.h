// Tesla through TeslaMate (MQTT): the car tile (car.h); touching it opens a read-only sub-page
// with charge, vehicle, climate, tyres and software.
#pragma once
#include <ctime>
#include "car.h"

namespace ui {

struct Tesla {
  const char *key;  // widget id (sub-page key)
  const char *name;
  sensor::Sensor *battery, *range, *charger_power, *limit, *time_to_full, *energy_added;
  sensor::Sensor *inside, *outside, *odometer, *tpms[4];  // tyres: front left, front right, rear left, rear right
  binary_sensor::BinarySensor *plugged, *port_open, *locked, *doors, *windows, *trunk, *frunk, *sentry;
  binary_sensor::BinarySensor *climate_on, *preconditioning, *update_available;
  text_sensor::TextSensor *state, *scheduled, *version, *update_version;
};

// Label (gray) on the left, value (bold) on the right of a w-wide row.
inline void kv(Display &it, int x, int y, int w, const char *label, const std::string &value) {
  txt(it, x, y, f_txt, GRAY, TextAlign::TOP_LEFT, label);
  txt(it, x + w, y, f_txt_b, BLACK, TextAlign::TOP_RIGHT, value);
}

// "2026-10-26T21:00:00+00:00" -> local "22:00", empty when not a date.
inline std::string local_hm(const std::string &iso) {
  int y, mo, d, h, mi, s = 0, oh = 0, om = 0;
  char sign = '+';
  if (sscanf(iso.c_str(), "%d-%d-%dT%d:%d:%d%c%d:%d", &y, &mo, &d, &h, &mi, &s, &sign, &oh, &om) < 5) return "";
  // Days since 1970-01-01 (H. Hinnant's days_from_civil).
  y -= mo <= 2;
  const int era = y / 400, yoe = y - era * 400, doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const long days = era * 146097L + yoe * 365 + yoe / 4 - yoe / 100 + doy - 719468;
  const int off = (sign == '-' ? -1 : 1) * (oh * 3600 + om * 60);
  time_t t = days * 86400 + h * 3600 + mi * 60 + s - off;
  struct tm lt;
  localtime_r(&t, &lt);
  char buf[8];
  strftime(buf, sizeof(buf), "%H:%M", &lt);
  return buf;
}

inline std::string thousands(float v) {
  if (std::isnan(v)) return "--";
  std::string s = fmt(v, 0);
  for (int i = (int) s.size() - 3; i > 0; i -= 3) s.insert(i, " ");
  return s;
}

inline void tesla_page(Display &it, const Tesla &c) {
  const Strings &t = tr();
  page_header(it, c.name);
  const bool charging = c.charger_power->state > 0;

  // Battery: level, range, gauge with the charge limit.
  txt(it, 48, 190, f_xl, BLACK, TextAlign::TOP_LEFT, fmt(c.battery->state, 0) + " %");
  txt(it, 52, 400, f_h2, BLACK, TextAlign::TOP_LEFT, fmt(c.range->state, 0) + " km");
  const int gx = 48, gy = 480, gw = 852, gh = 44;
  for (int b = 0; b < 3; b++) it.rectangle(gx + b, gy + b, gw - 2 * b, gh - 2 * b, BLACK);
  if (!std::isnan(c.battery->state))
    it.filled_rectangle(gx + 6, gy + 6, (int) ((gw - 12) * std::min(100.f, c.battery->state) / 100), gh - 12, GRAY);
  if (!std::isnan(c.limit->state)) {
    const int lx = gx + (int) (gw * c.limit->state / 100);
    it.filled_rectangle(lx - 2, gy - 12, 5, gh + 24, BLACK);
  }

  // Charge.
  const int cx = 1000, cw = 824;
  const char *status = charging ? t.ch_charging : c.plugged->state ? t.ch_plugged : t.ch_unplugged;
  txt(it, cx, 190, i_s, BLACK, TextAlign::TOP_LEFT, charging ? "\U000F0084" : "\U000F06A5");
  txt(it, cx + 70, 192, f_h2, BLACK, TextAlign::TOP_LEFT, status);
  int y = 275;
  auto row = [&](const char *label, const std::string &value) {
    kv(it, cx, y, cw, label, value);
    y += 62;
  };
  if (charging) {
    row(t.ch_power, fmt(c.charger_power->state) + " kW");
    const float h = c.time_to_full->state;
    if (!std::isnan(h) && h > 0) {
      char buf[16];
      snprintf(buf, sizeof(buf), "%d h %02d", (int) h, (int) std::lround((h - (int) h) * 60) % 60);
      row(t.ch_full_in, buf);
    }
  }
  if (c.plugged->state || charging) row(t.ch_added, fmt(c.energy_added->state) + " kWh");
  row(t.ch_limit, fmt(c.limit->state, 0) + " %");
  const std::string sched = local_hm(c.scheduled->state);
  if (c.plugged->state && !charging && !sched.empty()) row(t.ch_scheduled, sched);
  if (c.port_open->state && !c.plugged->state) row(t.ch_port, t.car_open);

  // Vehicle and climate.
  it.filled_rectangle(48, 600, 1776, 3, LIGHT);
  txt(it, 48, 625, f_h2, BLACK, TextAlign::TOP_LEFT, t.vehicle);
  txt(it, cx, 625, f_h2, BLACK, TextAlign::TOP_LEFT, t.climate);
  std::string open;
  for (auto &o : {std::make_pair(c.doors, t.doors), std::make_pair(c.windows, t.windows),
                  std::make_pair(c.trunk, t.trunk), std::make_pair(c.frunk, t.frunk)})
    if (o.first->state) open += (open.empty() ? "" : ", ") + std::string(o.second);
  if (!open.empty()) open[0] = toupper(open[0]);
  const std::string st = c.state->state;
  const char *state = st == "asleep" ? t.asleep : st == "online" ? t.online : st == "offline" ? t.offline
                    : st == "driving" ? t.driving : st == "charging" ? t.ch_charging : st == "updating" ? t.updating
                    : st == "suspended" ? t.suspended : st.c_str();
  kv(it, 48, 700, 852, t.car_lock, c.locked->state ? t.locked : t.unlocked);
  kv(it, 48, 762, 852, t.openings, open.empty() ? t.all_closed : open);
  kv(it, 48, 824, 852, t.sentry, c.sentry->state ? t.on : t.off);
  kv(it, 48, 886, 852, t.car_state, state);
  kv(it, cx, 700, cw, t.inside, fmt(c.inside->state) + "°");
  kv(it, cx, 762, cw, t.outside, fmt(c.outside->state) + "°");
  kv(it, cx, 824, cw, t.ac, c.preconditioning->state ? t.preconditioning : c.climate_on->state ? t.ac_on : t.off);

  // Tyres and other.
  it.filled_rectangle(48, 975, 1776, 3, LIGHT);
  txt(it, 48, 1000, f_h2, BLACK, TextAlign::TOP_LEFT, t.tyres);
  txt(it, cx, 1000, f_h2, BLACK, TextAlign::TOP_LEFT, t.other);
  const char *wheels[4] = {t.front_left, t.front_right, t.rear_left, t.rear_right};
  for (int i = 0; i < 4; i++) kv(it, 48, 1075 + i * 62, 852, wheels[i], fmt(c.tpms[i]->state, 2) + " bar");
  kv(it, cx, 1075, cw, t.odometer, thousands(c.odometer->state) + " km");
  kv(it, cx, 1137, cw, t.software, c.version->state);
  if (c.update_available->state) kv(it, cx, 1199, cw, t.update, c.update_version->state + t.available);
}

// Dashboard tile (suggested area: 370 x 220); touching it opens the sub-page.
inline void tesla_tile(Display &it, Rect r, const Tesla &c) {
  car(it, r, {c.name, c.battery, c.range, c.charger_power, c.plugged});
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)},
       [c] { open(c.key, [c](Display &it) { tesla_page(it, c); }); });
}

}  // namespace ui
