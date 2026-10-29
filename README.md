# Roto-Control for REAPER — v0.4.1

An independent native REAPER control-surface extension for Melbourne Instruments Roto-Control. Uses the published Bitwig protocol personality (DAW ID 02); no Bitwig installation, bridge app or firmware modification is required. Not an official Melbourne Instruments or Cockos integration.

## Status and known touch issue

Tested by one user on Apple Silicon macOS, REAPER 7.82, firmware 3.2.1 and 3.3.0, SysEx API 1.6. Windows and Intel Mac builds are not validated.

**Continuous knob touch can drop out before the hand is released. Touch automation holds are currently unreliable.** Incoming touch OFF events were reproduced outside REAPER using Snoize MIDI Monitor with REAPER closed, direct USB data to the Mac and the supplied wall adapter. One continuous hold over 20 seconds reported OFF after 2.01 seconds. This establishes an issue independent of this extension in that test, but does not determine the hardware, firmware or electrical cause. Reported to Melbourne Instruments; awaiting guidance. No artificial release delay is applied.

The extension suppresses motor feedback while its received touch state is ON. An early OFF allows feedback and can cause unwanted automation changes. Begin testing in Trim/Read with playback stopped, using a disposable project. Do not rely on Touch holds for important automation recordings.

## Features

- MIX all-tracks: volume, pan, mute, solo and record arm; banks of eight.
- MIX single-track preview: selected-track Level/Pan and send levels, with additional send pages. Ordinary mixer buttons are inactive in this view. See SINGLE-TRACK-TEST.md; hardware validation pending.
- Automatic MIDI reconnect; Apple Silicon hardware checks passed for v0.3.1.
- MIX all-tracks Send levels: select an existing destination bus and control each visible track's send to it, with names and motor feedback. Basic Apple Silicon hardware checks passed for v0.3.0; send automation remains unvalidated.
- Track names and approximate custom colors using the controller's fixed palette.
- 14-bit motor feedback, including volume/pan envelope playback through REAPER surface callbacks.
- Stereo MIX peak meters, up to 20 Hz, with yellow at -12 dBFS and red at 0 dBFS.
- Master track view; ordinary return tracks stay in the normal track list.
- PLUGIN: selected-track insert FX, focused-plugin selection, learned knobs/buttons, multiple pages, plugin enable/disable and lock.
- Controller-stored plugin mapping recall across projects. Hardware checks included ReaEQ, Pultec and Pro-Q 4.
- Play, Stop and Record transport.
- Diagnostic log and automatic detection of one Roto-Control MIDI input/output.

## Install on Apple Silicon macOS

If the release has a macOS ARM64 binary asset, unzip it and use the included `reaper_roto.dylib`. Otherwise build from source below. ARM64 requires native Apple Silicon REAPER, not REAPER running under Rosetta.

1. In REAPER, choose **Options > Show REAPER resource path in explorer/finder**.
2. Quit REAPER. Copy `reaper_roto.dylib` into that resource folder's **UserPlugins** directory. Create the directory if needed. Keep only one copy of this extension there.
3. Close other applications controlling Roto-Control. Connect the controller before starting REAPER.
4. In REAPER's MIDI device preferences, disable normal input, control-input and output use of Roto-Control so the control surface owns the ports. Disable other surfaces using the same device.
5. In **Preferences > Control/OSC/web**, add **Roto-Control REAPER v0.4.0**. Add it only once.
6. On the controller select **MIX > all tracks**, **Level** and **Mute**. A Bitwig identity is expected: it is the protocol personality.

There is no MIDI port selector. The extension requires exactly one input and output containing `Roto-Control`; restore original names if you have aliased them. Multiple matching devices are rejected. If absent from the surface list, check architecture, build output and installation location. The surface can remain installed while the controller is absent. It checks ports every 0.5 seconds and repeats the handshake after reconnecting. An idle API-version query checks the connection every 2 seconds; an unanswered query triggers reopening after 5 seconds. If REAPER itself does not rediscover the MIDI device, use its MIDI-device reset or restart REAPER. See RECONNECT-TEST.md for the hardware test checklist.

### Build from source on macOS

Install Apple's Command Line Tools if needed: `xcode-select --install`.

Open Terminal, type `cd `, drag the unzipped project folder into Terminal, press Return, then run:

```sh
bash build-mac.sh
```

This runs the tests, builds `build/reaper_roto.dylib` for the current Mac architecture, and applies an ad-hoc signature. No CMake or extra dependency downloads are required. Intel compilation is possible in principle, but has not been tested.

### Remove or roll back

Remove this surface from **Preferences > Control/OSC/web**, quit REAPER, then remove `UserPlugins/reaper_roto.dylib`. To roll back, install a previous binary with REAPER closed. Restore your MIDI device preferences if you want ordinary MIDI use afterward. Removing the extension does not erase controller-stored mappings.

## Plugin mappings

Select a track and open/focus an insert FX window, then switch Roto-Control to PLUGIN. Use LEARN on the controller to assign parameters. The controller stores assignments and recalls them when the same plugin identity is selected again, including in another project.

In **Preferences > Control/OSC/web**, edit the Roto-Control entry and choose **Plugin mapping source: REAPER / Ableton**. The choice is saved across restarts and reconnects. REAPER is the default. Ableton mode reuses controller-stored ordinary VST2, VST3 and AU mappings learned in Live; it does not import or convert files. Effects and instrument type labels are accepted. CLAP retains REAPER identities. Bitwig/Logic compatibility and rack/macro mappings are not included. VST2 instruments and AUv3 lack successful hardware validation.

**Upgrade note:** recognized VST3 plugins now use a path-independent REAPER identity. Existing REAPER VST3 mappings must be learned again in REAPER mode. Old setups remain stored and can be recalled with the previous extension. Other formats retain their previous identities. Selecting Ableton does not erase REAPER mappings, and switching back recalls mappings learned under the new REAPER scheme.

Hardware validation on Apple Silicon covered UADx Pultec and FabFilter Pro-Q: control in both directions, REAPER mapping recall across instances/restarts, Ableton VST3 recall, saved source selection, and switching between sources. Other plugin names or parameter labels may differ between hosts. Plugin updates that change parameter identities may require relearning. The extension sends no serial configuration commands.

## Quick beta check and feedback

Start in a disposable project with playback stopped, Trim/Read automation and monitor volume low.

1. Create two tracks. Verify names/colors and volume in both directions.
2. Test pan and mute/solo/arm buttons.
3. Create ten tracks; bank to tracks 9–10 and back. Verify unused screens clear.
4. Play audio and verify meters. Test a learned plugin and recall in another project.

The log is `roto-reaper.log` in the REAPER resource folder. It should show `Ports opened`, `Ping received`, `ROTO-DAW CONNECTED`, firmware and API versions. It is recreated when the surface is instantiated, so save a copy before restarting or re-adding the surface.

For a report, provide macOS and REAPER versions, Mac architecture, firmware/API versions, extension version, connection/power arrangement, reproduction steps and the saved log. Review the log for track/plugin names before posting publicly. For touch reports, say whether your hand stayed on the knob and how long the hold lasted.

## Send levels

In MIX all-tracks, select Send as the knob mode and choose a destination. Destinations are ordinary REAPER tracks receiving at least one existing track send, listed in project track order (up to 127). Each knob adjusts its visible source track's send to that destination. Source tracks retain their screen names and meters; the send selector shows destination names.

Routes are resolved by destination, so different local send orders on different tracks do not change the target. If a source has no matching send, its knob feedback is zero and turning it does nothing; the extension does not create sends. If a source has several sends to the same destination, the first is controlled. Hardware outputs and receives are excluded. Volume scaling uses the existing silence/-60 to +12 dB curve. Buttons continue controlling track mute, solo or record arm.

Send feedback reads REAPER's UI value and is suppressed while touch is ON. Edits use REAPER's send UI API and end on release. Send automation Touch holds are not validated; start in Trim/Read with playback stopped. The existing touch-dropout limitation still applies. Single-track MIX uses the same destination list, with Level/Pan followed by send levels.

## Limitations and future work

Serial setup/custom haptics editing, automatic parameter assignment, input/take/container FX and user-selectable MIDI ports are not implemented. Only the first 127 insert FX are listed. Reconnect depends on REAPER/OS MIDI device availability.

Plugin automation has received limited user testing; explicit plugin automation touch reporting is not implemented. Name/color refresh is polled and the complete REAPER track list is used, rather than mixer visibility. Track volume uses silence at minimum, -60 to 0 dB below unity and 0 to +12 dB above unity. The physical unity indent was aligned to measured raw position 13004; this does not follow REAPER's configurable visual fader taper. Pan is linear.

## Contributing and Windows

Original extension code is C++17 under MIT. Contributions, especially a Windows port and additional hardware testing, are welcome. Protocol and mapping logic should be reusable, but a Windows DLL build and host/MIDI validation are still required. CMake includes MSVC compiler settings and a static C++ runtime for Windows. GitHub Actions builds and tests Windows x64, Intel Mac and Apple Silicon Mac; successful compilation alone does not establish hardware support. Do not describe a Windows build as supported until it is tested.

Optional CMake workflow (Windows: use a Visual Studio developer shell and add `-A x64` to the configure command):

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Linux builds and mock tests check protocol and integration behavior; they do not establish working Linux hardware support. See VALIDATION.md for current validation and docs/DEVELOPMENT-HISTORY.md for historical development notes, whose old pending-test instructions are superseded by this README.

## References and licensing

- Protocol and official integrations: https://github.com/Melbourne-Instruments/roto_control
- API references: SysEx v1.6 and Serial v1.3 in the upstream project's docs directory.
- REAPER SDK: https://github.com/justinfrankel/reaper-sdk
- WDL: https://github.com/justinfrankel/WDL

Melbourne Instruments source was consulted, not copied into the extension. Original extension source is MIT licensed. Bundled SDK/WDL files retain their upstream notices and licenses.

## Experimental platform packages

The Build beta packages workflow produces separate Windows x64, Intel Mac x86_64 and Apple Silicon ARM64 ZIPs after automated tests pass. Download them from the completed workflow run's Artifacts section; the downloaded artifact may wrap the release ZIP in another ZIP. These artifacts do not automatically publish a release. Intel Mac and Windows need volunteer REAPER/controller validation. Use the matching native REAPER architecture. Installation uses UserPlugins on every platform; Windows loads reaper_roto.dll and macOS loads reaper_roto.dylib.

### VST3 mapping identity

VST3 mappings use `ROTOREAPER:v2:VST3:<class ID>` as their hash input.
The class ID is extracted from REAPER's `fx_ident`, normalized to uppercase,
and used without the plugin installation path or display name. Unknown or
malformed class-ID encodings retain the previous `ROTOREAPER:v1:` identity.
Other formats and parameter identifier extraction are unchanged in this stage.

This changes both plugin and parameter hashes for recognized VST3 plugins.
Existing v1 VST3 mappings are not migrated or automatically recalled; learn a
new mapping for v0.4.0. The old setup remains stored on the
controller. Returning to the previous extension restores its old identity rules.
This is the REAPER scheme; Ableton mode uses the separate rules below.

### Mapping source

In Preferences > Control/OSC/web, edit the Roto-Control entry and select the
Plugin mapping source: REAPER or Ableton. REAPER saves this selection with the
surface entry, across restarts and hardware reconnects. Older `0` configuration
entries default to REAPER. CLAP retains REAPER mapping rules.

Ableton mode hashes the device class plus original plugin name and the reverse
order, four bytes each, and hashes parameter names to six bytes. VST2/VST3 use
`PluginDevice`; AU uses `AuPluginDevice`. This mirrors Melbourne's Live integration
for ordinary plugins. REAPER's original decorated name is parsed to remove its
format and trailing manufacturer. Name differences between hosts can prevent
recall. Duplicate parameter names remain ambiguous unless the saved index hint
also resolves the matching parameter. Rack/macro mappings are not implemented.
New mappings learned in REAPER with Ableton selected use those same rules;
recall back in Live has not been validated.

Selecting a different source changes the mappings requested and newly learned;
it does not convert or delete existing controller setups. Use Apply/OK to save.

### Ableton compatibility validation

Apple Silicon hardware checks passed for AU Pultec, AUi Moog Minimoog and VST2
Devil-Loc Deluxe: existing Live mappings recalled without REAPER LEARN, control
in both directions and restart recall. VST3 Pultec and VST3i Moog regressions
also passed. VST3i Moog already worked in v0.4.0; no general instrument fix is
claimed. The separate reporter's instrument failure remains unresolved.

VST2 Twin 3 is a known pending name mismatch: REAPER reports FabFilter Twin 3,
while the Live export uses Twin 3. Manufacturer names are not stripped from the
start of plugin names. VST2 instrument success and AUv3 are not validated.
REAPER-mode identities and previously implemented Ableton VST3/AU hashes are
unchanged by adding VST2 support. No Logic compatibility is included.

### Plugin value display development test

Mapped plugin knobs and buttons send SHOW CONTROL VALUE (SysEx 0A 18) using
REAPER-formatted parameter text. Formatting is polled at most 20 Hz and unchanged
wire text is suppressed. Touching a knob redisplays its mapped knob/button values;
button input also requests its value. Text updates continue during touch while
motor feedback remains suppressed. Missing formatter text falls back to a labelled
normalized percentage. Mapping/page changes clear display caches. Existing
REAPER/Ableton mapping identities and stored setups are unchanged.

User hardware checks on Apple Silicon passed formatted plugin values, mouse-change
feedback, stationary-touch redisplay, mapped buttons and page switching. A Pultec
Low Frequency custom step label edited in ROTO-SETUP from 120 to 100 also displayed
correctly. This verifies that edited label, not every custom-step configuration.
The extension does not read custom controller step-slot labels. No Logic mapping
support is added; future Logic-compatible mappings can reuse this feedback path.
