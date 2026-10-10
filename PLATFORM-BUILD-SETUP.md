# Set up the beta builds

Keep v0.2.7 published while these builds are prepared. This update does not publish releases.

1. Unzip roto-reaper-v0.3.0-platform-build-update.zip into Downloads. The resulting folder is roto-reaper-v0.3.0-platform-build-update.
2. Quit REAPER if you are running the old test binary. Merge the update into Documents/GitHub/roto-reaper. The Terminal ditto command below includes the hidden .github folder, preserves all other source/vendor files and replaces only matching files. If Safari changes the extracted folder name, adjust the first path accordingly.

   ditto ~/Downloads/roto-reaper-v0.3.0-platform-build-update ~/Documents/GitHub/roto-reaper

3. In GitHub Desktop select roto-reaper. Review the changed files, commit with "Add Windows and Intel Mac beta builds", then Push origin.
4. On the repository website choose Actions, then Build beta packages. The push starts it automatically. If there is no run, select Run workflow on main.
5. Wait for windows-x64, macos-intel and macos-arm64 jobs. All three should be green. If a job fails, open it, open the failed step and copy the error text for diagnosis.
6. In the successful run's summary, download the three artifacts. Each contains a platform-specific ZIP. Do not attach the test executables or a DLL from a failed build to a release.
7. Share the successful run link here before making the prerelease. We will verify packages, prepare release notes and invite volunteer hardware testers. Windows and Intel Mac must stay explicitly marked unvalidated.

The workflow uses standard GitHub-hosted Windows and Mac runners, read-only repository permissions, and no secrets. It builds, runs protocol and surface mocks, verifies the packaged architecture, signs Mac binaries ad hoc, and uploads artifacts. It does not notarize binaries or exercise physical MIDI hardware. Mac deployment compatibility depends on the runner SDK and should be checked with volunteers.
