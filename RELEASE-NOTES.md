# v0.3.0 — experimental multi-platform beta

Adds MIX all-tracks send levels using the published Bitwig protocol. Select an existing destination bus; each knob controls its visible track's existing send to that bus. Includes destination names, 14-bit motor feedback and banking. Missing routes stay inactive and no sends are created. Hardware outputs are excluded; duplicate routes to one destination use the first send.

Retains the v0.2.7 fix that saves the control-surface entry across REAPER restarts. Existing learned plugin mappings are unaffected.

Mock tests and Apple Silicon basic send-mode checks pass, including banking, route removal and mixer/plugin/restart regressions. Windows x64 and Intel Mac builds are experimental and require volunteer hardware testing after successful CI builds. Existing touch dropouts remain unresolved; send automation Touch holds are not validated. Windows/Intel Mac and single-track MIX remain unvalidated/unsupported respectively. See README.md and SEND-TEST.md.
