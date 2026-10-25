# Roadmap

Objectif : passer d'une config perso à un repo public réutilisable (`vcanuel/reterminal-e1003-ha`),
**sans** viser un firmware binaire unique (un binaire ne connaît pas les entités HA de chacun, et la config depuis HA
est déjà le terrain d'ESPHomeDesigner).

## 1. Personnalisation : widgets + config perso — piste A faite (2026-10-25)

Choix retenu : **A. Paquets + fonctions C++ par widget** (voir README, « Organisation ») :
`hardware.yaml` générique, `ui.h` (helpers, zones tactiles, sous-pages), `widgets/<nom>.yaml` + `widgets/<nom>.h`,
`private/vincent.yaml` (écran perso, non versionné, identique à l'ancien `reterminal-e1003.yaml`) et `example.yaml` (générique).
Un widget = ses capteurs + son dessin + ses zones tactiles / sous-page (ex. serrure).

Écartées pour l'instant :
- **B. Composant externe `dashboard`** (codegen Python, mise en page déclarée en YAML) : à reconsidérer si le repo
  public intéresse du monde.
- **C. Config à chaud depuis HA** : hors périmètre (voir ESPHomeDesigner, Love4yzp/esphome-reterminal-e1003-workspace).

Reste :
- [ ] Widgets à fonts dédiées (météo, serrure) : polices dupliquées si plusieurs instances (taille flash).
- [x] Fichiers d'écran hors racine (`private/`) : chemins des paquets relatifs à chaque paquet.

## 2. Nettoyage avant publication

- [x] Remplacer les `entity_id` perso par des `vars` (fait via les widgets ; données perso isolées dans `vincent.yaml`).
- [x] Configs perso dans `private/` (ignoré par git).
- [x] Retirer adresses mail des calendriers, prénoms, noms de pièces (ne restent que dans `private/`).
- [ ] Vérifier qu'aucun secret n'est versionné (`secrets.yaml` ignoré, `secrets.example.yaml` fourni).
- [ ] Police MDI : la télécharger au build (`file: {type: web, url: ...}`) plutôt que versionner le .ttf (1,2 Mo).
- [ ] Licence (MIT), README FR + EN.
- [x] Captures du README générées par `simulator.yaml` (rendu exact, données de démo) plutôt qu'une photo.

## 3. Fonctionnalités à venir

- [ ] Sous-page chauffage par pièce (consigne +/− 0,5 °C).
- [ ] Page « souvenirs » photo (source à définir : Synology Photos, Immich…), via `online_image`.
- [ ] Nettoyage anti-rémanence (mode INIT) une fois par heure.
- [ ] Agenda : filtrer les jours d'observance (« Heure d'hiver », etc.).
- [ ] Contribuer en amont (wiki Seeed / ESPHome) la détection secteur via SY6974B si utile.
