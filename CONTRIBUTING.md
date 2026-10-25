# Contribuer

Merci de votre intérêt ! Issues et pull requests sont bienvenues, en français ou en anglais
(*issues and pull requests are welcome, in French or English*).

## Signaler un problème

Indiquer la version d'ESPHome (`esphome version`), le fichier d'écran utilisé (sans `secrets.yaml` ni entités
privées si vous préférez), les logs (`esphome logs ...`) et si possible une photo de l'écran.

## Proposer un widget

Un widget = deux fichiers dans `widgets/` (voir le README, « Organisation ») :

- `widgets/<nom>.yaml` : capteurs HA du widget, entités passées en `vars` (avec `defaults:` pour les options),
  une globale `${id}` de type `std::function<void(display::Display &, display::Rect)>` qui dessine le widget,
  et si besoin un `script: - id: !extend fetch_data` pour les actions HA appelées à chaque rafraîchissement.
  Chemins relatifs au paquet (`<nom>.h`), pour fonctionner depuis n'importe quel dossier.
- `widgets/<nom>.h` : fonctions dans le namespace `ui` (pas de namespace propre : il entrerait en collision avec
  les ids ESPHome). Dessiner relativement au `Rect` reçu, déclarer les zones tactiles avec `ui::zone` et les
  sous-pages avec `ui::open`.

Rester sobre : écran ePaper 16 niveaux de gris, police Inter et icônes Material Design de `fonts.yaml`.

## Avant d'ouvrir une pull request

1. `example.yaml` compile : `.venv/bin/esphome compile example.yaml` (ajouter le widget à l'exemple s'il est nouveau,
   avec des entités fictives).
2. Le simulateur tourne et les captures sont à jour (commande dans le README, « Captures d'écran ») ; ajouter des
   données de démo pour le widget dans `simulator.yaml`.
3. Si possible, testé sur un vrai reTerminal E1003 (le dire dans la PR, avec une photo).
4. README mis à jour (tableau des widgets).

La CI (`.github/workflows/build.yml`) refait les étapes 1 et 2 sur chaque PR : firmware et captures disponibles
en artefacts du run.

Pas de données personnelles dans les commits : les configs perso vont dans `private/` (ignoré par git).

En contribuant, vous acceptez que votre contribution soit publiée sous licence MIT (voir `LICENSE`).
