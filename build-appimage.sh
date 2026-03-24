#!/usr/bin/env bash
set -euo pipefail

QT_DIR="$HOME/Qt/6.7.3/gcc_64"
BUILD_DIR="build-appimage"
APP_VERSION="1.0.0"

[ ! -f "packaging/opennote.png" ]    && echo "ERROR: packaging/opennote.png not found."    && exit 1
[ ! -f "packaging/opennote.desktop" ] && echo "ERROR: packaging/opennote.desktop not found." && exit 1

echo ""
echo "[1/6] Compiling in Release mode..."
cmake -S . -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_PREFIX_PATH="$QT_DIR" \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -G Ninja
cmake --build "$BUILD_DIR" --parallel "$(nproc)"

echo ""
echo "[2/6] Staging binary into AppDir..."
rm -rf "$BUILD_DIR/AppDir"
DESTDIR="$BUILD_DIR/AppDir" cmake --install "$BUILD_DIR"

echo ""
echo "[3/6] Copying platform theme + icon engine plugins..."

PLUGIN_DEST="$BUILD_DIR/AppDir/usr/plugins"

mkdir -p "$PLUGIN_DEST/platformthemes"
if [ -f "$QT_DIR/plugins/platformthemes/libqgtk3.so" ]; then
    cp "$QT_DIR/plugins/platformthemes/libqgtk3.so" "$PLUGIN_DEST/platformthemes/"
    echo "✓ libqgtk3.so"
else
    echo "WARNING: libqgtk3.so not found at $QT_DIR/plugins/platformthemes/"
    echo "Dark mode and native icons may not work."
fi

mkdir -p "$PLUGIN_DEST/iconengines"
if [ -f "$QT_DIR/plugins/iconengines/libqsvgicon.so" ]; then
    cp "$QT_DIR/plugins/iconengines/libqsvgicon.so" "$PLUGIN_DEST/iconengines/"
    echo "✓ libqsvgicon.so"
fi

echo ""
echo "[4/6] Embedding application icon..."
mkdir -p "$BUILD_DIR/AppDir/usr/share/icons/hicolor/256x256/apps"
cp "packaging/opennote.png" "$BUILD_DIR/AppDir/usr/share/icons/hicolor/256x256/apps/opennote.png"
cp "packaging/opennote.png" "$BUILD_DIR/AppDir/opennote.png"
echo "✓ Icon embedded"

echo ""
echo "[5/6] Writing custom AppRun..."

cat > "$BUILD_DIR/AppDir/AppRun" << 'APPRUN'
#!/usr/bin/env bash
HERE="$(dirname "$(readlink -f "$0")")"

export QT_PLUGIN_PATH="$HERE/usr/plugins"
export QT_QPA_PLATFORMTHEME=gtk3
export QT_STYLE_OVERRIDE="${QT_STYLE_OVERRIDE:-}"
export LD_LIBRARY_PATH="$HERE/usr/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="$HERE/usr/share${XDG_DATA_DIRS:+:$XDG_DATA_DIRS}"

exec "$HERE/usr/bin/OpenNote_Linux" "$@"
APPRUN
chmod +x "$BUILD_DIR/AppDir/AppRun"

echo ""
echo "[6/6] Bundling Qt 6.7 libraries and building AppImage..."
mkdir -p "$BUILD_DIR/tools"
LD_BIN="$BUILD_DIR/tools/linuxdeploy-x86_64.AppImage"
LD_QT="$BUILD_DIR/tools/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "$LD_BIN" ]; then
    echo "Downloading linuxdeploy..."
    curl -# -L -o "$LD_BIN" \
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
    chmod +x "$LD_BIN"
fi
if [ ! -f "$LD_QT" ]; then
    echo "Downloading linuxdeploy-plugin-qt..."
    curl -# -L -o "$LD_QT" \
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
    chmod +x "$LD_QT"
fi

export APPIMAGE_EXTRACT_AND_RUN=1
export LD_LIBRARY_PATH="$QT_DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QMAKE="$QT_DIR/bin/qmake"
export QT_PLUGIN_PATH="$QT_DIR/plugins"

"$LD_BIN" \
    --appdir  "$BUILD_DIR/AppDir" \
    --desktop-file "packaging/opennote.desktop" \
    --icon-file    "packaging/opennote.png" \
    --plugin qt \
    --output appimage

OUTPUT="OpenNote-${APP_VERSION}-x86_64.AppImage"
for f in OpenNote*.AppImage; do
    [ "$f" != "$OUTPUT" ] && mv "$f" "$OUTPUT" || true
done

echo ""
echo "✓ Done! ./$OUTPUT"
echo ""
echo "To integrate into your system:"
echo "  ./install-appimage.sh $OUTPUT"
echo ""
