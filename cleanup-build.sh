#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

targets=(
  "$repo_root/CMakeCache.txt"
  "$repo_root/CMakeFiles"
  "$repo_root/Makefile"
  "$repo_root/cmake_install.cmake"
  "$repo_root/build"
  "$repo_root/thirdparty/jolt/Makefile"
  "$repo_root/thirdparty/jolt/cmake_install.cmake"
  "$repo_root/thirdparty/jolt/CMakeFiles"
  "$repo_root/thirdparty/jolt/libJolt.a"
  "$repo_root/thirdparty/jolt/JoltPhysics/Build/CMakeCache.txt"
  "$repo_root/thirdparty/jolt/JoltPhysics/Build/CMakeFiles"
  "$repo_root/thirdparty/jolt/JoltPhysics/Build/Makefile"
  "$repo_root/thirdparty/jolt/JoltPhysics/Build/cmake_install.cmake"
  "$repo_root/thirdparty/jolt/JoltPhysics/Build/libJolt.a"
)

for target in "${targets[@]}"; do
  if [[ -e "$target" ]]; then
    rm -rf "$target"
  fi
done

echo "Removed generated build artifacts."