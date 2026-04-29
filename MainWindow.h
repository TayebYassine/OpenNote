#ifndef OPENNOTE_LINUX_MAINWINDOW_H
#define OPENNOTE_LINUX_MAINWINDOW_H
#pragma once

#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>
#include <QSplitter>
#include <QTimer>

#include "EditorTabWidget.h"
#include "FileTreeWidget.h"
#include "Language.h"

class FindReplaceDialog;
class PreferencesDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override = default;

    void openFiles(const QStringList& paths);

    bool hasTabOpenWithPath(const QString& filePath);

protected:
    void closeEvent(QCloseEvent* event) override;

private
    slots :
    // File

    void onNewFile();

    void onOpenFile();

    void onOpenFolder();

    void onSave();

    void onSaveAs();

    void onSaveAll();

    void onRevertFile();

    void onCloseTab();

    void onCloseAllTabs();

    void onOpenRecentFile(const QString& path);

    void onQuit();

    // Edit
    void onUndo();

    void onRedo();

    void onCut();

    void onCopy();

    void onPaste();

    void onSelectAll();

    void onFindReplace();

    void onZoomIn();

    void onZoomOut();

    void onZoomReset();

    // View
    void onToggleFileTree();

    void onFontSelected(const QString& family);

    // Bookmarks
    void onToggleBookmark();

    void onNextBookmark();

    void onPrevBookmark();

    void onClearBookmarks();

    // Language override
    void onLanguageOverride(Language lang);

    // Preferences
    void onPreferences();

    void onAbout();

    // Status bar updates
    void onCurrentFileChanged(const QString& filePath, Language lang);

    void onCursorPositionChanged(int line, int col);

    // File tree
    void onFileSelected(const QString& filePath);

    // Autosave timer
    void onAutoSaveTick();

    // Settings applied from PreferencesDialog
    void applyAllSettings();

private:
    void setupMenuBar();

    void setupCentralWidget();

    void setupStatusBar();

    void refreshRecentFilesMenu();

    void buildLanguageMenu();

    void buildFontMenu();

    void applyFontToAllEditors(const QString& family, int pointSize);

    QString autoDetectFileUnicode(const QString& filePath);

    void onFilePathChanged(const QString& oldPath, const QString& newPath);
    void onFilePathDeleted(const QString& path);

    // Widgets
    QSplitter* m_splitter = nullptr;
    FileTreeWidget* m_fileTree = nullptr;
    EditorTabWidget* m_tabWidget = nullptr;

    // Dialogs
    FindReplaceDialog* m_findReplace = nullptr;

    // Status bar
    QLabel* m_langLabel = nullptr;
    QLabel* m_fileUnicode = nullptr;
    QLabel* m_cursorLabel = nullptr;

    // Menus
    QMenu* m_recentMenu = nullptr;
    QMenu* m_languageMenu = nullptr;
    QMenu* m_fontMenu = nullptr;
    QActionGroup* m_langGroup = nullptr;
    QActionGroup* m_fontGroup = nullptr;

    // Autosave
    QTimer* m_autoSaveTimer = nullptr;
};

#endif  // OPENNOTE_LINUX_MAINWINDOW_H