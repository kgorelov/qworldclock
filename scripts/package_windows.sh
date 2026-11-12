#!/bin/bash
set -e

# Ensure we are in the project root
cd "$(dirname "$0")/.."
PROJECT_ROOT="$(pwd)"

mkdir -p dist package/qworldclock

# Resolve default version from CMakeLists.txt
DEFAULT_VERSION=$(grep -m1 -E 'project\s*\([^)]*VERSION' "$PROJECT_ROOT/CMakeLists.txt" 2>/dev/null | sed -E 's/.*VERSION\s+([0-9.]+).*/\1/' || true)
DEFAULT_VERSION="${DEFAULT_VERSION:-0.1.0}"

# Resolve release tag and version
TAG_NAME="${TAG_NAME:-${GITHUB_REF_NAME:-v$DEFAULT_VERSION}}"
TAG_NAME="${TAG_NAME#refs/tags/}"
TAG_NAME="${TAG_NAME#refs/heads/}"
TAG_NAME="${TAG_NAME//\//-}"

# Extract version starting with digit if available, else fallback to CMakeLists version
EXTRACTED_VER=$(echo "$TAG_NAME" | grep -oE '[0-9]+\.[0-9]+(\.[0-9]+)?' | head -n1 || true)
VERSION="${VERSION:-$EXTRACTED_VER}"
VERSION="${VERSION:-$DEFAULT_VERSION}"
if [[ ! "$VERSION" =~ ^[0-9] ]]; then
  VERSION="$DEFAULT_VERSION"
fi
VERSION="${VERSION%%-*}"

# Format 3-part version for WiX (major.minor.build)
IFS='.' read -r -a VER_PARTS <<< "$VERSION"
while [ ${#VER_PARTS[@]} -lt 3 ]; do
  VER_PARTS+=("0")
done
WIX_VERSION="${VER_PARTS[0]}.${VER_PARTS[1]}.${VER_PARTS[2]}"

# 1. Normalize and resolve CONDA_PREFIX
if [ -n "$CONDA_PREFIX" ]; then
  CONDA_PREFIX="${CONDA_PREFIX//\\//}"
fi

if [ -z "$CONDA_PREFIX" ] && [ -d ".pixi/envs/default" ]; then
  CONDA_PREFIX="$(pwd)/.pixi/envs/default"
fi

# Locate PREFIX_BIN and PREFIX_PLUGINS
PREFIX_BIN=""
PREFIX_PLUGINS=""
if [ -n "$CONDA_PREFIX" ]; then
  if [ -d "$CONDA_PREFIX/Library/bin" ]; then
    PREFIX_BIN="$CONDA_PREFIX/Library/bin"
    PREFIX_PLUGINS="$CONDA_PREFIX/Library/plugins"
  elif [ -d "$CONDA_PREFIX/bin" ]; then
    PREFIX_BIN="$CONDA_PREFIX/bin"
    PREFIX_PLUGINS="$CONDA_PREFIX/plugins"
  fi
fi

echo "Packaging Windows distribution:"
echo "  CONDA_PREFIX: $CONDA_PREFIX"
echo "  PREFIX_BIN: $PREFIX_BIN"
echo "  PREFIX_PLUGINS: $PREFIX_PLUGINS"

# 2. Copy application binary
if [ -f "build/Release/qworldclock.exe" ]; then
  cp build/Release/qworldclock.exe package/qworldclock/
elif [ -f "build/qworldclock.exe" ]; then
  cp build/qworldclock.exe package/qworldclock/
else
  echo "Error: qworldclock.exe not found in build directory" >&2
  exit 1
fi

# 3. Copy any runtime DLLs deployed to build
cp build/*.dll package/qworldclock/ 2>/dev/null || true
if [ -d "build/Release" ]; then
  cp build/Release/*.dll package/qworldclock/ 2>/dev/null || true
fi

# 4. Copy Qt and dependency DLLs from Conda prefix if available
if [ -n "$PREFIX_BIN" ] && [ -d "$PREFIX_BIN" ]; then
  cp "$PREFIX_BIN"/*.dll package/qworldclock/ 2>/dev/null || true
  cp "$PREFIX_BIN"/Qt6*.dll package/qworldclock/ 2>/dev/null || true
  cp "$PREFIX_BIN"/icu*.dll package/qworldclock/ 2>/dev/null || true
  cp "$PREFIX_BIN"/libpng*.dll "$PREFIX_BIN"/zlib*.dll "$PREFIX_BIN"/pcre2*.dll "$PREFIX_BIN"/zstd*.dll "$PREFIX_BIN"/freetype*.dll "$PREFIX_BIN"/harfbuzz*.dll "$PREFIX_BIN"/libbz2*.dll "$PREFIX_BIN"/bzip2*.dll "$PREFIX_BIN"/double-conversion*.dll package/qworldclock/ 2>/dev/null || true
  cp "$PREFIX_BIN"/d3dcompiler*.dll "$PREFIX_BIN"/opengl32sw.dll "$PREFIX_BIN"/libEGL*.dll "$PREFIX_BIN"/libGLESv2*.dll package/qworldclock/ 2>/dev/null || true
  cp "$PREFIX_BIN"/vcruntime*.dll "$PREFIX_BIN"/msvcp*.dll "$PREFIX_BIN"/vcomp*.dll package/qworldclock/ 2>/dev/null || true
fi

if [ -n "$CONDA_PREFIX" ] && [ -d "$CONDA_PREFIX/Library/lib/qt6/bin" ]; then
  cp "$CONDA_PREFIX/Library/lib/qt6/bin"/*.dll package/qworldclock/ 2>/dev/null || true
fi

if [ -n "$CONDA_PREFIX" ] && [ -d "$CONDA_PREFIX/bin" ]; then
  cp "$CONDA_PREFIX"/bin/*.dll package/qworldclock/ 2>/dev/null || true
fi

# 5. Copy MSVC runtime from system if available
cp /c/Windows/System32/vcruntime140*.dll /c/Windows/System32/msvcp140*.dll package/qworldclock/ 2>/dev/null || true

# 6. Copy Qt plugins from build directory
mkdir -p package/qworldclock/plugins/platforms package/qworldclock/platforms
for plugin_type in platforms imageformats iconengines styles; do
  if [ -d "build/$plugin_type" ]; then
    echo "Copying $plugin_type from build..."
    mkdir -p "package/qworldclock/plugins/$plugin_type" "package/qworldclock/$plugin_type"
    cp -r "build/$plugin_type"/* "package/qworldclock/plugins/$plugin_type/" 2>/dev/null || true
    cp -r "build/$plugin_type"/* "package/qworldclock/$plugin_type/" 2>/dev/null || true
  elif [ -d "build/Release/$plugin_type" ]; then
    echo "Copying $plugin_type from build/Release..."
    mkdir -p "package/qworldclock/plugins/$plugin_type" "package/qworldclock/$plugin_type"
    cp -r "build/Release/$plugin_type"/* "package/qworldclock/plugins/$plugin_type/" 2>/dev/null || true
    cp -r "build/Release/$plugin_type"/* "package/qworldclock/$plugin_type/" 2>/dev/null || true
  fi
done

# 7. Copy Qt plugins from PREFIX_PLUGINS or qt6 plugins directory
for pdir in "$PREFIX_PLUGINS" "$CONDA_PREFIX/Library/lib/qt6/plugins" "$CONDA_PREFIX/lib/qt6/plugins"; do
  if [ -n "$pdir" ] && [ -d "$pdir" ]; then
    for plugin_type in platforms imageformats iconengines styles; do
      if [ -d "$pdir/$plugin_type" ]; then
        echo "Copying $plugin_type from $pdir..."
        mkdir -p "package/qworldclock/plugins/$plugin_type" "package/qworldclock/$plugin_type"
        cp "$pdir/$plugin_type"/*.dll "package/qworldclock/plugins/$plugin_type/" 2>/dev/null || true
        cp "$pdir/$plugin_type"/*.dll "package/qworldclock/$plugin_type/" 2>/dev/null || true
      fi
    done
  fi
done

# 8. Fallback search for qwindows.dll if still missing
if [ ! -f "package/qworldclock/plugins/platforms/qwindows.dll" ] && [ ! -f "package/qworldclock/platforms/qwindows.dll" ]; then
  echo "Warning: qwindows.dll not yet in package, performing search..."
  FOUND_QWIN=""
  if [ -n "$CONDA_PREFIX" ] && [ -d "$CONDA_PREFIX" ]; then
    FOUND_QWIN=$(find "$CONDA_PREFIX" -name "qwindows.dll" 2>/dev/null | head -n 1)
  fi
  if [ -z "$FOUND_QWIN" ]; then
    FOUND_QWIN=$(find build -name "qwindows.dll" 2>/dev/null | head -n 1)
  fi
  if [ -n "$FOUND_QWIN" ]; then
    echo "Found qwindows.dll at: $FOUND_QWIN"
    mkdir -p package/qworldclock/plugins/platforms package/qworldclock/platforms
    cp "$FOUND_QWIN" package/qworldclock/plugins/platforms/
    cp "$FOUND_QWIN" package/qworldclock/platforms/
  fi
fi

# 9. Create qt.conf so Qt discovers its plugins relative to application directory
cat << 'EOF' > package/qworldclock/qt.conf
[Paths]
Prefix = .
Plugins = plugins
EOF

# 10. Verification: Fail loudly if qwindows.dll is missing
echo "Verifying package contents:"
ls -la package/qworldclock/
ls -la package/qworldclock/platforms/ 2>/dev/null || true
ls -la package/qworldclock/plugins/platforms/ 2>/dev/null || true

if [ ! -f "package/qworldclock/plugins/platforms/qwindows.dll" ] && [ ! -f "package/qworldclock/platforms/qwindows.dll" ]; then
  echo "FATAL ERROR: qwindows.dll was NOT found in package!" >&2
  exit 1
fi
echo "Verification passed: qwindows.dll is present."

# 11. Compress portable zip
cmake -E chdir package cmake -E tar cfv "../dist/qworldclock-${TAG_NAME}-windows-x64.zip" --format=zip qworldclock
echo "Windows portable zip created: dist/qworldclock-${TAG_NAME}-windows-x64.zip"

# 12. Compile Inno Setup EXE Installer
echo ""
echo ">>> Checking for Inno Setup compiler..."
ISCC_BIN=""
if command -v iscc >/dev/null 2>&1; then
  ISCC_BIN="$(command -v iscc)"
elif command -v iscc.exe >/dev/null 2>&1; then
  ISCC_BIN="$(command -v iscc.exe)"
elif [ -f "/c/Program Files (x86)/Inno Setup 6/ISCC.exe" ]; then
  ISCC_BIN="/c/Program Files (x86)/Inno Setup 6/ISCC.exe"
elif [ -f "/c/Program Files/Inno Setup 6/ISCC.exe" ]; then
  ISCC_BIN="/c/Program Files/Inno Setup 6/ISCC.exe"
fi

if [ -n "$ISCC_BIN" ]; then
  echo "Compiling Inno Setup installer using $ISCC_BIN..."

  if command -v cygpath >/dev/null 2>&1; then
    ISS_FILE_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/qworldclock.iss")"
  else
    ISS_FILE_WIN="$PROJECT_ROOT\\packaging\\windows\\qworldclock.iss"
  fi

  MSYS2_ARG_CONV_EXCL="*" MSYS_NO_PATHCONV=1 "$ISCC_BIN" \
    -DMyAppVersion="$VERSION" \
    -DOutputBaseFilename="qworldclock-${TAG_NAME}-windows-x64-setup" \
    "$ISS_FILE_WIN"

  echo "Windows setup installer created: dist/qworldclock-${TAG_NAME}-windows-x64-setup.exe"
else
  echo "Inno Setup (iscc) not found; skipping Windows EXE installer."
fi

# 13. Compile WiX Toolset MSI Installer
echo ""
echo ">>> Checking for WiX Toolset..."
WIX_HEAT=""
WIX_CANDLE=""
WIX_LIGHT=""
if command -v heat >/dev/null 2>&1 && command -v candle >/dev/null 2>&1 && command -v light >/dev/null 2>&1; then
  WIX_HEAT="heat"
  WIX_CANDLE="candle"
  WIX_LIGHT="light"
elif [ -f "/c/Program Files (x86)/WiX Toolset v3.11/bin/candle.exe" ]; then
  WIX_HEAT="/c/Program Files (x86)/WiX Toolset v3.11/bin/heat.exe"
  WIX_CANDLE="/c/Program Files (x86)/WiX Toolset v3.11/bin/candle.exe"
  WIX_LIGHT="/c/Program Files (x86)/WiX Toolset v3.11/bin/light.exe"
elif [ -f "/c/Program Files (x86)/WiX Toolset v3.14/bin/candle.exe" ]; then
  WIX_HEAT="/c/Program Files (x86)/WiX Toolset v3.14/bin/heat.exe"
  WIX_CANDLE="/c/Program Files (x86)/WiX Toolset v3.14/bin/candle.exe"
  WIX_LIGHT="/c/Program Files (x86)/WiX Toolset v3.14/bin/light.exe"
fi

if [ -n "$WIX_CANDLE" ] && [ -n "$WIX_LIGHT" ] && [ -n "$WIX_HEAT" ]; then
  echo "Compiling WiX MSI installer..."

  if command -v cygpath >/dev/null 2>&1; then
    SOURCE_DIR_WIN="$(cygpath -w "$PROJECT_ROOT/package/qworldclock")"
    ICON_PATH_WIN="$(cygpath -w "$PROJECT_ROOT/resources/icons/qworldclock.ico")"
    LICENSE_RTF_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/License.rtf")"
    WXS_MAIN_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/qworldclock.wxs")"
    WXS_FILES_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/files.wxs")"
    OUT_DIR_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/")\\"
    MSI_OUT_WIN="$(cygpath -w "$PROJECT_ROOT/dist/qworldclock-${TAG_NAME}-windows-x64.msi")"
    OBJ_MAIN_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/qworldclock.wixobj")"
    OBJ_FILES_WIN="$(cygpath -w "$PROJECT_ROOT/packaging/windows/files.wixobj")"
  else
    SOURCE_DIR_WIN="$PROJECT_ROOT\\package\\qworldclock"
    ICON_PATH_WIN="$PROJECT_ROOT\\resources\\icons\\qworldclock.ico"
    LICENSE_RTF_WIN="$PROJECT_ROOT\\packaging\\windows\\License.rtf"
    WXS_MAIN_WIN="$PROJECT_ROOT\\packaging\\windows\\qworldclock.wxs"
    WXS_FILES_WIN="$PROJECT_ROOT\\packaging\\windows\\files.wxs"
    OUT_DIR_WIN="$PROJECT_ROOT\\packaging\\windows\\"
    MSI_OUT_WIN="$PROJECT_ROOT\\dist\\qworldclock-${TAG_NAME}-windows-x64.msi"
    OBJ_MAIN_WIN="$PROJECT_ROOT\\packaging\\windows\\qworldclock.wixobj"
    OBJ_FILES_WIN="$PROJECT_ROOT\\packaging\\windows\\files.wixobj"
  fi

  # Remove empty directories to avoid empty directory components
  find package/qworldclock -type d -empty -delete 2>/dev/null || true

  MSYS2_ARG_CONV_EXCL="*" MSYS_NO_PATHCONV=1 "$WIX_HEAT" dir "$SOURCE_DIR_WIN" \
    -cg AppFiles -dr INSTALLFOLDER -scom -sreg -sfrag -srd -ag -sw5150 \
    -var var.SourceDir -out "$WXS_FILES_WIN"

  # Replace any remaining PUT-GUID-HERE with valid GUIDs
  python3 -c "import re, uuid, os; p='${PROJECT_ROOT}/packaging/windows/files.wxs'; f=open(p,'r',encoding='utf-8',errors='ignore'); c=f.read(); f.close(); open(p,'w',encoding='utf-8').write(re.sub(r'PUT-GUID-HERE', lambda m: str(uuid.uuid4()).upper(), c))" 2>/dev/null || \
  python -c "import re, uuid, os; p='${PROJECT_ROOT}/packaging/windows/files.wxs'; f=open(p,'r',encoding='utf-8',errors='ignore'); c=f.read(); f.close(); open(p,'w',encoding='utf-8').write(re.sub(r'PUT-GUID-HERE', lambda m: str(uuid.uuid4()).upper(), c))" 2>/dev/null || \
  sed -i 's/PUT-GUID-HERE/*/g' "$PROJECT_ROOT/packaging/windows/files.wxs" 2>/dev/null || true

  MSYS2_ARG_CONV_EXCL="*" MSYS_NO_PATHCONV=1 "$WIX_CANDLE" \
    -dVersion="$WIX_VERSION" -dSourceDir="$SOURCE_DIR_WIN" -dIconPath="$ICON_PATH_WIN" -dLicenseRtf="$LICENSE_RTF_WIN" \
    -arch x64 "$WXS_MAIN_WIN" "$WXS_FILES_WIN" -out "$OUT_DIR_WIN"

  MSYS2_ARG_CONV_EXCL="*" MSYS_NO_PATHCONV=1 "$WIX_LIGHT" \
    -sval -ext WixUIExtension "$OBJ_MAIN_WIN" "$OBJ_FILES_WIN" -out "$MSI_OUT_WIN"

  rm -f packaging/windows/*.wixobj packaging/windows/files.wxs packaging/windows/*.wixpdb
  echo "Windows MSI installer created: dist/qworldclock-${TAG_NAME}-windows-x64.msi"
else
  echo "WiX Toolset (candle/light/heat) not found; skipping Windows MSI installer."
fi

echo ""
echo "=================================================="
echo " Windows Packaging Completed"
shopt -s nullglob
PACKAGES=(dist/*.zip dist/*.exe dist/*.msi)
shopt -u nullglob
if [ ${#PACKAGES[@]} -gt 0 ]; then
  ls -lh "${PACKAGES[@]}"
else
  echo "No packages generated."
fi
echo "=================================================="
