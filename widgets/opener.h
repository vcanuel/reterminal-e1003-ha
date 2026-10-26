// Doorbell with a Nuki Opener (lock.* entity): unlock = Ring to Open on, lock = off, open = buzz the door.
// A panel next to the front door lock (`page` var) and/or its own tile + sub-page.
#pragma once
#include <ctime>
#include "ui.h"

namespace ui {

struct Opener {
  const char *name;
  text_sensor::TextSensor *state;
  uint32_t &last_ring;  // time of the last ring (epoch, 0 = none)
  int &step;            // 1 = commands, 2 = confirmation
  int &pending;         // action to confirm: 1 = Ring to Open on, 3 = buzz
  std::string &msg;     // command feedback
  void (*send)(const std::string &action);
  font::Font *icon, *big;
};

// "14:32" today, "25/10 14:32" otherwise.
inline std::string ring_time(uint32_t ts) {
  time_t t = ts, now = ::time(nullptr);
  struct tm lt, ln;
  localtime_r(&t, &lt);
  localtime_r(&now, &ln);
  char buf[16];
  strftime(buf, sizeof(buf), lt.tm_yday == ln.tm_yday && lt.tm_year == ln.tm_year ? "%H:%M" : "%d/%m %H:%M", &lt);
  return buf;
}

// Badge for the tile of the widget it is attached to (instead of its subtitle): Ring to Open on (with a bell),
// rang in the last 10 min (ringing bell), Ring to Open off, or unavailable.
inline Badge opener_badge(text_sensor::TextSensor *state, uint32_t last_ring) {
  const Strings &t = tr();
  const std::string s = state->state;
  if (s == "unlocked") return {"\U000F009A", t.rto_on};
  if (last_ring && ::time(nullptr) - (time_t) last_ring < 600) return {"\U000F009E", t.rang_at + ring_time(last_ring)};
  if (s == "locked") return {nullptr, t.rto_off};
  return {nullptr, t.doorbell_unavailable};
}

inline void opener_panel(Display &it, Rect r, const Opener &o) {
  const Strings &t = tr();
  const std::string s = o.state->state;
  const bool rto = s == "unlocked", wide = r.w > 1200;
  const int cx = r.x + r.w / 2;
  if (o.step == 2) {
    confirm(it, r, o.pending == 1 ? t.ask_rto : t.ask_buzz, o.pending == 1 ? t.rto_warning : nullptr,
            [o] { o.step = 1; },
            [o] {
              o.send(o.pending == 1 ? "lock.unlock" : "lock.open");
              o.msg = tr().sent;
              o.step = 1;
            });
    return;
  }
  const bool ringing = o.last_ring && ::time(nullptr) - (time_t) o.last_ring < 120;
  if (!wide) txt(it, r.x, r.y, f_h2, BLACK, TextAlign::TOP_LEFT, o.name);
  txt(it, cx, r.y + (wide ? 30 : 60), o.icon, BLACK, TextAlign::TOP_CENTER, ringing ? "\U000F009E" : "\U000F009A");
  const char *label = s == "unlocked" ? t.rto_on : s == "locked" ? t.rto_off : "--";
  txt(it, cx, r.y + 370, fit(it, label, r.w, {o.big, f_h1}), BLACK, TextAlign::TOP_CENTER, label);
  if (o.last_ring) txt(it, cx, r.y + 470, f_txt, GRAY, TextAlign::TOP_CENTER, t.last_ring + ring_time(o.last_ring));
  if (!o.msg.empty()) txt(it, cx, r.y + (wide ? 540 : 515), f_txt, GRAY, TextAlign::TOP_CENTER, o.msg);
  auto rect = [&](int i) {
    return wide ? Rect{(int16_t) (r.x + 338 + i * 580), (int16_t) (r.y + 810), 520, 220}
                : Rect{r.x, (int16_t) (r.y + 560 + i * 150), r.w, 130};
  };
  button(it, rect(0), t.do_open, false, [o] { o.pending = 3; o.step = 2; });
  // Ring to Open: filled when on; turning it off needs no confirmation.
  button(it, rect(1), t.rto, rto, [o, rto] {
    if (rto) {
      o.send("lock.lock");
      o.msg = tr().sent;
    } else {
      o.pending = 1;
      o.step = 2;
    }
  });
}

// Dashboard tile (suggested area: 370 x 220); touching it opens its own sub-page.
inline void opener_tile(Display &it, Rect r, const char *key, const char *name, const char *sub,
                        text_sensor::TextSensor *state, Draw panel) {
  const std::string s = state->state;
  tile(it, r, "\U000F009A", name, s == "unlocked" ? tr().on : s == "locked" ? tr().off : "--", sub);
  auto show_page = [key, name, panel] {
    open(key, [name, panel](Display &it) {
      page_header(it, name);
      panel(it, {48, 190, 1776, 1190});
    });
  };
  can_open(key, show_page);
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)}, show_page);
}

}  // namespace ui
