#include "FindReplaceDialog.h"

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QRegularExpression>
#include <QTextDocument>
#include <QVBoxLayout>

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint) {
    setWindowTitle("Find and Replace");
    setModal(false);
    setupUi();
    resize(480, 280);
}

void FindReplaceDialog::setupUi() {
    auto *root = new QVBoxLayout(this);
    root->setSpacing(10);
    root->setContentsMargins(16, 16, 16, 12);

    auto *formLayout = new QGridLayout;
    formLayout->setColumnStretch(1, 1);
    formLayout->setVerticalSpacing(8);

    formLayout->addWidget(new QLabel("Find"), 0, 0, Qt::AlignRight);
    m_findEdit = new QLineEdit(this);
    m_findEdit->setPlaceholderText("Search text…");
    formLayout->addWidget(m_findEdit, 0, 1);

    formLayout->addWidget(new QLabel("Replace with"), 1, 0, Qt::AlignRight);
    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setPlaceholderText("Nothing");
    formLayout->addWidget(m_replaceEdit, 1, 1);

    root->addLayout(formLayout);

    auto *optGrid = new QGridLayout;
    optGrid->setHorizontalSpacing(20);
    optGrid->setVerticalSpacing(4);

    m_matchCase = new QCheckBox("Match case", this);
    m_wholeWord = new QCheckBox("Match entire word only", this);
    m_regex = new QCheckBox("Regular expression", this);
    m_backward = new QCheckBox("Search backwards", this);
    m_wrap = new QCheckBox("Wrap around", this);
    m_wrap->setChecked(true);

    optGrid->addWidget(m_matchCase, 0, 0);
    optGrid->addWidget(m_backward, 0, 1);
    optGrid->addWidget(m_wholeWord, 1, 0);
    optGrid->addWidget(m_wrap, 1, 1);
    optGrid->addWidget(m_regex, 2, 0);
    root->addLayout(optGrid);

    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #e74c3c; font-size: 12px;");
    root->addWidget(m_statusLabel);

    auto *btnRow = new QHBoxLayout;
    btnRow->addStretch();
    m_replaceAllBtn = new QPushButton("Replace All", this);
    m_replaceBtn = new QPushButton("Replace", this);
    m_findBtn = new QPushButton("Find", this);
    m_findBtn->setDefault(true);

    btnRow->addWidget(m_replaceAllBtn);
    btnRow->addWidget(m_replaceBtn);
    btnRow->addWidget(m_findBtn);
    root->addLayout(btnRow);

    connect(m_findBtn, &QPushButton::clicked, this, &FindReplaceDialog::findNext);
    connect(m_replaceBtn, &QPushButton::clicked, this,
            &FindReplaceDialog::replace);
    connect(m_replaceAllBtn, &QPushButton::clicked, this,
            &FindReplaceDialog::replaceAll);
    connect(m_findEdit, &QLineEdit::returnPressed, this,
            &FindReplaceDialog::findNext);
}

void FindReplaceDialog::setEditor(CodeEditor *editor) { m_editor = editor; }

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
    m_statusLabel->setText("Not found.");
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
    doFind(false); // Advance to next
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
    m_statusLabel->setStyleSheet("color: #2ecc71; font-size: 12px;");
    m_statusLabel->setText(count
                               ? QString("%1 replacement(s) made.").arg(count)
                               : "Not found.");
}
