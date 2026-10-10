# v0.2.6 — experimental macOS beta

Independent native Roto-Control integration for REAPER using the published Bitwig protocol. Tested on Apple Silicon macOS with REAPER 7.82 and controller firmware 3.2.1 / 3.3.0 (API 1.6).

Includes mixer banking, volume/pan, mute/solo/arm, names/colors, motor feedback, stereo meters, transport and plugin learn/recall with pages and lock. Unity alignment now uses the measured indent position; redundant unchanged motor resends after touch events are removed.

**Known issue:** touch OFF can arrive during a continuous hold. Reproduced with REAPER closed in raw MIDI capture; reported to Melbourne Instruments, cause unresolved. Touch automation holds remain unreliable. Explicit plugin automation touch reporting is not implemented.

Windows and Intel macOS are not validated. Sends, MIX single-track view and serial/haptics editing are not implemented. See README.md for installation, removal and beta checks.

If a macOS ARM64 binary is attached, use it with native Apple Silicon REAPER. GitHub's Source code downloads require building with `bash build-mac.sh` on a Mac. No firmware modification or Bitwig installation is required.
