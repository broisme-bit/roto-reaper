# MIX single-track — v0.3.2 preview

This adds the existing Roto-Control single-track MIX layout using SysEx API v1.6
and Melbourne Instruments' Bitwig implementation as references. No firmware
update, Bitwig installation, or relearning of plugin mappings is required.

## Install

Keep your working v0.3.1 release ZIP for rollback. Merge this source update into
your existing checkout, run `bash build-mac.sh`, and require both tests to pass.
Quit REAPER and replace its `UserPlugins/reaper_roto.dylib` with the newly built
`build/reaper_roto.dylib`. Reopen REAPER with the existing control-surface entry.

## Layout

In MIX mode, FUNC switches between all-tracks and single-track views. Select a
track in REAPER. On the first single-track page:

| Knob | Function |
| --- | --- |
| 1 | Selected track volume |
| 2 | Selected track pan |
| 3–8 | Send destination indices 1–6 |

Later pages show eight more send destinations each: page 2 starts at send 7,
page 3 at send 15. Destinations use the same project-wide order as all-tracks
Send mode. A selected track with no route to a listed destination cannot change
that send; the extension never creates routes. Hardware outputs are excluded.

Ordinary mixer buttons are inactive in this view, matching the Bitwig layout.
Existing Play/Stop/Record transport remains available. The stereo meter is sent
for the first screen on page 1; the other meter slots are cleared.

## Hardware checks

Use a saved test project, playback stopped, and Trim/Read initially.

1. Select a source track that sends to Delay and Reverb. Enter MIX single-track.
   Verify the selected track name/color, Level/Pan labels and send labels.
2. Turn knob 1: only the selected track's volume changes. Move its REAPER fader:
   knob 1 follows. Check the 0 dB indent. Turn knob 2 and move REAPER's pan control:
   both directions work, with center at pan zero.
3. Turn the appropriate send knobs: only that selected track's existing sends
   change. Move the corresponding REAPER send sliders and check motor feedback.
4. Select another track in REAPER. Its name/color and values should replace the
   previous track's, including when both tracks have the same name. A missing
   send route must not create a send or change either track's volume.
5. Check tracks 2, 3 and 4: their names must not replace Pan or send labels on
   the corresponding knob screen. Wait at least five seconds on each track to
   check the periodic refresh. Rename/color the selected track. Check that the header updates without
   switching modes. Play audio and check the meter if visible in this view.
6. Optional, if the project has enough destinations: move to the next control
   page. It should start at send 7, with no Level/Pan controls on that page.
   Return to page 1 and check Level/Pan again.
7. Hold Level or Pan, select another track in REAPER, then release. Target
   switching should wait for release while the original track is valid. On
   release, the name and controls should follow the newly selected track.
8. Return to all-tracks. Verify Level/Pan, banking, mute/solo/arm, Send mode and
   meters still work. Switch to PLUGIN and check a saved mapping.
9. Return to single-track and unplug/reconnect USB data. Verify the selected
   track and controls return without restarting REAPER or relearning mappings.

Retain `roto-reaper.log` from the REAPER resource folder if anything fails; it
is replaced on REAPER startup. Expect `Single-track MIX active` and
`Single-track MIX label` entries with the selected index and control page.

This preview has automated tests but needs physical controller validation.
Known physical touch dropouts and unvalidated send automation still apply.
Track selection is deferred during a reported hold; a deleted track is released
and cleared immediately. No additional track-lock feature is added in this step.
