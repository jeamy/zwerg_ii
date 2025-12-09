#!/usr/bin/env bash
set -e

# Einfaches Build-Script für zwergii
# Modi:
#   ./build.sh           -> Debug-Build (Default)
#   ./build.sh clean     -> Build-Verzeichnis löschen und neu konfigurieren + bauen
#   ./build.sh release   -> Release-Build (CMAKE_BUILD_TYPE=Release)

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$ROOT_DIR/build"

MODE="${1:-debug}"

mkdir -p "$BUILD_DIR"

case "$MODE" in
  clean)
    echo "[zwergii] Clean build: führe 'cmake --build . --target clean' aus und baue danach (Debug)."
    cd "$BUILD_DIR"
    if [ ! -f "CMakeCache.txt" ]; then
      echo "[zwergii] Build-Verzeichnis noch nicht konfiguriert, führe CMake-Konfiguration (Debug) aus."
      cmake -DCMAKE_BUILD_TYPE=Debug ..
    fi
    cmake --build . --target clean
    cmake --build .
    ;;

  release)
    echo "[zwergii] Release-Build (CMAKE_BUILD_TYPE=Release)."
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --config Release
    ;;

  debug|*)
    echo "[zwergii] Debug-Build (CMAKE_BUILD_TYPE=Debug)."
    cd "$BUILD_DIR"
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    cmake --build .
    ;;

esac
