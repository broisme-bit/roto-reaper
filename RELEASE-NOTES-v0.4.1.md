# v0.4.1 — Ableton AU/VST2 mappings and plugin value displays

Recall existing ordinary Ableton AU and VST2 mappings stored on Roto-Control by choosing Ableton as the plugin mapping source. VST3 support continues from v0.4.0. Mapped knobs and buttons now show REAPER-formatted parameter values; touching a knob redisplays its values, and mouse changes update the display.

## Upgrade

REAPER-source mapping identities and existing Ableton VST3 identities are unchanged from v0.4.0. No relearning is required because of this update. Switching an AU or VST2 plugin to the Ableton source requests its Ableton mapping instead of its REAPER mapping; switching back restores the REAPER source. Stored setups are not converted or deleted.

## Validation and limitations

The assembled v0.4.1 candidate passed the Apple Silicon Mac build and automated suites, REAPER-source Pultec and Pro-Q checks, Ableton-source Pultec VST3/AU, Minimoog VST3i/AUi and Devil-Loc Deluxe VST2 checks, formatted display feedback, MIX, sends, transport, unplug/replug reconnect and restart persistence.

Logic compatibility, compatibility profiles, and experimental REAPER identity changes are excluded. Ableton name differences can still prevent recall (including the known VST2 Twin 3 mismatch). Racks/macros, AUv3, VST2 instrument recall and reverse REAPER-to-Live recall are unvalidated. Intel Mac and Windows hardware remain unvalidated. Existing touch-timeout and automation limitations remain; see README.md.
