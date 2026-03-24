#ifndef OPENNOTE_LINUX_PREFERENCESDIALOG_H
#define OPENNOTE_LINUX_PREFERENCESDIALOG_H
#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>

#include "AppDatabase.h"
#include "SyntaxTheme.h"

class SyntaxHighlighter;

class PreferencesDialog : public QDialog {
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget* parent = nullptr);

    signals :
    
    void settingsChanged(); // emitted when OK or Apply is clicked

private
    slots :
    
    void apply();

    void onTokenSelected(int row);

    void onColorButtonClicked();

    void onDefaultColorButtonClicked();

    void onBoldToggled(bool on);

    void onItalicToggled(bool on);

    void onThemeChanged(int index);

    void onPreviewModeChanged(int index);

private:
    void setupUi();

    QWidget* buildViewTab();

    QWidget* buildEditorTab();

    QWidget* buildSyntaxHighlightingTab();

    void loadFromDatabase();

    void saveToDatabase();

    void refreshPreview();

    void updateColorButton();

    QTabWidget* m_tabs = nullptr;

    // View tab
    QCheckBox* m_showLineNumbers = nullptr;
    QCheckBox* m_highlightCurrentLine = nullptr;
    QCheckBox* m_showStatusBar = nullptr;
    QCheckBox* m_wordWrap = nullptr;
    QCheckBox* m_wrapAtWord = nullptr;

    // Editor tab
    QSpinBox* m_tabSize = nullptr;
    QCheckBox* m_spacesInsteadOfTabs = nullptr;
    QCheckBox* m_autoIndent = nullptr;
    QCheckBox* m_openRecentFiles = nullptr;
    QCheckBox* m_autoSave = nullptr;
    QSpinBox* m_autoSaveInterval = nullptr;

    // Font & Colors tab
    QListWidget* m_tokenList = nullptr;
    QPushButton* m_colorBtn = nullptr;
    QPushButton* m_defaultColorBtn = nullptr;
    QCheckBox* m_boldChk = nullptr;
    QCheckBox* m_italicChk = nullptr;
    QPlainTextEdit* m_preview = nullptr;
    SyntaxHighlighter* m_highlighter = nullptr;
    QComboBox* m_themeCombo = nullptr;
    QComboBox* m_previewModeCombo = nullptr;

    // Working copy of the theme being edited
    SyntaxTheme m_editTheme;
    QString m_currentToken;
    bool m_loadingTheme = false;
};

#endif  // OPENNOTE_LINUX_PREFERENCESDIALOG_H