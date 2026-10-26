# Roadmap

Goal: a reusable public repo (`vcanuel/reterminal-e1003-ha`), **not** a single binary firmware (a binary does not
know everyone's HA entities, and configuring from HA is already ESPHomeDesigner's territory).

## 1. Customization: widgets + personal screen file — done (2026-10-25)

Chosen approach: **packages + C++ functions per widget** (see the README, "Layout of the repo"):
generic `hardware.yaml`, `ui.h` (helpers, touch zones, sub-pages), `widgets/<name>.yaml` + `widgets/<name>.h`,
`example.yaml` (generic), personal screen files in `private/` (not versioned).
A widget = its sensors + its drawing + its touch zones / sub-page (e.g. lock).

Set aside for now:
- **External `dashboard` component** (Python codegen, layout declared in YAML): to reconsider if the public repo
  draws interest.
- **Live configuration from HA**: out of scope (see ESPHomeDesigner, Love4yzp/esphome-reterminal-e1003-workspace).

Left:
- [ ] Widgets with dedicated fonts (weather, lock): fonts duplicated with several instances (flash size).

## 2. Clean-up before publishing

- [x] Personal `entity_id`s replaced by `vars`; personal screen files in `private/` (ignored by git).
- [x] No secret versioned (`secrets.yaml` ignored, `secrets.example.yaml` provided).
- [x] MDI font downloaded at build time (v7.4.47) instead of versioned.
- [x] MIT license, CONTRIBUTING, CI build (example + simulator).
- [x] README, code and on-screen texts in English; French via `language: fr` (`ui_strings.h`).
- [x] README screenshots rendered by `simulator.yaml` (exact rendering, demo data) instead of a photo.
- [ ] Make the GitHub repo public.

## 3. Upcoming features

- [x] Weather sub-page (touch): current details, next 12 hours, next 7 days (2026-10-26).
- [x] Heating sub-page per room: target −/+ 0.5 °C, hvac mode (2026-10-26).
- [ ] Photo "memories" page (source to define: Synology Photos, Immich…), via `online_image`.
- [ ] Anti-ghosting clean-up (INIT mode) once an hour.
- [ ] Calendar: filter observance days ("Daylight saving time ends", etc.).
- [ ] Upstream (Seeed wiki / ESPHome) the mains detection through the SY6974B, if useful.
