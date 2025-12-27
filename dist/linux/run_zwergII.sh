#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

export LD_LIBRARY_PATH="$DIR/lib"
export QT_PLUGIN_PATH="$DIR/plugins"
export QT_QPA_PLATFORM_PLUGIN_PATH="$DIR/plugins/platforms"

exec "$DIR/DwarfController" "$@"
