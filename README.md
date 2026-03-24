# OpenNote

An open-source, lightweight text editor for Linux inspired by Notepad++.

## Features

- **Syntax Highlighting** - Support for multiple programming languages
- **File Tree Navigation** - Browse and manage project files easily
- **Tabbed Interface** - Work on multiple files simultaneously
- **Find & Replace** - Search content with regex support
- **Code Folding** - Collapse and expand code blocks
- **Customizable Preferences** - Tailor the editor to your workflow
- **Fast & Lightweight** - Native C++ performance
- **Auto-save Support** - Never lose your work
- **Multiple Themes** - Choose syntax highlighting themes

## Installation

1. Download `OpenNote-...-x86_64.AppImage` and `install-appimage.sh` from the latest [release](https://github.com/TayebYassine/OpenNote/releases).
2. Make it executable and run:

```bash
chmod +x OpenNote-...-x86_64.AppImage
./OpenNote-...-x86_64.AppImage
```

### System Integration

To make OpenNote appear in your system's application launcher:

```bash
chmod +x install-appimage.sh
./install-appimage.sh OpenNote-...-x86_64.AppImage
```

If it doesn't appear immediately, log out and back in, or restart your computer.

### Uninstall AppImage
```bash
rm ~/.local/bin/OpenNote.AppImage
rm ~/.local/share/applications/opennote.desktop
rm ~/.local/share/icons/hicolor/256x256/apps/opennote.png
```

## Building from Source

### Prerequisites

- **Qt 6.7+**
- **CMake 3.28+**
- **Ninja build system**
- **GCC** with C++ 17 support
- **Git**

#### Clone the Repository

```bash
git clone https://github.com/TayebYassine/OpenNote.git
cd OpenNote
```

#### Build AppImage

```bash
# Build
chmod +x build-appimage.sh
./build-appimage.sh

# Run
./OpenNote-...-x86_64.AppImage
```

#### Build for Development

```bash
# Configure
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_PREFIX_PATH="/path/to/Qt/6.7.3/gcc_64" \
    -G Ninja

# Build
cmake --build build --parallel $(nproc)

# Run
./build/OpenNote_Linux
```

## License

This project is licensed under the MIT License, see the [LICENSE](./LICENSE) file for details.
