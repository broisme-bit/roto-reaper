# Roto-Control for REAPER — experimental v0.2.6

A native REAPER control-surface extension using Melbourne Instruments' Bitwig protocol personality (DAW ID 02). No Athens, Bitwig installation, OSC bridge or firmware modification is required. This is an independent prototype, not an official Melbourne Instruments integration.

## What is implemented

- Documented startup / ping / connected handshake; requests firmware and API versions for the log.
- Auto-detects exactly one `Roto-Control` input and output. Multiple matching devices are refused rather than guessed. Uses REAPER's MIDI API; no extra MIDI libraries.
- MIX all-tracks mode: volume and pan; mute, solo and record-arm switches.
- Track banks of eight, selected by the controller's SET FIRST TRACK event.
- Track names (12 ASCII characters). v0.1.5 maps REAPER custom colors to the nearest of the controller’s 83 palette colors. Tracks without a custom color use neutral palette 70. The published track count and completed track batch define unused positions.
- Master/return view exposes REAPER's master only. REAPER has no special return-track category; ordinary return tracks appear in the regular track list.
- 14-bit motor feedback. v0.2.3 uses REAPER volume/pan surface callbacks to follow the displayed fader during automation playback; feedback is suppressed while the knob is touched and resynchronized after release.
- Stereo peak meters in MIX all-tracks Level/Pan and master view, updated at up to 20 Hz; yellow at -12 dBFS and red at 0 dBFS. v0.2.4 hardware tests passed meter display, banking, unused-screen clearing, silence, volume controls and plugin automation.
- REAPER control-surface touch reporting for volume/pan automation.
- Transport Play, Stop and Record.
- Diagnostic log at `roto-reaper.log` in the REAPER resource directory.

- PLUGIN mode: enumerate the selected track's insert FX, select a plugin on the controller or by focusing its window in REAPER, learn parameters, recall controller-stored mappings, and exchange normalized parameter values in both directions. Supports mapped knobs and buttons, plugin enable/disable and lock.
- Plugin identities are separate from Athens and other hosts. This release sends no serial configuration commands and does not erase stored mappings when an unknown parameter hash is received.

Not yet implemented: Athens mapping import, automatic parameter assignment, input/take/container FX, FX chains beyond the first 127 insert FX, plugin automation touch reporting, MIX single-track view, sends, arbitrary display-value text, serial setup/haptics editing, detailed track-group support, transport functions beyond Play/Stop/Record, USB reconnect recovery, or user-selectable MIDI ports. Unsupported send/single-track modes ignore knob input. Returning to MIX all-tracks resumes control.

Minimum protocol target: SysEx API 1.3 (two-byte track indices); latest source reference is 1.6. User hardware testing has confirmed mixer banking and track colors with firmware 3.2.1/API 1.6. Touch improved but slight movement remains. User testing of v0.2.2 passed plugin learning/recall, Pro-Q 4 pages including an empty page, track labels, mode/project switching, removal/Undo, and unplug/replug on this Mac. v0.2.3 user testing passed volume Read-mode motor playback and ReaEQ parameter automation recording in Touch, constant holds, release and playback. Slight motor movement while held remains accepted. Broader plugin touch behavior is not established; macOS compilation runs on your Mac. Name/color refresh uses polling; tracks include REAPER's complete track list, rather than mirroring mixer visibility.

## Build on your Apple Silicon Mac

1. Unzip the package.
2. If necessary install Apple Command Line Tools: `xcode-select --install`.
3. Open Terminal, type `cd `, drag the unzipped `roto-reaper` folder into Terminal, and press Return.
4. Run `bash build-mac.sh`.

The script runs both tests, builds `build/reaper_roto.dylib` for the current Mac architecture, and applies an ad-hoc code signature. No CMake or downloaded dependencies are required. It works with native Apple Silicon REAPER when built on your M4 or M2. A binary built for ARM is not for a REAPER process running under Rosetta.

CMake is also supported: `cmake -S . -B build`, `cmake --build build`, `ctest --test-dir build --output-on-failure`. These commands are optional.

## Install and activate

1. In REAPER choose Options > Show REAPER resource path in explorer/finder.
2. Quit REAPER. Copy `build/reaper_roto.dylib` into the resource folder's `UserPlugins` directory (create it if needed).
3. Close Athens and other software that controls the Roto-Control.
4. Connect the controller before launching REAPER. In REAPER's MIDI device preferences disable its normal input/control-input and output usage so the new surface owns the ports. Remove any old Athens/OSC surface configuration that would also control these tracks.
5. In Preferences > Control/OSC/web, Add the entry `Roto-Control REAPER (Bitwig protocol, experimental v0.2.6)`.
6. There is no port-selector dialog in this prototype. It finds the single input/output containing `Roto-Control`. If REAPER aliases those names, restore that name. Do not add this surface twice.
7. Select MIX > all tracks on the controller, with Level and Mute selected. If it shows a DAW identity, Bitwig is expected because that is our protocol personality.

If the extension is absent from the list, check the build output, file location and architecture. If ports fail, check exclusivity and the exact names. The log is recreated when the surface is instantiated. Remove and re-add the surface after reconnecting the USB device.

## First hardware test

Use a disposable empty project with automation in Trim/Read and playback stopped. Create two tracks called Drums and Bass; start with monitor volume down. This makes initial control tests easy to observe.

1. Open `roto-reaper.log`. Expect `Ports opened`, `Ping received`, and `ROTO-DAW CONNECTED`, plus firmware/API version responses. If there is no connected acknowledgment, stop here and retain the log.
2. Track names should appear. Change a name/color in REAPER and allow two seconds for refresh.
3. Move the first REAPER track fader with the mouse. Knob 1 should follow.
4. Turn knob 1. Only the first track's level should change. The knob scale is silence at its minimum, then -60 to 0 dB below the unity indent and 0 to +12 dB above it. It uses a fixed decibel-to-position mapping rather than REAPER's configurable visual fader taper. This does not change the track's gain range. v0.2.6 aligns unity to the raw indent value 13004 observed on firmware 3.2.1/API 1.6. CC 64 also restores unity if emitted; it did not arrive in the user's indent test. Physical alignment awaits the v0.2.6 retest. Pan uses a linear -1 to +1 scale.
5. Touch and hold knob 1, then change the software value. The extension should not send a new motor position while touch is active. Release it and confirm resynchronization. This policy avoids actively driving the motor against a hand; actual physical behavior still depends on firmware.
6. Select Pan and test knob 1; select Mute/Solo/Arm and test button 1. Switch back to Level/Mute afterward.
7. Create at least ten tracks and bank right: knobs should now control tracks 9–10 and unused screens should clear. Bank left again.
8. Only after those pass, test touch automation on a disposable volume envelope with REAPER in Touch mode. Confirm writing starts while touched and ends on release. Do not use a valued recording session for this initial test.

Share the log and which numbered step first fails. There is no connected hardware in the development environment, so automated tests do not prove this checklist works on the device.

## Development / validation

The extension and tests compile with g++ on Linux. The tests check known handshake bytes, all 16,384 track-index encodings, text truncation, volume conversion round trips, a simulated startup handshake, state gating, motor updates, touch suppression/release, and isolation of unsupported modes. Tests keep assertions enabled even in Release builds. macOS binaries must be built and tested on macOS; the package intentionally contains source rather than a misleading Linux binary renamed as a dylib.

Next stage: validate plugin learning, recall and page switching on hardware. Serial remains reserved for optional stored haptics/configuration tooling.

## References and provenance

- https://github.com/Melbourne-Instruments/roto_control
- `docs/ROTO-CONTROL_SYSEX_API_v1.6.pdf`, `docs/ROTO-CONTROL_SERIAL_API_v1.3.pdf`
- Official Bitwig `MidiProcessor.java`, `RotoKnob.java`, `TransportState.java`; Ableton and Logic sources were also reviewed.
- https://github.com/justinfrankel/reaper-sdk — bundled unmodified SDK headers.
- https://github.com/justinfrankel/WDL — bundled unmodified dependency headers/source; upstream notices are preserved.

Melbourne Instruments' source was consulted, not copied into this extension. Original extension source is MIT licensed; vendored files retain their upstream licenses.

## v0.1.1 fix

The original v0.1 registration left ShowConfig null. REAPER 7.82 on macOS called that pointer when selecting the surface, causing a null-address crash in CSurf_ShowConfig before the controller connection was opened. v0.1.1 supplies a callable configuration handler, which returns no custom panel because ports are automatically detected. A regression test invokes that callback.

To replace v0.1: quit REAPER, build this updated source with `bash build-mac.sh`, then replace the existing `UserPlugins/reaper_roto.dylib` with the newly built file. Keep only one copy in UserPlugins. Launch REAPER and repeat Install and activate step 5. The list entry now ends in v0.1.1. macOS/REAPER runtime confirmation remains pending.

## v0.1.2 banking and repeated-touch fix

Touch state now persists through REAPER track-list callbacks and controller mode refreshes until a physical touch OFF message arrives. Periodic label/color refresh no longer sends navigation commands or unchanged track metadata. Bank requests and every touch ON/OFF transition are now logged. Hardware confirmation remains pending.

Rebuild with `bash build-mac.sh`, quit REAPER, replace UserPlugins/reaper_roto.dylib, and restart. Retest touch twice (hold each time for at least five seconds while moving the software fader), then bank between tracks 1–8 and 9–10. Keep automation test 8 on hold until both pass. Attach the new roto-reaper.log if either fails.

## v0.1.3 diagnostic build

This build logs MIX mode payload, requested/accepted bank including REAPER track count, and motor commands with touch state. Unchanged switch feedback is no longer sent every Run cycle. This is a diagnostic change; the physical banking and repeated-touch failures are not yet confirmed fixed.

Rebuild and replace the installed dylib with REAPER closed. Verify the log begins with v0.1.3. For one short test, make sure the project has ten ordinary tracks, select MIX/all tracks/Level/Mute, bank right once, then touch knob 1 continuously while moving its software fader. Release, touch again, and repeat. Note whether you actually lifted your finger when the motor moved. Share the log. Do not test automation yet.

## v0.1.4 partial-bank protocol revision — test banking first

The first builds sent RESET TRACK DETAILS for nonexistent positions in a partial bank. The official Bitwig MixLayerSet sends only existing tracks followed by TRACK DETAILS END. This revision uses that complete reference sequence: NUM TRACKS, FIRST TRACK, TRACK DETAILS for each existing track, TRACK DETAILS END. For ten tracks and bank offset eight, the payload has exactly two track-detail messages. No RESET TRACK DETAILS, SET TRACK DETAILS or RGB messages are sent on this baseline path. Unused positions are left to the firmware's track-count/batch handling, matching the official integration. Physical confirmation is still pending; this is a targeted protocol correction, not a proven hardware fix.

The batch is paced at one message per Run frame (at least 30 ms apart). Motor and button feedback pause until the batch finishes and a 50 ms settling interval expires. Track metadata is resent only when count/bank/name/color changes, not every polling cycle. Metadata TX commands and batch completion are logged. Custom track colors temporarily use neutral palette 70 while we isolate this problem.

Build and replace the dylib using the earlier instructions, with REAPER closed. Power-cycle the controller before the new test. Create/save a disposable ten-track project before connecting. Start REAPER, select MIX/all tracks/Level/Mute, then bank right to tracks 9–10 and left to 1–8. First verify that names, controls and the controller's Mode button remain responsive. If the controller freezes again, stop and share the log. If banking passes, perform the repeated touch test and share its result. Automation test 8 remains on hold until touch handling is confirmed.

## v0.1.5 — restore track colors

Banking passed the user's v0.1.4 hardware test. This release restores colors using the palette byte already present in TRACK DETAILS. It adds no SysEx messages and keeps the same bank batch, pacing and motor settling interval. Colors approximate REAPER RGB values using the nearest palette entry; exact RGB output remains deferred. Touch behavior is unchanged.

Build and install this version using the steps above, replacing the existing extension while REAPER is closed. Test with ten tracks:

1. Give tracks 1 and 9 distinct custom colors in REAPER, such as red and green.
2. Confirm their controller screens show those colors, banking forward and back.
3. Change a visible track color; allow two seconds for refresh.
4. Remove that track's custom color; its screen should return to neutral.
5. Confirm banking still works after these changes. If it freezes, retain roto-reaper.log and report which action triggered it.

Hardware color confirmation is pending. Do not run automation test 8 yet.

## v0.2.0 — plugin learning and saved mappings

Build and replace the existing extension while REAPER is closed, following the instructions above. Keep only one reaper_roto.dylib in UserPlugins. The control-surface entry should show v0.2.2 after restarting.

### Will saved mappings work?

Ordinary MIDI setups stored on the controller are unchanged. Existing Athens/Ableton/Bitwig plugin assignments are not automatically compatible: those integrations identify plugins and parameters differently. This release deliberately uses a separate REAPER identity rather than guessing a match and controlling the wrong parameter. Learn your REAPER assignments once with this extension; Roto-Control stores them and supplies them again when that plugin is selected in another session or on another track. This device-side persistence still needs hardware confirmation.

The stable identity uses REAPER's canonical fx_ident (falling back to its original FX name), not the track name, FX instance GUID or display alias. Parameter IDs use TrackFX_GetParamIdent; if unavailable they use the parameter index plus name. The protocol hash is SHA-1 truncated and masked to seven bits, following the official Bitwig encoding. VST2/VST3/AU variants and plugin updates that change IDs may require separate learning. There is no automatic import of Athens mappings.

### First plugin hardware test — use ReaEQ

Use a separate project, playback stopped and automation in Trim/Read. No audio is required. Do not use automation recording for this test.

1. Create one track named EQ Test and insert ReaEQ. Select the track and open/focus its ReaEQ window.
2. Switch the controller to PLUGIN and select ReaEQ if the plugin list is shown. The list should contain ReaEQ. Initially, unassigned controls may be blank; this is expected. Parameters are learned, not automatically assigned in index order.
3. Press LEARN on the controller, touch knob 1 as the learning target, and move ReaEQ band 1 Frequency with the mouse. Its label should appear. Learn band 1 Gain on knob 2 the same way. Exit LEARN.
4. Turn each learned knob. Confirm only its assigned parameter changes. Move those parameters with the mouse and confirm the corresponding motorized knobs follow. Frequency follows the plugin's normalized scale, not a linear Hz scale.
5. Touch/hold knob 1 and move its software parameter; check that normal motor feedback pauses and resumes after release. Slight physical movement may still occur as in the mixer test.
6. Switch to controller control page 2. Learn a different ReaEQ parameter on knob 1 of that page. Exit LEARN, then switch between pages 1 and 2. Confirm names, values and assignments follow each page; unassigned slots should not control an earlier page's parameters. These are pages of learned controls, not the next eight REAPER parameters automatically.
7. Save the test project, close and reopen REAPER, select the track and ReaEQ, and enter PLUGIN. Confirm assignments return without LEARN. Also try another track with another ReaEQ instance; the same assignments should control that instance's values.
8. Return to MIX all tracks, Level/Mute. Check volume, mute, colors and track banking still work. Keep automation test 8 from the earlier checklist on hold.

If any step fails, stop that test and share roto-reaper.log plus the numbered step. For recall/page problems, mention whether labels appeared and whether turning a knob changed the previous page's parameter. Logs include Plugin selected, Plugin RX, Learn candidate, Control learned acknowledgment, and Control mapped. A parameter=-1 / hashMatches=0 entry means the stored assignment did not match a parameter in this extension; it is ignored without deleting the controller's assignment.

### Scope and limits

Plugin lock pins the current valid instance. Software target changes while a knob is held are deferred until release. Deleting or replacing an instance invalidates its bindings. Only track insert FX are enumerated; record-input, take FX and nested containers are deferred. Learn mode watches REAPER's last-touched parameter and requires a fresh touch/change after entry. It reads step sizes and formatted labels without sweeping or modifying parameter values. Unsupported step metadata uses a smooth sweep; custom detents/center indents are deferred.

The normal Bitwig protocol reports mapped controls for the current page, without an explicit page number or mapping-batch END. This prototype groups recall messages within 250 ms and clears prior slots at the start of a new batch. A fully empty page provides no mapping notification in the published normal-plugin protocol; feedback to prior slots may persist until a mode/selection notification or a subsequent mapped page arrives. Verify sparse-page behavior on hardware before broader use. Incoming plugin select/enable/track submodes suspend parameter control.

Checks: SHA-1 reference bytes and packet layouts; learning baseline; mapping resolution; two-way normalized values; mapped buttons; touch suppression/release; partial-page stale-slot clearing; unknown hashes; another instance; lock; deletion/replacement; return to mixer. Linux builds and address/undefined-behavior sanitizer checks pass. Physical hardware and macOS runtime validation remain pending for plugin mode.

## v0.2.1 — plugin track-name display

User hardware tests confirmed plugin/track switching and lock, ReaEQ learning and recall in a new project, plus Pultec continuous controls, a stepped control and an Enable button. The remaining display defect was a stale or missing track name underneath the plugin name.

The extension now sends the documented DAW SELECT TRACK packet (0C 04), as the official Bitwig integration does when entering PLUGIN mode. It supplies the controlled track's index, 12-character name and palette color. The label updates when switching targets, renaming/recoloring a track, entering PLUGIN mode or completing a new connection handshake. Lock keeps the label on the pinned plugin's track. Track-only changes do not reselect the plugin or change learned assignments. No selected track sends an empty label. Plugin identities/hashes are unchanged, so existing v0.2.0 mappings should recall without relearning.

Build/install as before, with REAPER closed while replacing the extension. Also replaces the two deprecated sprintf calls in test mocks with bounded snprintf calls.

Test the plugin header with tracks named Drums and Bass:

1. Enter PLUGIN and select a plugin on Drums. Verify Drums appears below its name.
2. Select Bass and its plugin. Verify Bass appears below that plugin's name.
3. Rename Bass to Bass New. Verify the label updates without losing assignments.
4. Lock the Bass plugin, then select Drums in REAPER. The label should stay on Bass New while controlling the locked plugin. Unlock; it should follow Drums again.
5. Restart REAPER and confirm the label appears immediately after connection, along with your saved assignments.

Physical USB unplug/replug recovery is still not implemented; after reconnecting, restart REAPER or remove/re-add the surface to reopen its MIDI ports. This label fix does not depend on unplugging the controller.

## v0.2.2 — track context before direct plugin selection

v0.2.1 sent the selected-track label after DAW SELECT PLUGIN. The user reported that explicitly selecting a track then its plugin worked, but opening a plugin directly on another track left the prior track name while the knobs followed the new plugin.

This revision queues DAW SELECT TRACK (0C 04) and SET CURRENT TRACK NAME (0A 16) before the plugin list/DAW SELECT PLUGIN. The explicit current-name command also updates the label for renames without requiring a plugin reselection. If the name changes during a paced plugin batch, stale queued labels are replaced ahead of its selection message. Metadata logs now include the command group so the two label packets can be distinguished. Uses the documented API available on your firmware 3.2.1/API 1.6; saved mapping identities remain unchanged.

Build/install as before. Test with lock off and knobs released:

1. Select track 1 and its plugin.
2. Open/focus a plugin on track 2 directly, without selecting track 2 first. Both knobs and the lower track-name label should follow track 2.
3. Open track 1's plugin directly and check the same behavior in reverse.
4. Rename the active track and verify the label updates. Recheck plugin lock: the name should describe the pinned plugin's track.

The message-order regression passes in software; firmware display behavior still needs this hardware retest. If it fails, retain roto-reaper.log and identify the displayed name versus the plugin's actual track.


## v0.2.3 — mixer automation motor feedback

Read-mode volume automation moved the REAPER fader but left the motor still. The extension previously polled D_VOL/D_PAN, which do not provide the displayed automated fader position in this case. It now caches SetSurfaceVolume/SetSurfacePan callbacks by track and uses those values for mixer motor feedback. Polling remains the fallback until a track receives a callback. Local knob edits update the cache because the host broadcast excludes this surface. Track-list/project changes discard cached pointers and values. Touch suppression and metadata pacing still apply. Plugin mapping identities and packets are unchanged.

Update with REAPER closed: build using `bash build-mac.sh`, replace the installed UserPlugins/reaper_roto.dylib with build/reaper_roto.dylib, then restart. Keep only one installed copy. Confirm the new log starts with `v0.2.3 mixer automation feedback`.

Retest the existing volume envelope:

1. Set the track to Read, Roto-Control to MIX/all tracks/Level, and play through the recorded changes. The hardware knob should follow the moving REAPER fader without being touched.
2. While playing, touch and hold the knob without turning it. Motor feedback should pause; release it and check that it follows the current fader position again.
3. Stop playback, select Trim/Read, and check normal knob volume adjustment. Trim/Read can play the envelope while the knob adjusts the underlying trim; do not expect it to mirror the envelope in the same way as Read.
4. Confirm a saved plugin mapping still recalls. If playback feedback fails, retain roto-reaper.log and describe whether the REAPER fader moved and whether the knob was touched.

Local tests pass for changing automation callbacks with fixed D_VOL, 14-bit output, touch suppression/release, pan callbacks, cache clearing, and existing mixer/plugin regressions. Physical motor playback and Mac compilation require the above hardware test. Plugin automation touch reporting remains unimplemented.


## v0.2.4 — stereo mixer peak meters

Adds visual peak levels on the eight MIX all-tracks screens using the documented channel-16 CC 65–80 stereo pairs. The documented SysEx MIX command 0C/0B configures color thresholds. REAPER Track_GetPeakInfo channels 0 and 1 provide the left/right peak amplitude (not RMS or averaged VU). Values map -60 through +6 dBFS to 0–127; silence and invalid values map to zero, and levels above +6 dBFS saturate. Yellow begins at approximately -12 dBFS and red at approximately 0 dBFS, subject to the device's threshold rendering and 7-bit rounding. These are visual meters, not a latched clipping detector.

Samples retain the maximum peak between updates. Updates occur at no more than 20 Hz and only changed values are transmitted. Display batches finish before meter transmission. Bank/view changes reset meter values; unused screens receive zero. Master view uses the first screen only. Meter updates continue while knobs are held and never send knob CCs or write REAPER parameters. PLUGIN mode and unsupported MIX views do not stream mixer meters. Saved mappings and motor/automation behavior are unchanged.

Build/install with REAPER closed as before, replacing only UserPlugins/reaper_roto.dylib. Restart and confirm the log begins with `v0.2.4 stereo mixer meters`.

Hardware check:

1. Select MIX/all tracks/Level and play tracks containing audio. Check that the controller's stereo meter bars respond and correspond to the tracks above each knob.
2. Bank to tracks 9–16 (or a partial last bank). Check that meters follow the new tracks and unused screens have no level bars.
3. Stop audio and allow any effect tails to decay. Check that meters fall to zero. Live input monitoring may keep a track meter active while transport is stopped.
4. Hold a knob during playback. Its meters should continue updating. Confirm normal volume control and Read-mode automation motor playback still work.
5. Switch to Pan and master view, then PLUGIN and back to MIX. Check meter routing and that a saved plugin mapping recalls.

Hardware display and exact device colors await this test. If bars are absent, send the current log and a photo of the controller in MIX during playback. Do not increase audio levels solely to test red.


## v0.2.5 — MIX unity/default-level event

User reported that returning to the volume indent produced -2.85 dB. SysEx API v1.6 Appendix B.1 defines CC 64 as MIX track level at 0 dB. The official Ableton script's default-volume listener interprets its value as a zero-based control index and restores that control's default parameter value. Our extension had ignored this event.

v0.2.5 handles channel-16 CC 64 values 0–7 in MIX/all-tracks Level view by setting the indicated track to unity gain (exactly 1.0 / 0 dB), updating the cached host value, and resynchronizing motor feedback under the existing touch rules. Bank and master routing apply. Invalid indices, missing tracks, PLUGIN, Pan and unsupported Send modes do not change volume. The log records `MIX unity event` when an eligible message arrives. No visual taper, plugin mapping, meter, or automation mapping is changed.

This fixes a missing protocol handler; whether the reported indent triggers it is not yet confirmed. Do not treat the physical indent problem as resolved until the hardware test passes.

Build/install as before with REAPER closed, then confirm `v0.2.5 mixer unity event` at the start of the log. On a test track in Trim/Read with playback stopped and no volume automation, select MIX/all tracks/Level. Move below the indent and return slowly, then repeat from above. Check whether the REAPER level reads 0.00 dB each time. Repeat on another knob and another bank. If it still lands at -2.85, send the new log; whether `MIX unity event` appears will determine the next step. Briefly check meter playback and saved plugin recall afterward.

Matching REAPER's visual taper would be a separate optional change to knob travel. It is not required for an exact unity reset.


## v0.2.6 — align unity with measured indent

The v0.2.5 hardware log showed no unity CC 64 event. Knobs 1 and 2 both returned to raw position 13004, corresponding to -2.85 dB on the old fixed scale; unity feedback had been position 13653. v0.2.6 anchors the conversion and inverse at 13004/16383 = exactly 0 dB, with -60 to 0 dB below that point and 0 to +12 dB above it. This is grounded in the tested firmware 3.2.1/API 1.6, rather than an asserted universal firmware constant. Normalized plugin parameters and pan conversion are unchanged. Existing project levels and envelope values remain in their original REAPER units; knob travel changes.

The log also showed knob 8 touch ON/OFF repeatedly arriving approximately every two seconds while the user was not touching it. The extension does not synthesize these touch messages. It previously invalidated motor feedback on every touch event, which made it resend an unchanged position after each OFF. That redundant invalidation is removed: actual value changes still resynchronize after release. Raw touch CC/value is now included in the log. This does not filter incoming touch or establish the cause of false touch detection.

Build/install as before and confirm `v0.2.6 unity indent alignment`. With playback stopped, no volume envelope, and Trim/Read, return to the indent from both directions on knobs 1 and 2. Expect 0.00 dB. Then check knob 8 idle for 10–15 seconds. If touch messages continue, retain the new log; unchanged Motor TX should no longer accompany every OFF. Check ordinary volume control, Read-mode automation motor playback and plugin recall before using this release as the new baseline.
