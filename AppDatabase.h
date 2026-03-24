#ifndef OPENNOTE_LINUX_APPDATABASE_H
#define OPENNOTE_LINUX_APPDATABASE_H
#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QStringList>

#include "SyntaxTheme.h"

class AppDatabase {
public:
    static AppDatabase &instance();

    void load();

    void save() const;

    // Recent directory
    QString recentDirectory() const { return m_recentDir; }

    void setRecentDirectory(const QString &dir);

    // Recent files
    QStringList recentFiles() const { return m_recentFiles; }

    void clearRecentFiles();

    void addRecentFile(const QString &path);

    void removeRecentFile(const QString &path);

    // Bookmarks
    QList<int> bookmarksForFile(const QString &filePath) const;

    void setBookmarks(const QString &filePath, const QList<int> &lines);

    void clearBookmarksForFile(const QString &filePath);

    const QMap<QString, QList<int> > &allBookmarks() const { return m_bookmarks; }

    // Editor font
    QString editorFontFamily() const { return m_editorFontFamily; }
    int editorFontSize() const { return m_editorFontSize; }

    void setEditorFont(const QString &family, int pointSize);

    // View settings
    bool showLineNumbers() const { return m_showLineNumbers; }
    bool highlightCurrentLine() const { return m_highlightCurrentLine; }
    bool showStatusBar() const { return m_showStatusBar; }
    bool wordWrap() const { return m_wordWrap; }
    bool wrapAtWord() const { return m_wrapAtWord; }
    void setShowLineNumbers(bool v) { m_showLineNumbers = v; }
    void setHighlightCurrentLine(bool v) { m_highlightCurrentLine = v; }
    void setShowStatusBar(bool v) { m_showStatusBar = v; }
    void setWordWrap(bool v) { m_wordWrap = v; }
    void setWrapAtWord(bool v) { m_wrapAtWord = v; }

    // Editor behaviour
    int tabSize() const { return m_tabSize; }
    bool spacesInsteadOfTabs() const { return m_spacesInsteadOfTabs; }
    bool autoIndent() const { return m_autoIndent; }
    bool openRecentFiles() const { return m_openRecentFiles; }
    bool autoSave() const { return m_autoSave; }
    int autoSaveIntervalMin() const { return m_autoSaveIntervalMin; }
    void setTabSize(int v) { m_tabSize = qMax(1, qMin(v, 16)); }
    void setSpacesInsteadOfTabs(bool v) { m_spacesInsteadOfTabs = v; }
    void setAutoIndent(bool v) { m_autoIndent = v; }
    void setOpenRecentFiles(bool v) { m_openRecentFiles = v; }
    void setAutoSave(bool v) { m_autoSave = v; }
    void setAutoSaveIntervalMin(int v) { m_autoSaveIntervalMin = qMax(1, v); }

    // Syntax highlighting theme
    SyntaxTheme syntaxTheme() const { return m_syntaxTheme; }
    void setSyntaxTheme(const SyntaxTheme &t) { m_syntaxTheme = t; }

private:
    AppDatabase() = default;

    QString dbPath() const;

    QString m_recentDir;
    QStringList m_recentFiles;
    QMap<QString, QList<int> > m_bookmarks;

    QString m_editorFontFamily;
    int m_editorFontSize = 12;

    bool m_showLineNumbers = true;
    bool m_highlightCurrentLine = true;
    bool m_showStatusBar = true;
    bool m_wordWrap = false;
    bool m_wrapAtWord = true;

    int m_tabSize = 4;
    bool m_spacesInsteadOfTabs = false;
    bool m_autoIndent = true;
    bool m_openRecentFiles = true;
    bool m_autoSave = false;
    int m_autoSaveIntervalMin = 5;

    SyntaxTheme m_syntaxTheme = openNoteDefaultTheme();

    static constexpr int MAX_RECENT_FILES = 20;
};

#endif  // OPENNOTE_LINUX_APPDATABASE_H
