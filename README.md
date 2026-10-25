# reterminal-e1003-ha

Tableau de bord Home Assistant pour le **Seeed reTerminal E1003** (ePaper 10,3", 1872×1404, 16 niveaux de gris,
tactile GT911), en **ESPHome pur** : rien à configurer côté Home Assistant à part autoriser les actions.

> État : config perso fonctionnelle, découpée en socle matériel + widgets réutilisables (voir `ROADMAP.md`).

![Tableau de bord (example.yaml, rendu par le simulateur)](docs/example.png)

## Ce que ça fait

- Météo (actuelle + 5 jours) et agenda récupérés via les actions HA `weather.get_forecasts` / `calendar.get_events`
  avec `response_template` : pas de capteur modèle à créer dans HA.
- Pièces (climate.*), électricité, serrure, voiture, présence via des capteurs `homeassistant`.
- Tactile : sous-page serrure (verrouiller / déverrouiller / ouvrir, avec confirmation), bip de retour,
  retour auto au tableau de bord après 2 min. Toucher ailleurs rafraîchit l'écran.
- Énergie : détection secteur/batterie par le chargeur **SY6974B** (I2C 0x6B, REG08 bit 2), secours par saut de tension.
  Sur secteur : rafraîchissement toutes les 5 min + tactile. Sur batterie : un rafraîchissement puis deep sleep 30 min
  (réveil par le bouton vert).

## Organisation

```
hardware.yaml        socle matériel (écran, tactile, buzzer, LED, batterie, chargeur, veille, Wi-Fi/API/OTA)
fonts.yaml           polices communes (Inter + icônes Material Design)
ui.h                 helpers de dessin, zones tactiles, sous-pages
widgets/<nom>.yaml   capteurs HA du widget, entités passées en vars
widgets/<nom>.h      dessin du widget dans un rectangle (+ zones tactiles / sous-page)
example.yaml         écran d'exemple, à copier dans private/ (ignoré par git) pour son écran perso
simulator.yaml       remplace hardware.yaml sur ordinateur pour générer les captures (docs/)
```

| Widget | Paquet (vars) | Dessin |
|---|---|---|
| En-tête (date, MAJ, alimentation) | `widgets/header.yaml` | `ui::header(it, {x, y, w, h})` |
| Météo actuelle + 5 jours | `widgets/weather.yaml` (`id`, `entity`) | `id(meteo)(it, {x, y, 980, 620})` |
| Agenda | `widgets/agenda.yaml` (`id`, `calendars`, `title`, `duration`, `max_events`, `max_chars`) | `id(agenda)(it, {x, y, w, h})` |
| Pièce climate.* | `widgets/room.yaml` (`id`, `entity`, `name`), une par pièce | `ui::rooms(it, r, "Pièces", {id(salon), ...})` |
| Électricité | `widgets/power.yaml` (`id`, `entity`, `title`, `subtitle`) | `id(elec)(it, {x, y, 370, 220})` |
| Serrure + sous-page | `widgets/lock.yaml` (`id`, `entity`, `name`, `subtitle`) | `id(porte)(it, {x, y, 370, 220})` |
| Voiture électrique | `widgets/car.yaml` (`id`, `name`, `battery`, `range`, `charger_power`, `plugged`) | `id(voiture)(it, {x, y, 370, 220})` |
| Présence person.* | `widgets/person.yaml` (`id`, `entity`, `name`), une par personne | `ui::presence(it, r, "Présence", {id(alice), ...})` |

Chaque paquet expose une globale `id(<id>)` qui dessine le widget dans le rectangle donné ; le fichier d'écran
liste ses paquets et place les widgets dans un lambda court (voir `example.yaml`) :

```yaml
packages:
  hardware: !include hardware.yaml
  meteo: !include { file: widgets/weather.yaml, vars: { id: meteo, entity: weather.home } }
display:
  - id: !extend epaper_display
    lambda: |-
      if (ui::begin(it)) return;   // écran Wi-Fi ou sous-page
      id(meteo)(it, {40, 160, 978, 620});
```

Écrire un widget : un `widgets/<nom>.h` avec une fonction `ui::<nom>(Display &it, Rect r, ...)` (helpers `ui::txt`,
`ui::tile`, `ui::title`, `ui::zone` pour une zone tactile, `ui::open` pour une sous-page), et un `widgets/<nom>.yaml`
qui déclare ses capteurs et, s'il appelle une action HA au rafraîchissement, l'ajoute au script commun :
`script: [{ id: !extend fetch_data, then: [...] }]`. Icônes Material Design : la police `i_s` de `hardware.yaml`,
à compléter avec `font: [{ id: !extend i_s, glyphs: [...] }]`.

## Utilisation

```bash
uv venv .venv && uv pip install --python .venv esphome      # ESPHome >= 2026.7 (driver it8951)
cp secrets.example.yaml secrets.yaml                          # api_key, ota_password, ap_password
mkdir -p private && cp example.yaml private/mon-ecran.yaml      # private/ est ignoré par git
# dans private/mon-ecran.yaml : préfixer les chemins par ../ (hardware.yaml, widgets/...) et y mettre ses entités HA
.venv/bin/esphome run private/mon-ecran.yaml --device /dev/cu.wchusbserial*   # 1er flash en USB (CH340)
.venv/bin/esphome run private/mon-ecran.yaml --device <ip>                    # ensuite en OTA
```

1. Au premier démarrage l'écran ouvre le point d'accès `reTerminal-E1003` : y choisir le Wi-Fi de la maison.
2. HA découvre l'appareil (intégration ESPHome).
3. **Options de l'appareil ESPHome > « Autoriser l'appareil à effectuer des actions Home Assistant »** (sinon météo et
   agenda restent vides, et la serrure ne répond pas).

## Captures d'écran (simulateur)

`simulator.yaml` remplace `hardware.yaml` par la plateforme `host` d'ESPHome et un écran en mémoire : il dessine
`example.yaml` avec des données de démo, enregistre le tableau de bord et la sous-page serrure, puis s'arrête.
Pas besoin de l'écran ni de Home Assistant.

```bash
ESPHOME_SNAPSHOT_DIR=$PWD/docs .venv/bin/esphome -s name reterminal-sim -s hardware simulator.yaml run example.yaml --no-logs
for f in example example-lock; do sips -s format png docs/$f.bmp --out docs/$f.png && rm docs/$f.bmp; done   # macOS
```

<img src="docs/example-lock.png" width="400" alt="Sous-page serrure">

## Limites connues

- Brancher l'USB ne réveille pas l'écran (l'interruption du chargeur n'arrive sur aucun GPIO) : bouton vert ou réveil
  suivant (≤ 30 min). Pour un OTA sur batterie : brancher l'USB puis appuyer sur le bouton vert.
- Le tactile ne fonctionne que réveillé, donc en pratique sur secteur.
