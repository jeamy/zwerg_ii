#!/usr/bin/env bash
# zwergII - Linux Release Build
# Builds a Release binary and creates a minimal dist folder.
# Optional: bundle Qt runtime using linuxdeployqt (if available) or qtpaths6+patchelf.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_DIR/build-linux-release"
DIST_DIR="$PROJECT_DIR/dist/linux"
BUILD_TYPE="Release"
ZIP_NAME="zwergII-linux-release.zip"

BIN_NAME="DwarfController"

echo "=== zwergII - Linux Release Build ==="

# ==============================================================================
# Dependency checks (no auto-install)
# ==============================================================================
MISSING_DEPS=""

if ! command -v cmake &>/dev/null; then
  MISSING_DEPS="$MISSING_DEPS cmake"
fi

if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
  MISSING_DEPS="$MISSING_DEPS compiler"
fi

QT6_OK=0
if command -v qmake6 &>/dev/null; then
  QT6_OK=1
elif pkg-config --exists Qt6Core 2>/dev/null; then
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
  echo "" >&2
  echo "Please install the missing tools/libraries and retry." >&2
  echo "Common packages:" >&2
  echo "- Debian/Ubuntu: cmake build-essential qt6-base-dev qt6-multimedia-dev qt6-websockets-dev qt6-tools-dev libqt6sql6-sqlite protobuf-compiler libprotobuf-dev" >&2
  echo "- Fedora: dnf install cmake gcc-c++ qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qtwebsockets-devel qt6-qtsql-devel protobuf-compiler protobuf-devel" >&2
  exit 1
fi

echo "All dependencies found."

# ==============================================================================
echo "[1/3] Configure (CMake)..."
cmake -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# ==============================================================================
echo "[2/3] Build..."
NPROC=$(nproc 2>/dev/null || echo 4)
cmake --build "$BUILD_DIR" -j"$NPROC"

# ==============================================================================
echo "[3/3] Create dist directory..."
rm -rf "$DIST_DIR"
mkdir -p "$DIST_DIR"

# Copy binary
if [ -f "$BUILD_DIR/$BIN_NAME" ]; then
  cp "$BUILD_DIR/$BIN_NAME" "$DIST_DIR/"
elif [ -f "$BUILD_DIR/$BUILD_TYPE/$BIN_NAME" ]; then
  cp "$BUILD_DIR/$BUILD_TYPE/$BIN_NAME" "$DIST_DIR/"
else
  echo "ERROR: Binary not found: $BIN_NAME" >&2
  exit 1
fi

# Required runtime assets
mkdir -p "$DIST_DIR/styles" "$DIST_DIR/i18n" "$DIST_DIR/data"

if [ -f "$PROJECT_DIR/styles/app.qss" ]; then
  cp "$PROJECT_DIR/styles/app.qss" "$DIST_DIR/styles/"
fi

for qm in "$PROJECT_DIR/i18n/"*.qm; do
  [ -f "$qm" ] && cp "$qm" "$DIST_DIR/i18n/" || true
done

# Star catalog (used by AstroNavigationPanel/StarMapWidget)
for f in stars.db constellationship.fab; do
  if [ -f "$PROJECT_DIR/data/$f" ]; then
    cp "$PROJECT_DIR/data/$f" "$DIST_DIR/data/"
  fi
done

# Optional resources (icons etc.)
if [ -d "$PROJECT_DIR/resources" ]; then
  mkdir -p "$DIST_DIR/resources"
  cp -r "$PROJECT_DIR/resources/." "$DIST_DIR/resources/"
fi

# ==============================================================================
# Bundle Qt runtime (optional)
# ==============================================================================
if command -v linuxdeployqt &>/dev/null; then
  echo "Bundling Qt libs with linuxdeployqt..."
  DESKTOP_FILE="$DIST_DIR/$BIN_NAME.desktop"
  cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=zwergII
Exec=$BIN_NAME
Categories=Education;
EOF

  linuxdeployqt "$DESKTOP_FILE" -bundle-non-qt-libs -executable="$DIST_DIR/$BIN_NAME" || {
    echo "WARNING: linuxdeployqt failed to bundle Qt libs." >&2
  }
elif (command -v qtpaths6 &>/dev/null || command -v qtpaths &>/dev/null) && command -v patchelf &>/dev/null; then
  echo "Bundling Qt libs manually (qtpaths + patchelf fallback)..."

  BUNDLE_NON_QT_LIBS="${BUNDLE_NON_QT_LIBS:-0}"
  BUNDLE_TOOLCHAIN_LIBS="${BUNDLE_TOOLCHAIN_LIBS:-0}"

  QT_PATHS_CMD="qtpaths6"
  if ! command -v qtpaths6 &>/dev/null; then
    QT_PATHS_CMD="qtpaths"
  fi

  QT_PLUGIN_DIR="$("$QT_PATHS_CMD" --plugin-dir 2>/dev/null || true)"
  QT_PREFIX="$("$QT_PATHS_CMD" --install-prefix 2>/dev/null || true)"
  QT_LIB_DIR="$("$QT_PATHS_CMD" --library-dir 2>/dev/null || true)"

  if [ -z "$QT_PLUGIN_DIR" ] && command -v qmake6 &>/dev/null; then
    QT_PLUGIN_DIR="$(qmake6 -query QT_INSTALL_PLUGINS 2>/dev/null || true)"
  fi
  if [ -z "$QT_LIB_DIR" ] && command -v qmake6 &>/dev/null; then
    QT_LIB_DIR="$(qmake6 -query QT_INSTALL_LIBS 2>/dev/null || true)"
  fi
  if [ -z "$QT_PREFIX" ] && command -v qmake6 &>/dev/null; then
    QT_PREFIX="$(qmake6 -query QT_INSTALL_PREFIX 2>/dev/null || true)"
  fi
  if [ -z "$QT_LIB_DIR" ] && [ -n "$QT_PREFIX" ]; then
    QT_LIB_DIR="$QT_PREFIX/lib"
  fi

  mkdir -p "$DIST_DIR/lib" "$DIST_DIR/plugins"

  copy_lib() {
    local src="$1"
    [ -z "$src" ] && return 0
    [ ! -e "$src" ] && return 0

    local base
    base="$(basename "$src")"

    if [ ! -e "$DIST_DIR/lib/$base" ]; then
      cp -P "$src" "$DIST_DIR/lib/" 2>/dev/null || true
    fi

    if [ -L "$src" ]; then
      local real
      real="$(readlink -f "$src" 2>/dev/null || true)"
      if [ -n "$real" ] && [ -f "$real" ]; then
        local real_base
        real_base="$(basename "$real")"
        if [ ! -e "$DIST_DIR/lib/$real_base" ]; then
          cp -P "$real" "$DIST_DIR/lib/" 2>/dev/null || true
        fi
      fi
    fi
  }

  copy_deps() {
    local target="$1"
    [ -z "$target" ] && return 0
    [ ! -e "$target" ] && return 0

    while IFS= read -r dep; do
      [ -z "$dep" ] && continue
      local base
      base="$(basename "$dep")"
      case "$base" in
        linux-vdso.so.*|ld-linux*.so*|libc.so.*|libm.so.*|libdl.so.*|libpthread.so.*|librt.so.*)
          continue
          ;;
      esac

      if [ "$BUNDLE_TOOLCHAIN_LIBS" != "1" ]; then
        case "$base" in
          libstdc++.so.*|libgcc_s.so.*)
            continue
            ;;
        esac
      fi

      if [[ "$BUNDLE_NON_QT_LIBS" != "1" ]]; then
        # Special case: always bundle libprotobuf (needed at runtime even with minimal bundling)
        if [[ "$dep" == *"libprotobuf"* ]]; then
          echo "  → Bundling required non-Qt lib: $dep"
        else
          echo "  → Skipping non-Qt lib: $dep"
          continue
        fi
      fi

      # Exclude host-incompatible X11/XCB/XKB libs to prevent ABI mix (causes segfault)
      case "$dep" in
        *libxkbcommon-x11*|*libxcb-xkb*|*libxcb-icccm*|*libxcb-image*|*libxcb-keysyms*|*libxcb-randr*|*libxcb-render*|*libxcb-render-util*|*libxcb-shape*|*libxcb-shm*|*libxcb-sync*|*libxcb-xfixes*|*libxcb-xinerama*|*libxcb-cursor*)
          echo "  → Skipping host-incompatible X11/XCB lib: $dep"
          continue
          ;;
      esac

      if [ ! -f "$DIST_DIR/lib/$base" ]; then
        copy_lib "$dep"
      fi
    done < <(ldd "$target" 2>/dev/null | awk '/=>/ {print $(NF-1)}' | grep -E '^/' || true)
  }

  # Copy Qt libs detected from the binary
  for lib in $(ldd "$DIST_DIR/$BIN_NAME" | awk '/Qt6/ {print $3}'); do
    [ -e "$lib" ] && copy_lib "$lib"
  done

  # Ensure core Qt libs exist
  for base in libQt6Core.so libQt6Gui.so libQt6Widgets.so libQt6Network.so libQt6WebSockets.so libQt6Multimedia.so libQt6MultimediaWidgets.so libQt6Sql.so libQt6DBus.so libQt6OpenGL.so libQt6XcbQpa.so; do
    if [ -e "$QT_LIB_DIR/$base" ]; then
      copy_lib "$QT_LIB_DIR/$base"
    fi
    if ls "$QT_LIB_DIR/${base}."* >/dev/null 2>&1; then
      for cand in "$QT_LIB_DIR/${base}."*; do
        [ -e "$cand" ] && copy_lib "$cand"
      done
    fi
  done

  # Minimal non-Qt runtime libs required by the app.
  # We do NOT auto-bundle the full system stack by default (see BUNDLE_NON_QT_LIBS),
  # but protobuf is required for startup on many distros.
  for lib in $(ldd "$DIST_DIR/$BIN_NAME" 2>/dev/null | awk '/libprotobuf\.so/ {print $3}'); do
    [ -e "$lib" ] && copy_lib "$lib"
  done

  # Dependencies der Qt-Libs einsammeln
  if [ "$BUNDLE_NON_QT_LIBS" = "1" ]; then
    if ls "$DIST_DIR/lib/"libQt6*.so* >/dev/null 2>&1; then
      for lib in "$DIST_DIR/lib/"libQt6*.so*; do
        [ -f "$lib" ] && copy_deps "$lib"
      done
    fi

    # Also collect deps for any other bundled libs (e.g. libxcb*, libicu*, etc.)
    if ls "$DIST_DIR/lib/"*.so* >/dev/null 2>&1; then
      for lib in "$DIST_DIR/lib/"*.so*; do
        [ -f "$lib" ] && copy_deps "$lib"
      done
    fi

    # Non-Qt deps
    copy_deps "$DIST_DIR/$BIN_NAME"
  else
    echo "NOTE: BUNDLE_NON_QT_LIBS=0 - not bundling system libraries (X11/XCB/XKB/GL/etc.)." >&2
    echo "      This avoids ABI mixing crashes. Ensure required runtime packages are installed on the target system." >&2
  fi

  # Plugins
  for sub in platforms styles imageformats xcbglintegrations iconengines sqldrivers; do
    if [ -n "$QT_PLUGIN_DIR" ] && [ -d "$QT_PLUGIN_DIR/$sub" ]; then
      mkdir -p "$DIST_DIR/plugins/$sub"
      cp -L "$QT_PLUGIN_DIR/$sub"/*.so "$DIST_DIR/plugins/$sub"/ 2>/dev/null || true
    fi
  done

  if [ "$BUNDLE_NON_QT_LIBS" = "1" ] && [ -d "$DIST_DIR/plugins" ]; then
    while IFS= read -r sofile; do
      copy_deps "$sofile"
    done < <(find "$DIST_DIR/plugins" -type f -name "*.so" 2>/dev/null)
  fi

  cat > "$DIST_DIR/qt.conf" <<EOF
[Paths]
Prefix=.
Plugins=plugins
Imports=imports
Qml2Imports=qml
EOF

  patchelf --set-rpath '$ORIGIN/lib' "$DIST_DIR/$BIN_NAME" || true
  for lib in "$DIST_DIR/lib/"*.so*; do
    patchelf --set-rpath '$ORIGIN' "$lib" || true
  done

  # Plugins live under plugins/<type>/*.so, so they must find libs in ../../lib.
  if [ -d "$DIST_DIR/plugins" ]; then
    while IFS= read -r sofile; do
      [ -f "$sofile" ] && patchelf --set-rpath '$ORIGIN/../../lib' "$sofile" || true
    done < <(find "$DIST_DIR/plugins" -type f -name "*.so" 2>/dev/null)
  fi
else
  echo "NOTE: No linuxdeployqt and no (qtpaths+patchelf) found - Qt libs will NOT be bundled." >&2
  echo "      Install linuxdeployqt or qtpaths6+patchelf if you want a portable dist." >&2
fi

# Launcher
cat > "$DIST_DIR/run_zwergII.sh" <<'EOF'
#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export LD_LIBRARY_PATH="$DIR/lib"
export QT_PLUGIN_PATH="$DIR/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$DIR/plugins/platforms"

exec "$DIR/DwarfController" "$@"
EOF
chmod +x "$DIST_DIR/run_zwergII.sh" 2>/dev/null || true

# Zip (optional)
if command -v zip &>/dev/null; then
  echo "Creating release zip: $ZIP_NAME"
  (
    cd "$PROJECT_DIR/dist" || exit 1
    rm -f "$ZIP_NAME"
    zip -r "$ZIP_NAME" "linux" >/dev/null
  )
  echo "Release zip created: $PROJECT_DIR/dist/$ZIP_NAME"
else
  echo "NOTE: 'zip' not found - no zip created." >&2
fi

echo ""
echo "========================================"
echo "  Release build finished"
echo "========================================"
echo ""
echo "Run:"
echo "  cd $DIST_DIR"
echo "  ./run_zwergII.sh"
