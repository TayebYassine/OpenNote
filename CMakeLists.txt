cmake_minimum_required(VERSION 3.22)
project(OpenNote_Linux VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/home/tayeb-yassine/Qt/6.7.3/gcc_64")
set(Qt6_ROOT "/home/tayeb-yassine/Qt/6.7.3/gcc_64")
set(Qt6_DIR "/home/tayeb-yassine/Qt/6.7.3/gcc_64/lib/cmake/Qt6")

find_package(Qt6 6.7 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(OpenNote_Linux
        main.cpp
        MainWindow.cpp
        MainWindow.h
        FileTreeWidget.h
        FileTreeWidget.cpp
        Language.h
        SyntaxHighlighter.h
        SyntaxHighlighter.cpp
        CodeEditor.h
        CodeEditor.cpp
        EditorTabWidget.h
        EditorTabWidget.cpp
        AppDatabase.h
        AppDatabase.cpp
        SyntaxTheme.h
        FindReplaceDialog.h
        FindReplaceDialog.cpp
        PreferencesDialog.h
        PreferencesDialog.cpp
        AboutDialog.h
        AboutDialog.cpp
        FoldManager.h
        FoldManager.cpp
)

target_compile_definitions(OpenNote_Linux PRIVATE
        OPENNOTE_VERSION="${PROJECT_VERSION}"
)

target_link_libraries(OpenNote_Linux PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

include(GNUInstallDirs)

# Install binary
install(TARGETS OpenNote_Linux
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install desktop file
install(FILES packaging/opennote.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# Install icon
install(FILES packaging/opennote.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps
)

# CPack configuration for DEB packages
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "opennote")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "OpenNote Developers <you@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A Notepad++ inspired code editor for Linux")
set(CPACK_PACKAGE_DESCRIPTION "OpenNote is a powerful, lightweight code editor for Linux
inspired by Notepad++. It features syntax highlighting, multiple tabs,
find and replace, code folding, and a clean, intuitive interface.")
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.4), libqt6gui6 (>= 6.4), libqt6widgets6 (>= 6.4)")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/packaging/postinst")

include(CPack)