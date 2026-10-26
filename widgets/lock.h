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

// Lock commands inside r: full width (row of buttons) or half page next to attached panels (stacked buttons).
inline void lock_panel(Display &it, Rect r, const Lock &l) {
  const Strings &t = tr();
  const std::string ls = l.state->state;
  const int cx = r.x + r.w / 2;
  const bool wide = r.w > 1200;
  if (l.step == 2) {
    const int a = l.pending;
    confirm(it, r, a == 1 ? t.ask_lock : a == 2 ? t.ask_unlock : t.ask_open, a == 3 ? t.open_warning : nullptr,
            [l] { l.step = 1; },
            [l] {
              l.send(l.pending == 1 ? "lock.lock" : l.pending == 2 ? "lock.unlock" : "lock.open");
              l.msg = tr().sent;
              l.step = 1;
            });
    return;
  }
  const char *label = ls == "locked" ? t.locked : ls == "unlocked" ? t.unlocked
                    : ls == "open" ? t.open : ls == "opening" ? t.opening
                    : ls == "locking" ? t.locking : ls == "unlocking" ? t.unlocking
                    : ls == "jammed" ? t.jammed : "--";
  if (!wide) txt(it, r.x, r.y, f_h2, BLACK, TextAlign::TOP_LEFT, t.lock_title);
  txt(it, cx, r.y + (wide ? 30 : 60), l.icon, BLACK, TextAlign::TOP_CENTER, lock_icon(ls));
  txt(it, cx, r.y + 370, fit(it, label, r.w, {l.big, f_h1}), BLACK, TextAlign::TOP_CENTER, label);
  if (!l.msg.empty()) txt(it, cx, r.y + (wide ? 540 : 480), f_txt, GRAY, TextAlign::TOP_CENTER, l.msg);
  auto ask = [l](int a) { return [l, a] { l.pending = a; l.step = 2; }; };
  const char *labels[3] = {t.do_lock, t.do_unlock, t.do_open};
  for (int i = 0; i < 3; i++) {
    const Rect b = wide ? Rect{(int16_t) (r.x + 48 + i * 580), (int16_t) (r.y + 810), 520, 220}
                        : Rect{r.x, (int16_t) (r.y + 560 + i * 150), r.w, 130};
    button(it, b, labels[i], false, ask(i + 1));
  }
  if (wide) txt(it, r.x + 1468, r.y + 1050, f_small, GRAY, TextAlign::TOP_CENTER, t.open_hint);
  else txt(it, cx, r.y + 1000, f_small, GRAY, TextAlign::TOP_CENTER, t.open_hint);
}

// Sub-page: the lock, and the panels attached to it (e.g. widgets/opener.yaml with `page`) on the right.
inline void lock_page(Display &it, const Lock &l) {
  page_header(it, l.name);
  auto extra = panels(l.key);
  if (extra.empty()) {
    lock_panel(it, {48, 190, 1776, 1190}, l);
    return;
  }
  lock_panel(it, {48, 190, 840, 1190}, l);
  it.filled_rectangle(935, 200, 3, 1150, LIGHT);
  extra[0](it, {984, 190, 840, 1190});
}

// Dashboard tile (suggested area: 370 x 220); touching it opens the sub-page.
inline void lock_tile(Display &it, Rect r, const Lock &l) {
  const Strings &t = tr();
  const std::string s = l.state->state;
  const Badge b = badge(l.key);  // e.g. the doorbell attached to this lock
  tile(it, r, lock_icon(s), l.name,
       s == "locked" ? t.locked : s == "unlocked" ? t.tile_unlocked : s == "jammed" ? t.jammed : "--",
       b.text.empty() ? l.sub : b.text);
  if (b.icon) txt(it, r.x + r.w, r.y, i_s, BLACK, TextAlign::TOP_RIGHT, b.icon);
  auto show_page = [l] {
    l.step = 1;
    l.msg = "";
    open(l.key, [l](Display &it) { lock_page(it, l); });
  };
  can_open(l.key, show_page);
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)}, show_page);
}

}  // namespace ui
