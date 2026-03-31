#ifndef OPENNOTE_LINUX_EDITORTABWIDGET_H
#define OPENNOTE_LINUX_EDITORTABWIDGET_H
#pragma once

#include <QTabWidget>

#include "CodeEditor.h"
#include "Language.h"

class EditorTabWidget : public QTabWidget {
    Q_OBJECT

public:
    explicit EditorTabWidget(QWidget* parent = nullptr);

    CodeEditor* currentEditor() const;

    CodeEditor* editorAt(int index) const;

    void openFile(const QString& filePath);

    void newFile();

    bool closeTab(int index);
    bool closeAllTabs();

    bool saveTab(int index);
    bool saveTabAs(int index);

    void overrideLanguage(Language lang);

    void updateRecentFiles();

    void setTabSize(int size);

    int tabSize() const { return m_tabSize; }

    void setAutoIndent(bool on);

    bool autoIndent() const { return m_autoIndent; }

    QIcon iconForFile(const QString& filePath) const;

    signals :


    
    void currentFileChanged(const QString& filePath, Language lang);

    void cursorPositionChanged(int line, int col);

private
    slots :

    void onTabCloseRequested(int index);

    void onCurrentChanged(int index);

    void onCursorMoved();

private:
    void updateTabTitle(int index);

    void attachCloseButton(int index);

    int findTabByPath(const QString& filePath) const;

    void connectEditor(CodeEditor* editor);

    void applySettingsToEditor(CodeEditor* editor) const;

    int m_tabSize = 4;
    bool m_autoIndent = true;
};

#endif  // OPENNOTE_LINUX_EDITORTABWIDGET_H