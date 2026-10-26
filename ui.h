// Drawing building blocks shared by the widgets: colors, text, touch zones, sub-pages.
// Widgets (widgets/*.h) draw inside a rectangle and declare their touch zones while drawing,
// so the zones always match what is on screen.
#pragma once
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#include "ui_strings.h"

namespace ui {

using display::Display;
using display::Rect;
using display::TextAlign;
using Draw = std::function<void(Display &, Rect)>;  // type of the globals exposed by the widgets

static const Color BLACK(0, 0, 0, 255), WHITE(255, 255, 255, 255);
static const Color GRAY(100, 100, 100, 255), LIGHT(190, 190, 190, 255);

inline void txt(Display &it, int x, int y, font::Font *f, Color c, TextAlign a, const std::string &s) {
  it.print(x, y, f, c, a, s.c_str(), WHITE);
}

// "12.3", or "--" when the value is not available.
inline std::string fmt(float v, int decimals = 1) {
  if (std::isnan(v)) return "--";
  char buf[16];
  snprintf(buf, sizeof(buf), "%.*f", decimals, v);
  return buf;
}

inline std::vector<std::string> split(const std::string &s, char sep) {
  std::vector<std::string> out;
  size_t start = 0;
  while (start <= s.size()) {
    size_t pos = s.find(sep, start);
    if (pos == std::string::npos) pos = s.size();
    std::string part = s.substr(start, pos - start);
    if (sep != '\n' || !part.empty()) out.push_back(part);
    start = pos + 1;
  }
  return out;
}

// Section title: icon (i_s font) + label.
inline void title(Display &it, int x, int y, const char *icon, const char *label) {
  txt(it, x, y, i_s, BLACK, TextAlign::TOP_LEFT, icon);
  txt(it, x + 70, y + 2, f_h2, BLACK, TextAlign::TOP_LEFT, label);
}

// Tile: icon + label, big value, subtitle.
inline void tile(Display &it, Rect r, const char *icon, const char *label, const std::string &value,
                 const std::string &sub) {
  txt(it, r.x, r.y, i_s, BLACK, TextAlign::TOP_LEFT, icon);
  txt(it, r.x + 70, r.y + 12, f_small, GRAY, TextAlign::TOP_LEFT, label);
  txt(it, r.x, r.y + 72, f_h1, BLACK, TextAlign::TOP_LEFT, value);
  if (!sub.empty()) txt(it, r.x, r.y + 150, f_small, GRAY, TextAlign::TOP_LEFT, sub);
}

// ---------- Touch zones ----------
struct Zone {
  Rect r;
  std::function<void()> fn;
};
inline std::vector<Zone> &zones() {
  static std::vector<Zone> z;
  return z;
}
inline void zone(Rect r, std::function<void()> fn) { zones().push_back({r, std::move(fn)}); }

// Runs the touched zone; false when the touch is outside every zone.
inline bool touch(int x, int y) {
  for (auto &z : zones())
    if (z.r.inside(x, y)) {
      z.fn();
      return true;
    }
  return false;
}

// ---------- Sub-pages (full screen, back to the dashboard after 2 min without touch) ----------
struct Page {
  std::string key;  // id of the owning widget, empty = dashboard
  std::function<void(Display &)> draw;
};
inline Page &page() {
  static Page p;
  return p;
}
inline void open(const std::string &key, std::function<void(Display &)> draw) { page() = {key, std::move(draw)}; }
inline void close() { page() = {}; }
inline bool is_open(const std::string &key) { return page().key == key; }
inline bool on_dashboard() { return page().key.empty(); }

// Opening a sub-page from elsewhere than its tile (e.g. a doorbell ring): the tile registers how to open it
// while drawing; show() opens it, false when no tile with that key has been drawn yet.
inline std::vector<std::pair<std::string, std::function<void()>>> &openers() {
  static std::vector<std::pair<std::string, std::function<void()>>> o;
  return o;
}
inline void can_open(const std::string &key, std::function<void()> fn) {
  for (auto &o : openers())
    if (o.first == key) {
      o.second = std::move(fn);
      return;
    }
  openers().push_back({key, std::move(fn)});
}
inline bool show(const std::string &key) {
  for (auto &o : openers())
    if (o.first == key) {
      o.second();
      return true;
    }
  return false;
}

inline void button(Display &it, Rect r, const char *label, bool primary, std::function<void()> fn) {
  if (primary) {
    it.filled_rectangle(r.x, r.y, r.w, r.h, BLACK);
    it.print(r.x + r.w / 2, r.y + r.h / 2, f_h1, WHITE, TextAlign::CENTER, label, BLACK);
  } else {
    for (int b = 0; b < 4; b++) it.rectangle(r.x + b, r.y + b, r.w - 2 * b, r.h - 2 * b, BLACK);
    txt(it, r.x + r.w / 2, r.y + r.h / 2, f_h1, BLACK, TextAlign::CENTER, label);
  }
  zone(r, std::move(fn));
}

// Largest of the fonts (biggest first) that fits s in width w.
inline font::Font *fit(Display &it, const std::string &s, int w, std::initializer_list<font::Font *> fonts) {
  font::Font *last = nullptr;
  for (auto *f : fonts) {
    int x1, y1, tw, th;
    it.get_text_bounds(0, 0, s.c_str(), f, TextAlign::TOP_LEFT, &x1, &y1, &tw, &th);
    last = f;
    if (tw <= w) break;
  }
  return last;
}

// Confirmation inside r: question, optional warning, Cancel / Confirm.
inline void confirm(Display &it, Rect r, const char *question, const char *warning, std::function<void()> cancel,
                    std::function<void()> ok) {
  const Strings &t = tr();
  const int cx = r.x + r.w / 2, bw = std::min(640, (r.w - 40) / 2);
  txt(it, cx, r.y + 290, fit(it, question, r.w, {f_h1, f_h2}), BLACK, TextAlign::TOP_CENTER, question);
  if (warning) txt(it, cx, r.y + 400, fit(it, warning, r.w, {f_txt, f_small}), GRAY, TextAlign::TOP_CENTER, warning);
  button(it, {(int16_t) (cx - 20 - bw), (int16_t) (r.y + 710), (int16_t) bw, 240}, t.cancel, false, std::move(cancel));
  button(it, {(int16_t) (cx + 20), (int16_t) (r.y + 710), (int16_t) bw, 240}, t.confirm, true, std::move(ok));
}

// Panels attached to a widget's sub-page, drawn next to it (e.g. a doorbell next to the front door lock).
inline std::vector<std::pair<std::string, Draw>> &attached() {
  static std::vector<std::pair<std::string, Draw>> a;
  return a;
}
inline void attach(const std::string &key, Draw panel) { attached().push_back({key, std::move(panel)}); }
inline std::vector<Draw> panels(const std::string &key) {
  std::vector<Draw> out;
  for (auto &a : attached())
    if (a.first == key) out.push_back(a.second);
  return out;
}

// Badge shown on another widget's tile: icon in the top-right corner, text instead of its subtitle
// (e.g. the doorbell's Ring to Open on the front door tile). Empty text = nothing to show.
struct Badge {
  const char *icon;
  std::string text;
};
inline std::vector<std::pair<std::string, std::function<Badge()>>> &badges() {
  static std::vector<std::pair<std::string, std::function<Badge()>>> b;
  return b;
}
inline void attach_badge(const std::string &key, std::function<Badge()> fn) { badges().push_back({key, std::move(fn)}); }
inline Badge badge(const std::string &key) {
  for (auto &b : badges())
    if (b.first == key) {
      Badge v = b.second();
      if (!v.text.empty()) return v;
    }
  return {nullptr, ""};
}

// Sub-page header: Back button, title, rule.
inline void page_header(Display &it, const char *label) {
  button(it, {48, 30, 300, 100}, tr().back, false, ui::close);
  txt(it, 1824, 50, f_h1, BLACK, TextAlign::TOP_RIGHT, label);
  it.filled_rectangle(48, 160, 1776, 4, BLACK);
}

// Call first in the display lambda. Draws the Wi-Fi setup screen or the open sub-page and returns true
// (the lambda must then stop), otherwise clears the screen for the dashboard.
inline bool begin(Display &it) {
  zones().clear();
  it.fill(WHITE);
#ifdef USE_WIFI
  if (!wifi_comp->is_connected()) {
    const Strings &t = tr();
    txt(it, 936, 560, f_h1, BLACK, TextAlign::TOP_CENTER, t.wifi_title);
    txt(it, 936, 680, f_txt, BLACK, TextAlign::TOP_CENTER, t.wifi_join_before + ap_ssid->value() + t.wifi_join_after);
    txt(it, 936, 740, f_txt, GRAY, TextAlign::TOP_CENTER, t.wifi_portal);
    return true;
  }
#endif
  if (!on_dashboard()) {
    page().draw(it);
    return true;
  }
  return false;
}

}  // namespace ui
