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
    echo "[zwergii] Clean build: Lösche komplettes Build-Verzeichnis und baue neu (Debug)."
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    echo "[zwergii] CMake-Konfiguration (Debug)..."
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    echo "[zwergii] Baue Projekt..."
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
