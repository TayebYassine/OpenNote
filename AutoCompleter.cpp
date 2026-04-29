#include "AutoCompleter.h"

#include <QAbstractItemView>
#include <QKeyEvent>
#include <QScrollBar>
#include <QStringListModel>

#include "CodeEditor.h"

AutoCompleter::AutoCompleter(CodeEditor* editor)
    : QObject(editor), m_editor(editor) {
    m_model = new QStringListModel(this);
    m_completer = new QCompleter(m_model, editor);
    m_completer->setWidget(editor);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setWrapAround(false);
    m_completer->setMaxVisibleItems(10);

    connect(m_completer, QOverload<const QString&>::of(&QCompleter::activated),
            this, &AutoCompleter::insertCompletion);
}

void AutoCompleter::updateWordList(const QString& text) {
    // Extract all words (alphanumeric + underscore, min length 2)
    QRegularExpression re(R"(\b[a-zA-Z_]\w{1,}\b)");
    auto it = re.globalMatch(text);

    QSet<QString> newWords;
    while (it.hasNext()) {
        auto match = it.next();
        newWords.insert(match.captured());
    }

    if (newWords == m_wordSet) return;

    m_wordSet = newWords;
    QStringList list = m_wordSet.values();
    list.sort(Qt::CaseInsensitive);
    m_model->setStringList(list);
}

void AutoCompleter::showCompletion() {
    const QString prefix = wordUnderCursor();
    if (prefix.length() < 2) {
        hideCompletion();
        return;
    }

    m_completer->setCompletionPrefix(prefix);

    if (m_completer->completionCount() <= 1) {
        // Only the word itself — no point showing
        hideCompletion();
        return;
    }

    QRect cr = m_editor->cursorRect();
    cr.setWidth(m_completer->popup()->sizeHintForColumn(0) +
                m_completer->popup()->verticalScrollBar()->sizeHint().width());
    cr.moveLeft(cr.left() + m_editor->lineNumberAreaWidth());
    m_completer->complete(cr);
}

void AutoCompleter::hideCompletion() {
    m_completer->popup()->hide();
}

bool AutoCompleter::isPopupVisible() const {
    return m_completer->popup()->isVisible();
}

QString AutoCompleter::wordUnderCursor() const {
    QTextCursor cursor = m_editor->textCursor();
    cursor.select(QTextCursor::WordUnderCursor);
    return cursor.selectedText();
}

void AutoCompleter::insertCompletion(const QString& completion) {
    if (m_completer->widget() != m_editor) return;

    QTextCursor cursor = m_editor->textCursor();
    int extra = completion.length() - m_completer->completionPrefix().length();
    cursor.movePosition(QTextCursor::EndOfWord);
    cursor.insertText(completion.right(extra));
    m_editor->setTextCursor(cursor);
}