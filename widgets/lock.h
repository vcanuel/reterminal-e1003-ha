// Serrure (lock.*) : tuile d'état + sous-page tactile verrouiller / déverrouiller / ouvrir, avec confirmation.
#pragma once
#include "ui.h"

namespace ui {

struct Lock {
  const char *key;  // id du widget (clé de sous-page)
  const char *name, *sub;
  text_sensor::TextSensor *state;
  int &step;         // sous-page : 1 = commandes, 2 = confirmation
  int &pending;      // action à confirmer : 1 = verrouiller, 2 = déverrouiller, 3 = ouvrir
  std::string &msg;  // retour de commande affiché sous l'état
  void (*send)(const std::string &action);
  font::Font *icon, *big;
};

inline const char *lock_icon(const std::string &s) { return s == "locked" ? "\U000F033E" : "\U000F0FC6"; }

inline void lock_page(Display &it, const Lock &l) {
  const std::string ls = l.state->state;
  page_header(it, l.name);
  if (l.step == 1) {
    const char *label = ls == "locked" ? "Verrouillée" : ls == "unlocked" ? "Déverrouillée"
                      : ls == "open" ? "Ouverte" : ls == "opening" ? "Ouverture..."
                      : ls == "locking" ? "Verrouillage..." : ls == "unlocking" ? "Déverrouillage..."
                      : ls == "jammed" ? "Bloquée" : "--";
    txt(it, 936, 220, l.icon, BLACK, TextAlign::TOP_CENTER, lock_icon(ls));
    txt(it, 936, 560, l.big, BLACK, TextAlign::TOP_CENTER, label);
    if (!l.msg.empty()) txt(it, 936, 730, f_txt, GRAY, TextAlign::TOP_CENTER, l.msg);
    auto ask = [l](int a) { return [l, a] { l.pending = a; l.step = 2; }; };
    button(it, {96, 1000, 520, 220}, "Verrouiller", false, ask(1));
    button(it, {676, 1000, 520, 220}, "Déverrouiller", false, ask(2));
    button(it, {1256, 1000, 520, 220}, "Ouvrir", false, ask(3));
    txt(it, 1516, 1240, f_small, GRAY, TextAlign::TOP_CENTER, "tire le pêne");
  } else {
    const int a = l.pending;
    txt(it, 936, 480, f_h1, BLACK, TextAlign::TOP_CENTER,
        a == 1 ? "Verrouiller la porte ?" : a == 2 ? "Déverrouiller la porte ?" : "Ouvrir la porte ?");
    if (a == 3)
      txt(it, 936, 590, f_txt, GRAY, TextAlign::TOP_CENTER, "Le pêne sera tiré : la porte s'ouvrira en poussant.");
    button(it, {196, 900, 640, 240}, "Annuler", false, [l] { l.step = 1; });
    button(it, {1036, 900, 640, 240}, "Confirmer", true, [l] {
      l.send(l.pending == 1 ? "lock.lock" : l.pending == 2 ? "lock.unlock" : "lock.open");
      l.msg = "Commande envoyée...";
      l.step = 1;
    });
  }
}

// Tuile du tableau de bord (zone conseillée : 370 x 220) ; la toucher ouvre la sous-page.
inline void lock_tile(Display &it, Rect r, const Lock &l) {
  const std::string s = l.state->state;
  tile(it, r, lock_icon(s), l.name,
           s == "locked" ? "Verrouillée" : s == "unlocked" ? "Ouverte" : s == "jammed" ? "Bloquée" : "--", l.sub);
  zone({(int16_t) (r.x - 14), (int16_t) (r.y - 15), (int16_t) (r.w + 30), (int16_t) (r.h + 30)}, [l] {
    l.step = 1;
    l.msg = "";
    open(l.key, [l](Display &it) { lock_page(it, l); });
  });
}

}  // namespace ui
