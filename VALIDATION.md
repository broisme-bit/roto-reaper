# Ableton AU/VST2 development validation — 2026-10-16 (user local date)

Apple Silicon user hardware checks passed:
- AU UADx Pultec: existing Ableton mapping recall, all three controls in both directions, restart recall.
- AUi UADx Moog Minimoog: existing Ableton mapping recall, controls in both directions, restart recall.
- VST2 Soundtoys Devil-Loc Deluxe: Crush, Crunch and Darkness in both directions, restart recall.
- VST3 Pultec and VST3i Moog regression checks passed after AU support was added.

VST3i Moog also passed with released v0.4.0. The reporter's failing VST3 instruments
remain unresolved. VST2 Twin 3 naming differs between hosts; it has not passed.
AUv3, successful VST2 instrument recall, reverse REAPER-to-Live recall and
Windows/Intel hardware compatibility remain unvalidated.

Automated protocol checks match captured AU Pultec, VST3i Moog and VST2 Devil-Loc
identifiers and parameter hashes. Surface mock suites pass for REAPER, Ableton
VST3, AU and VST2. Linux module compiles/links; Mac script syntax checks pass.
User ran the Mac build script and tested the resulting development binaries.
No release has been created for these changes.

---

# v0.4.0 validation — 2026-10-13 (user local date)

Apple Silicon user hardware checks passed for UADx Pultec and FabFilter Pro-Q VST3:
- REAPER stable class-ID identity: learn, both control directions, second instance, restart recall.
- Ableton source: recall mapping learned in Live without REAPER LEARN, both directions, restart recall.
- Source dropdown: persisted selection and switching back to REAPER mappings.
- MIX/PLUGIN USB reconnect tested on the user's USB-powered setup.

Automated protocol tests cover path/name-independent class IDs, malformed-ID fallback,
source parsing, nested manufacturer names, and exact captured Ableton hashes.
Surface mock regressions pass under REAPER and Ableton source configurations.
Linux extension including SWELL configuration UI compiles and links. Mac CMake build
and configuration UI passed user testing. Windows/Intel hardware remains unvalidated;
release workflow build/test results must be checked before publishing those packages.
The updated standalone Mac build script requires its own Mac run before relying on it.

---

# Current validation — 2026-10-10

v0.2.7 configuration persistence passed user restart and multiple-project checks. v0.3.0 all-tracks send levels passed user Apple Silicon hardware checks: send input and motor feedback, destination routing, banking, removal and Level/Pan/plugin/restart regressions. Send automation remains unvalidated. Windows/Intel build configuration and CI are prepared; CI results and volunteer hardware testing are pending. Automated surface tests cover destination names (including a second names page), shared-target resolution with different local send orders, hardware-output offsets, missing routes, input and motor feedback, touch suppression/edit completion, banking, routing removal, buttons and return to track Level. Protocol and existing surface regressions pass.

The records below are historical development notes. Their pending-test statements describe the state at the time; use this summary and README.md for the current release status.

User hardware checks on Apple Silicon macOS / REAPER 7.82 passed banking, track colors/names, meters, basic controls, plugin learn/recall, page isolation, instance selection/lock, removal/Undo, and measured unity-indent alignment. Firmware 3.2.1 and 3.3.0 reported SysEx API 1.6. Volume Read-mode motor playback and limited ReaEQ plugin automation were exercised.

Later continuous-hold automation checks failed intermittently due to incoming touch OFF events. This overrides earlier broad hold-test success reports. Direct USB data, separate supplied wall power, firmware 3.3.0 and stopped playback did not eliminate the issue. Standalone Snoize MIDI Monitor capture with REAPER closed recorded CC 52 ON at 23:10:30.094, OFF at 23:10:32.104 during a user-reported continuous hold over 20 seconds. Root cause unknown; support contacted. No release debounce workaround was added.

Windows and Intel macOS are unvalidated. No physical controller is connected to the development environment. Automated tests use mocks and cannot establish physical touch reliability.

---

# Validation record — 2026-10-09 UTC

- Linux x86_64: C++17 extension compiled and linked with g++.
- Exported `ReaperPluginEntry` symbol verified.
- Protocol test passed: exact Bitwig handshake response, full 14-bit word range, ASCII name limit, inverse volume feedback conversion.
- Simulated surface test passed: pre-handshake MIDI ignored, ping response ID 02, connected acknowledgment gating, mixer motor output, touched motor suppression, release state, paired-CC input, send/plugin mode isolation.
- Mac build script shell syntax validated.
- No macOS compilation, REAPER process integration, automation recording or physical Roto-Control test performed. Those are pending user hardware validation.

Upstream snapshots:
- Melbourne Instruments roto_control: 2d43a72c6699983080f8765292498502ff08be57
- Cockos reaper-sdk: c0eafe87863b2bf69c5c822760f1b32a753b211b
- Cockos WDL: d30c30b356b2b7fb1654def8dbda90066f43061a

All bundled SDK/WDL files are unmodified copies with upstream notices. Source scope and installation instructions are in README.md.

## v0.1.1 regression fix

Crash report from REAPER 7.82 ARM64 macOS: address zero immediately above CSurf_ShowConfig. Registration inspection confirmed null ShowConfig. Fixed by supplying a callable no-panel callback. Regression test checks the callback exists and invokes it as the host does when selecting the surface. Linux extension compilation and both test executables pass after this change. User hardware/macOS runtime retest is pending.

## v0.1.2 regressions

User log confirmed firmware 3.2.1/API 1.6 and handshake on macOS. Simulated tests now cover touch/release/re-touch across host track-list refresh and repeated MIX-mode notification, continued motor suppression after a timed refresh, ten-track banking to index 8 and back, and absence of FIRST TRACK during unchanged periodic refresh. Linux compilation and tests pass. Physical banking and touch behavior require retest.

## v0.1.4 outgoing partial-bank regression

Compared directly with upstream Bitwig MixLayerSet.sendStates and TrackState.toSysExUpdate. Mock test now compares every outgoing SysEx byte for ten tracks banked to offset 8: NUM TRACKS 10, FIRST TRACK 8, TRACK DETAILS 8 and 9, END. The equality check excludes nonexistent-track resets and extra messages. Paced output is pumped over simulated host Run frames. Touch suppression tests still pass through repeated mode/host refreshes. Native Linux extension compilation passes; macOS hardware confirmation remains pending.

## v0.1.5 palette colors

User confirmed v0.1.4 banking on hardware; touch improved with slight movement remaining. Color restoration changes only the palette byte in existing TRACK DETAILS. All 83 palette entries round-trip to their IDs; primary colors map as expected. Mock regression compares the complete partial-bank batch after a color-only change and after clearing the custom color. Protocol and surface tests pass, Linux extension compiles/links and exports ReaperPluginEntry. macOS build and physical color/banking retest remain pending.

## v0.2.0 plugin control — 2026-10-09

User confirmed v0.1.5 colors and banking passed. Reviewed SysEx API v1.6 sections 3.1–3.17 and Appendix B.2, plus official Bitwig PluginModeHandler, DeviceParameterSet, RotoParameter, StringUtil and ParameterSettings at upstream snapshot 2d43a72c6699983080f8765292498502ff08be57 before implementation. REAPER API signatures/semantics were checked in bundled official SDK headers.

Protocol tests cover known SHA-1/masked bytes, stable parameter IDs, plugin/learn packet lengths and positions. Surface mock tests cover enumeration, fresh-change learn gating, recall responses, two-way knob values, touched motor suppression/release, page-burst isolation, unknown hash rejection, seven-bit buttons, plugin selection, deletion/replacement, empty/invalid tracks, labels without parameter sweeps, recall on another track, lock, mode reentry and return to MIX. Existing mixer banking/color regressions remain in the same suite.

Linux extension compiles/links and exports ReaperPluginEntry; no warnings in own extension source. Protocol and surface tests pass. AddressSanitizer/UndefinedBehaviorSanitizer surface suite passes with leak detection disabled because LeakSanitizer cannot inspect /proc tasks in this environment. Mac build script shell syntax passes; no Mac binary can be built here. Hardware learning/persistence/page timing remain unverified; README records sparse/empty-page limitations.

## v0.2.1 selected-track label

Reviewed SysEx API v1.6 section 4.4 (DAW SELECT TRACK, 0C 04) and official Bitwig MainLayerHandler.sendUpdateFocusTrack / TrackState.toSysExUpdateSel at snapshot 2d43a72c6699983080f8765292498502ff08be57. Root cause: PLUGIN metadata never included the selected-track label packet.

Added cached, independent label updates using the current plugin target. Exact-packet protocol test and surface regressions pass for initial display, track selection, rename without DAW SELECT PLUGIN, unchanged-label suppression, palette color, pinned rename while locked, unlock, no valid selection and new connection handshake. Existing mixer and plugin tests pass. Bounded snprintf replaces the two test-only sprintf calls reported by the Mac compiler. Stable plugin/parameter identities remain unchanged. Linux extension compilation/link and Mac build-script syntax validated; hardware display retest pending.

## v0.2.2 direct cross-track plugin focus

Reviewed SysEx API v1.6 SET CURRENT TRACK NAME (0A 16). Code inspection found v0.2.1 enqueued the track context after DAW SELECT PLUGIN. Sends both selection details and explicit name before plugin selection, including updates arriving mid-batch. Mock FX instances now have distinct per-track GUIDs, and focus reports a configurable track index. Regression covers selecting track 1 then directly focusing track 2's plugin, confirming actual host target changes and exact track/name-before-plugin-selection ordering; reverse selection and mid-batch rename also pass. Prior protocol/mixer/plugin/lock tests pass. Linux extension compiles/links with no own-source warnings and exports ReaperPluginEntry; Mac build script syntax passes. No connected hardware available here; user display retest pending.


## v0.2.3 mixer automation feedback

User confirmed v0.2.2 track labels, saved mappings, restart and USB reconnect, Pro-Q 4 partial mappings on three pages and empty fourth page, MIX/PLUGIN and project switching, plugin removal and Undo restoration. Volume recording produced envelope points. Read-mode playback moved the REAPER fader but not the motor; this is a failed playback test, not a pass.

v0.2.3 adds cached track volume/pan surface callbacks, local input cache updates, and invalidation on track-list changes. Mock tests exercise multiple callback values while D_VOL stays fixed, exact MIDI motor pairs, suppression while touched and catch-up on release, and pan callbacks. Protocol and full surface regressions pass locally. Hardware automation playback and Mac build are pending. Plugin automation is not validated.


## v0.2.4 stereo meters

Primary protocol: Melbourne Instruments SysEx API v1.6, section 4.11 (threshold command) and Appendix B.1 (channel-16 stereo CC 65–80); the official Ableton integration also uses these CC pairs. REAPER SDK Track_GetPeakInfo defines amplitude 1 as 0 dB.

Protocol conversion tests cover silence, negative/nonfinite input, -60 dB floor, 0 dB, saturation, and monotonic scaling. Surface tests cover stereo pairs, thresholds 92/115, 20 Hz pacing, transient maximum retention, stable-value deduplication, silence clearing, unused screens, touch independence, actual track-pointer bank routing, master-only screen routing, and no peak reads or mixer meter output in PLUGIN. Full existing surface and protocol tests pass locally; Linux shared-library compilation and Mac script syntax pass. Mac build and physical meter rendering remain pending.

User subsequently confirmed v0.2.3 volume automation motor playback and ReaEQ Touch automation recording, constant holds between movements, release back to envelope, and motor playback. Slight movement while touched remains accepted; explicit plugin touch reporting is still absent in code, so this is evidence for the tested behavior rather than general compatibility.


## v0.2.5 MIX unity event

v0.2.4 user hardware tests passed stereo meter display, banking, unused-screen clearing, silence settling, normal volume control and plugin automation.

v0.2.5 adds missing channel-16 CC 64 handler, grounded in SysEx API v1.6 Appendix B.1 and official Ableton _default_volume_listener, which uses the CC value as a zero-based control index. Tests cover value 0, exact unity and feedback, bank-to-track routing, master view, invalid/missing slots, and no volume changes in Pan, Send or PLUGIN. Full protocol/surface tests and Linux module build pass. The user-reported -2.85 dB indent return remains pending hardware confirmation; an eligible event is logged for diagnosis. No assumption that the device's physical indent emits this event has been validated.


## v0.2.6 measured indent and touch feedback

User v0.2.5 log confirms correct version, API 1.6/firmware 3.2.1, no MIX unity event, raw position 13004 on knobs 1 and 2 at the reported -2.85 dB indent. Previous unity feedback was 13653. Piecewise volume conversion now anchors unity at 13004 and retains the -60/+12 dB endpoints with exact inverse round-trip across all 14-bit positions. Tests verify exact unity via ordinary MIDI CC, positions adjacent to unity, endpoint +12 dB, and existing motor automation/touch/meter/plugin regressions.

Knob 8 log touch ON/OFF repeats around 2.04-second intervals, each OFF followed by unchanged Motor TX, while user reports no touch. Touch logging occurs only in the incoming MIDI CC handler; cause unknown. Remove motor-target invalidation on touch events without filtering touches. Test confirms no redundant motor output on touch/release at an unchanged value; existing changing-host-value release tests still pass. Physical indent and idle touch behavior require retest.
