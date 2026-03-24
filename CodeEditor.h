#ifndef OPENNOTE_LINUX_CODEEDITOR_H
#define OPENNOTE_LINUX_CODEEDITOR_H
#pragma once

#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QSet>

#include "Language.h"
#include "SyntaxTheme.h"
#include "FoldManager.h"

class SyntaxHighlighter;
class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget* parent = nullptr);

    void setFilePath(const QString& path);
    QString filePath() const { return m_filePath; }

    void setLanguage(Language lang);
    Language language() const { return m_language; }

    void applyTheme(const SyntaxTheme& theme) const;

    void setShowLineNumbers(bool show);

    void setHighlightCurrentLine(bool hl);

    void setWordWrap(bool wrap, bool atWord = true);

    void setTabSize(int size);

    void setAutoIndent(bool on) { m_autoIndent = on; }
    void setSpacesInsteadOfTabs(bool on) { m_spacesInsteadOfTabs = on; }

    void toggleBookmark(int line);
    void nextBookmark();
    void prevBookmark();
    void clearBookmarks();

    const QSet<int>& bookmarks() const { return m_bookmarks; }
    void setBookmarks(const QSet<int>& bm);

    void setCodeFoldingEnabled(bool enabled);
    bool codeFoldingEnabled() const { return m_codeFoldingEnabled; }
    void refreshFolds();

    int lineNumberAreaWidth() const;

    void lineNumberAreaPaintEvent(const QPaintEvent* event) const;

    void lineNumberAreaMousePress(const QMouseEvent* event);

public slots:
    void goToLine(int lineNumber);

protected:
    void resizeEvent(QResizeEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

    void paintEvent(QPaintEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);

    void onHighlightCurrentLine();

    void updateLineNumberArea(const QRect& rect, int dy);

    void onCursorPositionChanged();

private:
    void initEditor();

    LineNumberArea* m_lineNumberArea = nullptr;
    SyntaxHighlighter* m_highlighter = nullptr;
    FoldManager* m_foldManager = nullptr;
    QSet<int> m_bookmarks;
    Language m_language = Language::None;
    QString m_filePath;

    bool m_showLineNumbers = true;
    bool m_doHighlightCurrentLine = true;
    bool m_autoIndent = true;
    bool m_spacesInsteadOfTabs = false;
    bool m_codeFoldingEnabled = true;
    int m_tabSize = 4;
};

#endif  // OPENNOTE_LINUX_CODEEDITOR_H
