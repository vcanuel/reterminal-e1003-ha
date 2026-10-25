// Présence : une ligne par personne (person.*), « maison », « absent·e » ou nom de la zone.
#pragma once
#include "ui.h"

namespace ui {

inline std::string presence_where(const std::string &s) {
  if (s == "home") return "maison";
  if (s == "not_home") return "absent·e";
  if (s.empty() || s == "unknown" || s == "unavailable") return "--";
  return s;
}

// Une ligne de 65 px de haut.
inline void person_row(Display &it, Rect r, const char *name, text_sensor::TextSensor *state) {
  txt(it, r.x, r.y, f_txt_b, BLACK, TextAlign::TOP_LEFT, name);
  txt(it, r.x2(), r.y, f_txt, GRAY, TextAlign::TOP_RIGHT, presence_where(state->state));
}

// Titre de tuile + liste des personnes (chacune = la globale exposée par widgets/person.yaml).
// Zone conseillée : 370 x (75 + 65 x nombre de personnes).
inline void presence(Display &it, Rect r, const char *label, std::initializer_list<Draw> people) {
  txt(it, r.x, r.y, i_s, BLACK, TextAlign::TOP_LEFT, "\U000F02DC");
  txt(it, r.x + 70, r.y + 12, f_small, GRAY, TextAlign::TOP_LEFT, label);
  int i = 0;
  for (auto &p : people) {
    p(it, {r.x, (int16_t) (r.y + 75 + i * 65), r.w, 65});
    i++;
  }
}

}  // namespace ui
