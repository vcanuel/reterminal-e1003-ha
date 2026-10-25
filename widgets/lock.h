// Lock (lock.*): status tile + touch sub-page lock / unlock / open, with confirmation.
#pragma once
#include "ui.h"

namespace ui {

struct Lock {
  const char *key;  // widget id (sub-page key)
  const char *name, *sub;
  text_sensor::TextSensor *state;
  int &step;         // sub-page: 1 = commands, 2 = confirmation
  int &pending;      // action to confirm: 1 = lock, 2 = unlock, 3 = open
  std::string &msg;  // command feedback shown under the state
  void (*send)(const std::string &action);
  font::Font *icon, *big;
};

inline const char *lock_icon(const std::string &s) { return s == "locked" ? "\U000F033E" : "\U000F0FC6"; }

inline void lock_page(Display &it, const Lock &l) {
  const Strings &t = tr();
  const std::string ls = l.state->state;
  page_header(it, l.name);
  if (l.step == 1) {
    const char *label = ls == "locked" ? t.locked : ls == "unlocked" ? t.unlocked
                      : ls == "open" ? t.open : ls == "opening" ? t.opening
                      : ls == "locking" ? t.locking : ls == "unlocking" ? t.unlocking
                      : ls == "jammed" ? t.jammed : "--";
    txt(it, 936, 220, l.icon, BLACK, TextAlign::TOP_CENTER, lock_icon(ls));
    txt(it, 936, 560, l.big, BLACK, TextAlign::TOP_CENTER, label);
    if (!l.msg.empty()) txt(it, 936, 730, f_txt, GRAY, TextAlign::TOP_CENTER, l.msg);
    auto ask = [l](int a) { return [l, a] { l.pending = a; l.step = 2; }; };
    button(it, {96, 1000, 520, 220}, t.do_lock, false, ask(1));
    button(it, {676, 1000, 520, 220}, t.do_unlock, false, ask(2));
    button(it, {1256, 1000, 520, 220}, t.do_open, false, ask(3));
    txt(it, 1516, 1240, f_small, GRAY, TextAlign::TOP_CENTER, t.open_hint);
  } else {
    const int a = l.pending;
    txt(it, 936, 480, f_h1, BLACK, TextAlign::TOP_CENTER, a == 1 ? t.ask_lock : a == 2 ? t.ask_unlock : t.ask_open);
    if (a == 3) txt(it, 936, 590, f_txt, GRAY, TextAlign::TOP_CENTER, t.open_warning);
    button(it, {196, 900, 640, 240}, t.cancel, false, [l] { l.step = 1; });
    button(it, {1036, 900, 640, 240}, t.confirm, true, [l] {
      l.send(l.pending == 1 ? "lock.lock" : l.pending == 2 ? "lock.unlock" : "lock.open");
      l.msg = tr().sent;
      l.step = 1;
    });
  }
}

// Dashboard tile (suggested area: 370 x 220); touching it opens the sub-page.
inline void lock_tile(Display &it, Rect r, const Lock &l) {
  const Strings &t = tr();
  const std::string s = l.state->state;
  tile(it, r, lock_icon(s), l.name,
       s == "locked" ? t.locked : s == "unlocked" ? t.tile_unlocked : s == "jammed" ? t.jammed : "--", l.sub);
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)}, [l] {
    l.step = 1;
    l.msg = "";
    open(l.key, [l](Display &it) { lock_page(it, l); });
  });
}

}  // namespace ui
