# v0.4.0 — Ableton VST3 mapping compatibility

Choose REAPER or Ableton as the VST3 mapping source in Preferences > Control/OSC/web > Edit Roto-Control. The setting persists across REAPER restarts and controller reconnects. Ableton mode recalls ordinary VST3 mappings already stored on Roto-Control.

REAPER VST3 mappings now identify plugins by their class ID rather than installation path or display name.

## Upgrade note

Existing REAPER VST3 mappings for recognized class IDs must be learned again. Previous setups remain stored on the controller; they are not migrated or deleted. AU, VST2 and CLAP retain their previous REAPER identities.

## Validation and scope

Apple Silicon hardware checks passed with UADx Pultec and FabFilter Pro-Q: control in both directions, recall across instances/restarts, saved source selection and switching between REAPER/Ableton mappings. Automated protocol and surface tests pass.

Ableton compatibility covers ordinary VST3 plugins. AU/VST2, racks/macros, Bitwig and Logic mapping compatibility are not included. Host name/parameter-label differences can prevent recall for other plugins. Windows and Intel Mac packages remain experimental until hardware-tested. Existing touch automation and send automation limitations remain; see README.md.
