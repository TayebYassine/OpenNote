#ifndef OPENNOTE_LINUX_FINDREPLACEDIALOG_H
#define OPENNOTE_LINUX_FINDREPLACEDIALOG_H
#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "CodeEditor.h"

class FindReplaceDialog : public QDialog {
    Q_OBJECT

public:
    explicit FindReplaceDialog(QWidget* parent = nullptr);

    void setEditor(CodeEditor* editor);

public
    slots :
    
    void findNext();
    void findPrev();

    void replace();
    void replaceAll();

private:
    void setupUi();

    bool doFind(bool backward);

    QTextDocument::FindFlags buildFlags(bool backward) const;

    CodeEditor* m_editor = nullptr;

    QLineEdit* m_findEdit = nullptr;
    QLineEdit* m_replaceEdit = nullptr;
    QLabel* m_statusLabel = nullptr;

    QCheckBox* m_matchCase = nullptr;
    QCheckBox* m_wholeWord = nullptr;
    QCheckBox* m_regex = nullptr;
    QCheckBox* m_backward = nullptr;
    QCheckBox* m_wrap = nullptr;

    QPushButton* m_findBtn = nullptr;
    QPushButton* m_replaceBtn = nullptr;
    QPushButton* m_replaceAllBtn = nullptr;
};

#endif  // OPENNOTE_LINUX_FINDREPLACEDIALOG_H