#!/usr/bin/env bash
# =============================================================================
# install-appimage.sh - Install AppImage to system and integrate with launcher
# =============================================================================
# Usage: ./install-appimage.sh OpenNote-....AppImage
#
# This script will:
#   1. Copy the AppImage to ~/.local/bin/
#   2. Extract and install the .desktop file to ~/.local/share/applications/
#   3. Extract and install the icon to ~/.local/share/icons/
#   4. Update desktop database
# =============================================================================

set -euo pipefail

# Check if AppImage path is provided
if [ $# -eq 0 ]; then
    echo "Missing arguments!"
    echo "Usage: $0 <path to OpenNote-....AppImage>"
    exit 1
fi

APPIMAGE_PATH="$1"
APPIMAGE_NAME="$(basename "$APPIMAGE_PATH")"

# Verify the AppImage exists
if [ ! -f "$APPIMAGE_PATH" ]; then
    echo "ERROR: AppImage not found: $APPIMAGE_PATH"
    exit 1
fi

# Make it executable if it isn't already
chmod +x "$APPIMAGE_PATH"

echo "Installing OpenNote AppImage..."
echo ""

# Copy AppImage to ~/.local/bin/
echo "[1/4] Installing AppImage to ~/.local/bin/..."
mkdir -p "$HOME/.local/bin"
INSTALL_PATH="$HOME/.local/bin/OpenNote.AppImage"
cp "$APPIMAGE_PATH" "$INSTALL_PATH"
chmod +x "$INSTALL_PATH"
echo "✓ Installed to: $INSTALL_PATH"

# Extract the desktop file from the AppImage
echo ""
echo "[2/4] Extracting and installing .desktop file..."
mkdir -p "$HOME/.local/share/applications"

# Extract the AppImage to a temp directory
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"
"$INSTALL_PATH" --appimage-extract >/dev/null 2>&1

# Find and copy the desktop file
DESKTOP_FILE=$(find squashfs-root -name "*.desktop" | head -n 1)
if [ -z "$DESKTOP_FILE" ]; then
    echo "WARNING: No .desktop file found in AppImage"
    echo "Creating a basic desktop entry..."

    cat > "$HOME/.local/share/applications/opennote.desktop" << EOF
[Desktop Entry]
Type=Application
Name=OpenNote
GenericName=Text Editor
Comment=A Notepad++ inspired code editor for Linux
Exec=$INSTALL_PATH %F
Icon=opennote
Categories=Development;TextEditor;
MimeType=text/plain;text/x-csrc;text/x-c++src;text/x-python;text/html;text/css;application/json;text/markdown;
StartupNotify=true
Terminal=false
EOF
else
    cp "$DESKTOP_FILE" "$HOME/.local/share/applications/opennote.desktop"
    # Update the Exec line to point to the installed AppImage
    sed -i "s|^Exec=.*|Exec=$INSTALL_PATH %F|g" "$HOME/.local/share/applications/opennote.desktop"
fi
echo "✓ Desktop file installed"

# Extract and install the icon
echo ""
echo "[3/4] Extracting and installing icon..."
mkdir -p "$HOME/.local/share/icons/hicolor/256x256/apps"

# Try to find the icon in the extracted AppImage
ICON_FILE=$(find squashfs-root -name "opennote.png" -o -name "*.png" | head -n 1)
if [ -n "$ICON_FILE" ]; then
    cp "$ICON_FILE" "$HOME/.local/share/icons/hicolor/256x256/apps/opennote.png"
    echo "✓ Icon installed"
else
    echo "WARNING: No icon found in AppImage"
    echo "You may need to manually place opennote.png in:"
    echo "$HOME/.local/share/icons/hicolor/256x256/apps/"
fi

# Clean up temp directory
cd - >/dev/null
rm -rf "$TEMP_DIR"

# Update desktop database
echo ""
echo "[4/4] Updating desktop database..."
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$HOME/.local/share/applications" 2>/dev/null || true
    echo "✓ Desktop database updated"
fi

if command -v gtk-update-icon-cache >/dev/null 2>&1; then
    gtk-update-icon-cache "$HOME/.local/share/icons/hicolor" 2>/dev/null || true
    echo "✓ Icon cache updated"
fi

echo ""
echo "✓ Installation complete!"
echo ""
echo "OpenNote should now appear in your application launcher."
echo "If it doesn't show up immediately, try:"
echo "  - Logging out and back in"
echo "  - Restarting your desktop environment"
echo ""
echo "To uninstall:"
echo "  rm $INSTALL_PATH"
echo "  rm $HOME/.local/share/applications/opennote.desktop"
echo "  rm $HOME/.local/share/icons/hicolor/256x256/apps/opennote.png"
echo ""
