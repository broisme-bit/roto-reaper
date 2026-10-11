# Automatic reconnect — v0.3.1 preview

This preview keeps the existing ROTOREAPER surface identity, configuration token,
plugin mapping identities, and reaper_roto binary filename. Saved mappings do not
need to be learned again. Do not publish this preview until hardware checks pass.

## Install on your Apple Silicon Mac

1. Keep a copy of your working v0.3.0 binary or release ZIP for rollback.
2. Merge the update ZIP into your existing source folder. Do not delete the folder.
3. In Terminal run `cd ~/Documents/GitHub/roto-reaper`, then `bash build-mac.sh`.
4. Both tests must pass. In REAPER use Options > Show REAPER resource path in
   explorer/finder, then quit REAPER normally.
5. Copy `build/reaper_roto.dylib` into that resource folder's `UserPlugins`, replacing
   the existing file. Keep the filename exactly `reaper_roto.dylib`.
6. Reopen REAPER. Keep the existing control-surface entry; do not add another.

## Hardware checks

Save your project first. Test with playback stopped initially. Disconnect the
USB data cable, keeping power connected where possible. A power-cycle test is
separate because the controller may return to a different page or mode itself.

1. MIX Level: confirm controls, names/colors and meters. Disconnect USB data for
   10 seconds; reconnect. Without restarting REAPER or re-adding the surface,
   confirm the controller reconnects, refreshes, and controls the right tracks.
   It may take several seconds. If it resumes in another mode, note that behavior.
2. Repeat in Pan, then Send. Verify each controls the correct target and Send
   destination names return. Check bank navigation after reconnecting.
3. PLUGIN: open a plugin with a saved mapping. Disconnect/reconnect; verify its
   track/plugin names, learned controls, buttons and parameter feedback return.
   No relearning should be required. Try a different mapped page too.
4. Startup absent: quit REAPER, disconnect the controller's data cable, reopen
   REAPER, then reconnect. The saved surface should remain and connect itself.
5. Leave it connected and idle for at least a minute. There should be no repeated
   disconnects, screen resets, or loss of control. Version replies in the log are
   expected from the connection checks.
6. Repeat unplug/replug three times. Also test a full controller power cycle.
7. With playback stopped, hold a mixer knob and unplug USB data, then release it.
   Reconnect and verify motor feedback resumes rather than remaining suppressed.

## Log and limitations

`roto-reaper.log` is in the REAPER resource folder and is replaced at each REAPER
startup. Save it before restarting if a check fails. Expect `Ports opened`,
`Ping received`, and `ROTO-DAW CONNECTED` again after reconnecting. Port loss may
log `MIDI ports changed or unavailable`; stale handles may log
`Controller response timed out` before reopening.

The extension checks enumerated port names/indices every 0.5 seconds. Connected
devices are queried for their SysEx API version every 2 seconds; a missing reply
for 5 seconds triggers port reopening. Handshakes that do not finish within
8 seconds also trigger reopening. Missing or ambiguous ports are never selected
arbitrarily, and partial opens are cleaned up before retrying.

Recovery still depends on REAPER/OS making MIDI devices available. If reopening
fails, retain the log and describe whether REAPER's MIDI-device reset or a REAPER
restart restores it. This preview does not change physical touch sensitivity or
fix the independently observed touch dropouts. Windows/Intel Mac need hardware
testing separately.
