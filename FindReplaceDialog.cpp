#include "FindReplaceDialog.h"

#include <QApplication>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QSpacerItem>
#include <QStyle>
#include <QTextDocument>
#include <QVBoxLayout>

#include "CodeEditor.h"

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint) {
    setWindowTitle("Find and Replace");
    setModal(false);
    setupUi();
    resize(520, 340);
}

void FindReplaceDialog::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);


    auto *content = new QWidget(this);
    auto *contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(20, 16, 20, 16);


    auto *findRow = new QHBoxLayout;
    findRow->setSpacing(10);
    auto *findLabel = new QLabel("Find", content);
    findLabel->setFixedWidth(80);
    findLabel->setObjectName("frLabel");
    m_findEdit = new QLineEdit(content);
    m_findEdit->setObjectName("frInput");
    m_findEdit->setPlaceholderText("Search text…");
    findRow->addWidget(findLabel);
    findRow->addWidget(m_findEdit, 1);
    contentLayout->addLayout(findRow);


    auto *replaceRow = new QHBoxLayout;
    replaceRow->setSpacing(10);
    auto *replaceLabel = new QLabel("Replace", content);
    replaceLabel->setFixedWidth(80);
    replaceLabel->setObjectName("frLabel");
    m_replaceEdit = new QLineEdit(content);
    m_replaceEdit->setObjectName("frInput");
    m_replaceEdit->setPlaceholderText("Replacement text…");
    replaceRow->addWidget(replaceLabel);
    replaceRow->addWidget(m_replaceEdit, 1);
    contentLayout->addLayout(replaceRow);


    auto *optGroup = new QWidget(content);
    optGroup->setObjectName("frOptions");
    auto *optLayout = new QGridLayout(optGroup);
    optLayout->setContentsMargins(12, 12, 12, 12);
    optLayout->setSpacing(10);
    optLayout->setColumnStretch(1, 1);

    m_matchCase = new QCheckBox("Match case", optGroup);
    m_wholeWord = new QCheckBox("Whole word", optGroup);
    m_regex = new QCheckBox("Regular expression", optGroup);
    m_backward = new QCheckBox("Search backwards", optGroup);
    m_wrap = new QCheckBox("Wrap around", optGroup);
    m_wrap->setChecked(true);

    optLayout->addWidget(m_matchCase, 0, 0);
    optLayout->addWidget(m_wholeWord, 0, 1);
    optLayout->addWidget(m_regex, 1, 0);
    optLayout->addWidget(m_backward, 1, 1);
    optLayout->addWidget(m_wrap, 2, 0);

    contentLayout->addWidget(optGroup);


    m_statusLabel = new QLabel(content);
    m_statusLabel->setObjectName("frStatus");
    m_statusLabel->setMinimumHeight(20);
    contentLayout->addWidget(m_statusLabel);


    auto *btnRow = new QHBoxLayout;
    btnRow->setSpacing(8);
    btnRow->addStretch();

    m_findPrevBtn = new QPushButton("◀  Find Previous", content);
    m_findPrevBtn->setObjectName("frBtnSecondary");
    m_findPrevBtn->setCursor(Qt::PointingHandCursor);

    m_findBtn = new QPushButton("Find Next  ▶", content);
    m_findBtn->setObjectName("frBtnPrimary");
    m_findBtn->setCursor(Qt::PointingHandCursor);
    m_findBtn->setDefault(true);

    m_replaceBtn = new QPushButton("Replace", content);
    m_replaceBtn->setObjectName("frBtnSecondary");
    m_replaceBtn->setCursor(Qt::PointingHandCursor);

    m_replaceAllBtn = new QPushButton("Replace All", content);
    m_replaceAllBtn->setObjectName("frBtnSecondary");
    m_replaceAllBtn->setCursor(Qt::PointingHandCursor);

    btnRow->addWidget(m_findPrevBtn);
    btnRow->addWidget(m_findBtn);
    btnRow->addSpacing(12);
    btnRow->addWidget(m_replaceBtn);
    btnRow->addWidget(m_replaceAllBtn);
    contentLayout->addLayout(btnRow);

    root->addWidget(content, 1);


    connect(m_findBtn, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(m_findPrevBtn, &QPushButton::clicked, this, &FindReplaceDialog::findPrev);
    connect(m_replaceBtn, &QPushButton::clicked, this, &FindReplaceDialog::replace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this, &FindReplaceDialog::replaceAll);
    connect(m_findEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::findNext);
    connect(m_replaceEdit, &QLineEdit::returnPressed, this, &FindReplaceDialog::replace);
    connect(m_findEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::updateButtonStates);
    connect(m_replaceEdit, &QLineEdit::textChanged, this, &FindReplaceDialog::updateButtonStates);
}

void FindReplaceDialog::updateButtonStates() {
    const bool hasFindText = !m_findEdit->text().isEmpty();
    m_findBtn->setEnabled(hasFindText);
    m_findPrevBtn->setEnabled(hasFindText);
    m_replaceBtn->setEnabled(hasFindText);
    m_replaceAllBtn->setEnabled(hasFindText);
}

void FindReplaceDialog::setEditor(CodeEditor *editor) {
    m_editor = editor;
    updateButtonStates();
}

QTextDocument::FindFlags FindReplaceDialog::buildFlags(bool backward) const {
    QTextDocument::FindFlags flags;
    if (m_matchCase->isChecked()) flags |= QTextDocument::FindCaseSensitively;
    if (m_wholeWord->isChecked()) flags |= QTextDocument::FindWholeWords;
    if (backward || m_backward->isChecked()) flags |= QTextDocument::FindBackward;
    return flags;
}

bool FindReplaceDialog::doFind(bool backward) {
    if (!m_editor || m_findEdit->text().isEmpty()) return false;
    m_statusLabel->clear();

    const QTextDocument::FindFlags flags = buildFlags(backward);
    const QString needle = m_findEdit->text();
    QTextDocument *doc = m_editor->document();
    QTextCursor cur = m_editor->textCursor();
    QTextCursor found;

    if (m_regex->isChecked()) {
        QRegularExpression::PatternOptions opts;
        if (!m_matchCase->isChecked())
            opts |= QRegularExpression::CaseInsensitiveOption;
        found = doc->find(QRegularExpression(needle, opts), cur, flags);
        if (found.isNull() && m_wrap->isChecked()) {
            QTextCursor start(doc);
            if (backward) start.movePosition(QTextCursor::End);
            found = doc->find(QRegularExpression(needle, opts), start, flags);
        }
    } else {
        found = doc->find(needle, cur, flags);
        if (found.isNull() && m_wrap->isChecked()) {
            QTextCursor start(doc);
            if (backward) start.movePosition(QTextCursor::End);
            found = doc->find(needle, start, flags);
        }
    }

    if (!found.isNull()) {
        m_editor->setTextCursor(found);
        return true;
    }
    m_statusLabel->setText("No matches found.");
    return false;
}

void FindReplaceDialog::findNext() { doFind(false); }
void FindReplaceDialog::findPrev() { doFind(true); }

void FindReplaceDialog::replace() {
    if (!m_editor) return;
    QTextCursor cur = m_editor->textCursor();

    if (cur.hasSelection()) {
        const QString sel = cur.selectedText();
        const QString needle = m_findEdit->text();
        bool matches = false;

        if (m_regex->isChecked()) {
            QRegularExpression::PatternOptions opts;
            if (!m_matchCase->isChecked())
                opts |= QRegularExpression::CaseInsensitiveOption;
            matches = QRegularExpression(needle, opts).match(sel).hasMatch();
        } else {
            Qt::CaseSensitivity cs =
                    m_matchCase->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
            matches = (sel.compare(needle, cs) == 0);
        }

        if (matches) {
            cur.insertText(m_replaceEdit->text());
            m_editor->setTextCursor(cur);
        }
    }
    doFind(false);
}

void FindReplaceDialog::replaceAll() {
    if (!m_editor || m_findEdit->text().isEmpty()) return;
    m_statusLabel->clear();

    const QString needle = m_findEdit->text();
    const QString replacement = m_replaceEdit->text();
    const QTextDocument::FindFlags flags =
            (m_matchCase->isChecked()
                 ? QTextDocument::FindCaseSensitively
                 : QTextDocument::FindFlags()) |
            (m_wholeWord->isChecked()
                 ? QTextDocument::FindWholeWords
                 : QTextDocument::FindFlags());

    QTextDocument *doc = m_editor->document();
    QTextCursor cur(doc);
    cur.beginEditBlock();
    int count = 0;

    while (true) {
        QTextCursor found;
        if (m_regex->isChecked()) {
            QRegularExpression::PatternOptions opts;
            if (!m_matchCase->isChecked())
                opts |= QRegularExpression::CaseInsensitiveOption;
            found = doc->find(QRegularExpression(needle, opts), cur, flags);
        } else {
            found = doc->find(needle, cur, flags);
        }
        if (found.isNull()) break;
        found.insertText(replacement);
        cur = found;
        ++count;
    }

    cur.endEditBlock();
    m_statusLabel->setStyleSheet("color: #2ecc71; font-size: 12px; padding-left: 4px;");
    m_statusLabel->setText(count
                               ? QString("%1 replacement(s) made.").arg(count)
                               : "No matches found.");
}

void FindReplaceDialog::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }
    QDialog::keyPressEvent(event);
}

void FindReplaceDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    if (m_findEdit) {
        m_findEdit->setFocus();
        m_findEdit->selectAll();
    }
    updateButtonStates();
}
