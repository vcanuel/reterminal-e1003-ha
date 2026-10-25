// Briques d'affichage communes aux widgets : couleurs, texte, zones tactiles, sous-pages.
// Les widgets (widgets/*.h) dessinent dans un rectangle et déclarent leurs zones tactiles pendant le dessin :
// les zones correspondent donc toujours à ce qui est affiché.
#pragma once
#include <cmath>
#include <cstring>
#include <functional>
#include <string>
#include <vector>

namespace ui {

using display::Display;
using display::Rect;
using display::TextAlign;
using Draw = std::function<void(Display &, Rect)>;  // type des globales exposées par les widgets

static const Color BLACK(0, 0, 0, 255), WHITE(255, 255, 255, 255);
static const Color GRAY(100, 100, 100, 255), LIGHT(190, 190, 190, 255);

inline void txt(Display &it, int x, int y, font::Font *f, Color c, TextAlign a, const std::string &s) {
  it.print(x, y, f, c, a, s.c_str(), WHITE);
}

// "12.3" ou "--" si la valeur n'est pas disponible.
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

static const char *const JOURS[] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};
static const char *const JOURS_COURTS[] = {"Lun", "Mar", "Mer", "Jeu", "Ven", "Sam", "Dim"};  // weekday() Python
static const char *const MOIS[] = {"janvier", "février", "mars",      "avril",   "mai",      "juin",
                                   "juillet", "août",    "septembre", "octobre", "novembre", "décembre"};

// Titre de section : icône (police i_s) + libellé.
inline void title(Display &it, int x, int y, const char *icon, const char *label) {
  txt(it, x, y, i_s, BLACK, TextAlign::TOP_LEFT, icon);
  txt(it, x + 70, y + 2, f_h2, BLACK, TextAlign::TOP_LEFT, label);
}

// Tuile : icône + libellé, grande valeur, sous-titre.
inline void tile(Display &it, Rect r, const char *icon, const char *label, const std::string &value,
                 const std::string &sub) {
  txt(it, r.x, r.y, i_s, BLACK, TextAlign::TOP_LEFT, icon);
  txt(it, r.x + 70, r.y + 12, f_small, GRAY, TextAlign::TOP_LEFT, label);
  txt(it, r.x, r.y + 72, f_h1, BLACK, TextAlign::TOP_LEFT, value);
  if (!sub.empty()) txt(it, r.x, r.y + 150, f_small, GRAY, TextAlign::TOP_LEFT, sub);
}

// ---------- Zones tactiles ----------
struct Zone {
  Rect r;
  std::function<void()> fn;
};
inline std::vector<Zone> &zones() {
  static std::vector<Zone> z;
  return z;
}
inline void zone(Rect r, std::function<void()> fn) { zones().push_back({r, std::move(fn)}); }

// Appelle la zone touchée ; false si le toucher est hors de toute zone.
inline bool touch(int x, int y) {
  for (auto &z : zones())
    if (z.r.inside(x, y)) {
      z.fn();
      return true;
    }
  return false;
}

// ---------- Sous-pages (plein écran, retour auto au tableau de bord après 2 min) ----------
struct Page {
  std::string key;  // id du widget propriétaire, vide = tableau de bord
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

// En-tête de sous-page : bouton Retour, titre, filet.
inline void page_header(Display &it, const char *label) {
  button(it, {48, 30, 300, 100}, "Retour", false, ui::close);
  txt(it, 1824, 50, f_h1, BLACK, TextAlign::TOP_RIGHT, label);
  it.filled_rectangle(48, 160, 1776, 4, BLACK);
}

// À appeler en tête du lambda d'affichage. Dessine l'écran de configuration Wi-Fi ou la sous-page ouverte
// et renvoie true dans ce cas (le lambda doit alors s'arrêter), sinon efface l'écran pour le tableau de bord.
inline bool begin(Display &it) {
  zones().clear();
  it.fill(WHITE);
#ifdef USE_WIFI
  if (!wifi_comp->is_connected()) {
    txt(it, 936, 560, f_h1, BLACK, TextAlign::TOP_CENTER, "Configuration Wi-Fi");
    txt(it, 936, 680, f_txt, BLACK, TextAlign::TOP_CENTER, "Rejoindre le réseau « " + ap_ssid->value() + " »");
    txt(it, 936, 740, f_txt, GRAY, TextAlign::TOP_CENTER, "puis choisir le Wi-Fi de la maison dans le portail");
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
