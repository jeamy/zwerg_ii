#!/usr/bin/env bash
# zwergII - macOS Release Build
# Builds a Release app bundle and creates a dist folder.
# Uses macdeployqt if available.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_DIR/build-macos-release"
DIST_DIR="$PROJECT_DIR/dist/macos"
BUILD_TYPE="Release"
DMG_NAME="zwergII-macos.dmg"

APP_NAME="DwarfController"
APP_BUNDLE_NAME="$APP_NAME.app"

echo "=== zwergII - macOS Release Build ==="

# ==============================================================================
# Dependency checks (no auto-install)
# ==============================================================================
MISSING_DEPS=""

if ! command -v clang++ &>/dev/null; then
  MISSING_DEPS="$MISSING_DEPS clang++"
fi

if ! command -v cmake &>/dev/null; then
  MISSING_DEPS="$MISSING_DEPS cmake"
fi

QT6_OK=0
if command -v qmake6 &>/dev/null; then
  QT6_OK=1
elif command -v qtpaths6 &>/dev/null; then
  QT6_OK=1
fi
if [ "$QT6_OK" -eq 0 ]; then
  MISSING_DEPS="$MISSING_DEPS qt6"
fi

if ! command -v protoc &>/dev/null; then
  MISSING_DEPS="$MISSING_DEPS protoc"
fi

if [ -n "$MISSING_DEPS" ]; then
  echo "ERROR: Missing dependencies:$MISSING_DEPS" >&2
  echo "Please install them and retry." >&2
  exit 1
fi

echo "All dependencies found."

# Help CMake find Qt if possible
if [ -z "$CMAKE_PREFIX_PATH" ] && command -v qtpaths6 &>/dev/null; then
  QT_PREFIX_DETECTED="$(qtpaths6 --install-prefix 2>/dev/null || echo "")"
  if [ -n "$QT_PREFIX_DETECTED" ]; then
    export CMAKE_PREFIX_PATH="$QT_PREFIX_DETECTED"
  fi
fi

if [ -n "$CMAKE_PREFIX_PATH" ] && [ -d "$CMAKE_PREFIX_PATH/bin" ]; then
  export PATH="$CMAKE_PREFIX_PATH/bin:$PATH"
fi

if [ -z "$Qt6_DIR" ] && [ -n "$CMAKE_PREFIX_PATH" ] && [ -d "$CMAKE_PREFIX_PATH/lib/cmake/Qt6" ]; then
  export Qt6_DIR="$CMAKE_PREFIX_PATH/lib/cmake/Qt6"
fi

# ==============================================================================
echo "[1/3] Configure (CMake)..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# ==============================================================================
echo "[2/3] Build..."
NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
cmake --build "$BUILD_DIR" -j"$NPROC" --config "$BUILD_TYPE"

# ==============================================================================
echo "[3/3] Create dist directory..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Locate app bundle
APP_SRC=""
if [ -d "$BUILD_DIR/$APP_BUNDLE_NAME" ]; then
  APP_SRC="$BUILD_DIR/$APP_BUNDLE_NAME"
elif [ -d "$BUILD_DIR/$BUILD_TYPE/$APP_BUNDLE_NAME" ]; then
  APP_SRC="$BUILD_DIR/$BUILD_TYPE/$APP_BUNDLE_NAME"
fi

if [ -z "$APP_SRC" ]; then
  echo "ERROR: App bundle not found: $APP_BUNDLE_NAME" >&2
  exit 1
fi

cp -R "$APP_SRC" "$DIST_DIR/"
APP_BUNDLE="$DIST_DIR/$APP_BUNDLE_NAME"

# Copy runtime assets next to the executable (applicationDirPath() points to Contents/MacOS)
mkdir -p "$APP_BUNDLE/Contents/MacOS/styles" \
         "$APP_BUNDLE/Contents/MacOS/i18n" \
         "$APP_BUNDLE/Contents/MacOS/data"

if [ -f "$PROJECT_DIR/styles/app.qss" ]; then
  cp "$PROJECT_DIR/styles/app.qss" "$APP_BUNDLE/Contents/MacOS/styles/"
fi

for qm in "$PROJECT_DIR/i18n/"*.qm; do
  [ -f "$qm" ] && cp "$qm" "$APP_BUNDLE/Contents/MacOS/i18n/" || true
done

for f in stars.db constellationship.fab; do
  if [ -f "$PROJECT_DIR/data/$f" ]; then
    cp "$PROJECT_DIR/data/$f" "$APP_BUNDLE/Contents/MacOS/data/"
  fi
done

if [ -d "$PROJECT_DIR/resources" ]; then
  mkdir -p "$APP_BUNDLE/Contents/MacOS/resources"
  cp -R "$PROJECT_DIR/resources/." "$APP_BUNDLE/Contents/MacOS/resources/"
fi

# Bundle Qt
if command -v macdeployqt &>/dev/null; then
  echo "Bundling Qt frameworks with macdeployqt..."
  macdeployqt "$APP_BUNDLE" -verbose=1 || {
    echo "WARNING: macdeployqt failed." >&2
  }
else
  echo "NOTE: macdeployqt not found - Qt frameworks will NOT be bundled." >&2
fi

# Optional DMG
if command -v hdiutil &>/dev/null; then
  DMG_PATH="$PROJECT_DIR/dist/$DMG_NAME"
  echo "Creating DMG at: $DMG_PATH"
  rm -f "$DMG_PATH"
  hdiutil create -volname "zwergII" -srcfolder "$APP_BUNDLE" -ov -format UDZO "$DMG_PATH" || {
    echo "WARNING: failed to create DMG" >&2
  }
fi

echo ""
echo "========================================"
echo "  Release build finished"
echo "========================================"
echo ""
echo "Output:"
echo "  $DIST_DIR/$APP_BUNDLE_NAME"
