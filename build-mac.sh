#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
if [[ $(uname -s) != Darwin ]]; then
  echo 'Run this script on your Mac.' >&2
  exit 1
fi
if ! xcrun --find clang++ >/dev/null 2>&1; then
  echo 'Install Apple Command Line Tools with: xcode-select --install' >&2
  exit 1
fi
mkdir -p build
xcrun clang++ -std=c++17 -O2 tests/protocol_test.cpp vendor/WDL/sha.cpp -o build/protocol_test
./build/protocol_test
xcrun clang++ -std=c++17 -O2 -Wl,-undefined,dynamic_lookup tests/surface_test.cpp vendor/WDL/sha.cpp -o build/surface_test
./build/surface_test
xcrun clang++ -std=c++17 -O2 -fPIC -dynamiclib -Wl,-undefined,dynamic_lookup src/extension.cpp vendor/WDL/sha.cpp -o build/reaper_roto.dylib
codesign --force --sign - build/reaper_roto.dylib
echo 'Built build/reaper_roto.dylib for this Mac. See README.md to install.'
