# Contributing

Thanks for your interest! Issues and pull requests are welcome, in English or French.

## Reporting a problem

Include the ESPHome version (`esphome version`), the screen file you use (without `secrets.yaml` or private
entities if you prefer), the logs (`esphome logs ...`) and, if possible, a photo of the screen.

## Adding a widget

A widget = two files in `widgets/` (see the README, "Layout of the repo"):

- `widgets/<name>.yaml`: the widget's HA sensors, entities passed as `vars` (with `defaults:` for options),
  a global `${id}` of type `std::function<void(display::Display &, display::Rect)>` that draws the widget,
  and if needed a `script: - id: !extend fetch_data` for the HA actions called on each refresh.
  Paths relative to the package (`<name>.h`), so it works from any folder.
- `widgets/<name>.h`: functions in the `ui` namespace (no namespace of its own: it would clash with ESPHome ids).
  Draw relative to the given `Rect`, declare touch zones with `ui::zone` and sub-pages with `ui::open`.
  On-screen texts go in `ui_strings.h` (every language), read with `ui::tr()`.

Keep it sober: 16-gray-level ePaper, Inter font and Material Design icons from `fonts.yaml`.

## Before opening a pull request

1. `example.yaml` compiles: `.venv/bin/esphome compile example.yaml` (add a new widget to the example, with
   made-up entities).
2. The simulator runs and the screenshots are up to date (command in the README, "Screenshots"); add demo data
   for the widget in `simulator.yaml`.
3. If possible, tested on a real reTerminal E1003 (say so in the PR, with a photo).
4. README updated (widget table).

The CI (`.github/workflows/build.yml`) runs steps 1 and 2 on every PR: firmware and screenshots are available as
run artifacts.

No personal data in commits: personal screen files go in `private/` (ignored by git).

By contributing, you agree that your contribution is released under the MIT license (see `LICENSE`).
