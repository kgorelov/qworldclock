#!/bin/bash
set -e

# Ensure we are in the project root
cd "$(dirname "$0")/.."

# Build the project
cmake --build build --config Release

# Create/Clear AppDir
rm -rf AppDir
mkdir -p AppDir

# Install to AppDir
cmake --install build --prefix AppDir/usr

# Download linuxdeploy and plugins if not present
DOWNLOAD_DIR="build_tools"
mkdir -p "$DOWNLOAD_DIR"
cd "$DOWNLOAD_DIR"

download() {
    local url="$1"
    local output="$2"
    if [ ! -f "$output" ]; then
        if command -v wget >/dev/null 2>&1; then
            wget -c "$url" -O "$output"
        elif command -v curl >/dev/null 2>&1; then
            curl -L -C - "$url" -o "$output"
        else
            echo "Error: Neither wget nor curl is available" >&2
            exit 1
        fi
        chmod +x "$output"
    fi
}

download "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" "linuxdeploy-x86_64.AppImage"
download "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage" "linuxdeploy-plugin-qt-x86_64.AppImage"

cd ..

# Set up environment for linuxdeploy
if [ -z "$CONDA_PREFIX" ] && [ -d ".pixi/envs/default" ]; then
    CONDA_PREFIX="$(pwd)/.pixi/envs/default"
fi

if [ -n "$CONDA_PREFIX" ]; then
    if [ -f "$CONDA_PREFIX/bin/qmake6" ]; then
        export QMAKE="$CONDA_PREFIX/bin/qmake6"
    elif [ -f "$CONDA_PREFIX/lib/qt6/bin/qmake" ]; then
        export QMAKE="$CONDA_PREFIX/lib/qt6/bin/qmake"
    elif [ -f "$CONDA_PREFIX/bin/qmake" ]; then
        export QMAKE="$CONDA_PREFIX/bin/qmake"
    fi
    export LD_LIBRARY_PATH="$CONDA_PREFIX/lib:$LD_LIBRARY_PATH"
fi

# Pre-populate extra platform plugins if available (e.g. offscreen, wayland)
mkdir -p AppDir/usr/plugins/platforms
if [ -n "$CONDA_PREFIX" ]; then
    if [ -f "$CONDA_PREFIX/lib/qt6/plugins/platforms/libqoffscreen.so" ]; then
        cp -f "$CONDA_PREFIX/lib/qt6/plugins/platforms/libqoffscreen.so" AppDir/usr/plugins/platforms/
    fi
    if [ -f "$CONDA_PREFIX/lib/qt6/plugins/platforms/libqwayland.so" ]; then
        cp -f "$CONDA_PREFIX/lib/qt6/plugins/platforms/libqwayland.so" AppDir/usr/plugins/platforms/
    fi
fi

# Ensure custom AppRun is executable
chmod +x scripts/AppRun

# Create qt.conf in AppDir root
cat << 'EOF' > AppDir/qt.conf
[Paths]
Prefix = usr
Plugins = usr/plugins
Imports = usr/qml
Qml2Imports = usr/qml
EOF

# Run linuxdeploy with Qt plugin and AppImage output
./build_tools/linuxdeploy-x86_64.AppImage --appimage-extract-and-run \
    --appdir AppDir \
    --plugin qt \
    --custom-apprun scripts/AppRun \
    --output appimage \
    --desktop-file AppDir/usr/share/applications/qworldclock.desktop \
    --icon-file AppDir/usr/share/icons/hicolor/256x256/apps/qworldclock.png

echo "AppImage created successfully!"
