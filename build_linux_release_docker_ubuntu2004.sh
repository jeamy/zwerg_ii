#!/usr/bin/env bash
# zwergII - Linux Release Build (Docker / Ubuntu 20.04)
# Builds the Linux release in an Ubuntu 20.04 container (glibc 2.31) for maximum compatibility.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

IMAGE_NAME="zwergii-build-ubuntu2004"
DOCKERFILE_PATH="$SCRIPT_DIR/docker/ubuntu20.04/Dockerfile"

SKIP_BUILD=0
if [ "${1:-}" = "--skip-build" ]; then
  SKIP_BUILD=1
fi

if ! command -v docker &>/dev/null; then
  echo "ERROR: docker not found. Please install Docker." >&2
  exit 1
fi

if [ ! -f "$DOCKERFILE_PATH" ]; then
  echo "ERROR: Dockerfile not found: $DOCKERFILE_PATH" >&2
  exit 1
fi

echo "=== zwergII - Linux Release Build (Docker / Ubuntu 20.04) ==="

if [ "$SKIP_BUILD" -eq 0 ]; then
  echo "[1/2] Building Docker image..."
  docker build -t "$IMAGE_NAME" -f "$DOCKERFILE_PATH" "$SCRIPT_DIR"
else
  echo "[1/2] Skipping Docker image build (--skip-build)"
fi

echo "[2/2] Running release build inside container..."
USER_ID="$(id -u 2>/dev/null || echo 0)"
GROUP_ID="$(id -g 2>/dev/null || echo 0)"

docker run --rm \
  -u "$USER_ID:$GROUP_ID" \
  -v "$SCRIPT_DIR:/work" \
  -w /work \
  "$IMAGE_NAME" \
  bash -lc "rm -rf build-linux-release-docker dist/linux dist/zwergII-linux-release.zip && BUILD_DIR=/work/build-linux-release-docker BUNDLE_NON_QT_LIBS=1 BUNDLE_TOOLCHAIN_LIBS=1 bash build_linux_release.sh"

echo ""
echo "Done. Output under:"
echo "  $SCRIPT_DIR/dist/linux"
