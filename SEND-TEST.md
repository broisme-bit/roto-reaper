# v0.3.0 send-level hardware check

## Update

Copy the included src/extension.cpp, tests/surface_test.cpp, CMakeLists.txt, README.md, VALIDATION.md, RELEASE-NOTES.md and SEND-TEST.md into the matching locations in Documents/GitHub/roto-reaper. Replace those files; keep all other source/vendor files. Run bash build-mac.sh in that folder. Quit REAPER normally and replace UserPlugins/reaper_roto.dylib with build/reaper_roto.dylib. Reopen REAPER. Your existing surface entry should remain; learned plugin mappings are unchanged.

## Test with playback stopped in Trim/Read

1. Create a disposable project with tracks named Source 1, Source 2, Reverb, Delay and No Send.
2. In REAPER Routing, create Source 1 -> Delay first, then Source 1 -> Reverb. Create Source 2 -> Reverb only. Leave No Send without any sends. Keep monitoring low if using audio; playback is not required.
3. On the controller choose MIX > all tracks, select Send for the knobs and Mute for the buttons. Choose Reverb in its send selector. Expect Reverb and Delay as destination choices.
4. Turn knob 1: only Source 1's Reverb send should change. Source 1's Delay send and track fader must stay unchanged. Turn knob 2: only Source 2's Reverb send should change.
5. Change either Reverb send slider in REAPER. Its corresponding controller knob should follow. Hold the knob while changing it in REAPER: motor feedback should be suppressed while received touch is ON. Release it and it should synchronize. Known hardware touch dropouts can still affect this check.
6. Turn knob 5 (No Send): no route should be created and no track fader should change. Select Delay: knob 1 should control Source 1's Delay send; knob 2 should do nothing.
7. Buttons should still mute their source tracks. Return to Level and Pan and check ordinary mixer controls. Enter PLUGIN and check an existing learned mapping. Restart REAPER and confirm the surface remains.
8. Optional: create Source 9 at track position 9 and send it to Reverb. Bank to tracks 9-16, select Reverb and confirm knob 1 changes Source 9's Reverb send. Rename Reverb and confirm its selector name updates. Remove a route and confirm its source knob becomes inactive. Remove all sends and confirm destination choices clear.

Report which steps passed or failed and attach roto-reaper.log if the destination selector, names or controls fail. Do not publish v0.3.0 yet. Send automation recording is a later test, after basic controls pass.
