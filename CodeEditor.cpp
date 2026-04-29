#include "CodeEditor.h"

#include <QFontDatabase>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTimer>

#include "AppDatabase.h"
#include "SyntaxHighlighter.h"
#include "AutoCompleter.h"

namespace {
constexpr int kBookmarkColumnWidth = 18;
constexpr int kFoldColumnWidth = 20;
}  // namespace

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor), m_editor(editor) {
        setCursor(Qt::ArrowCursor);
        setMouseTracking(true);
    }

    [[nodiscard]] QSize sizeHint() const override {
        return {m_editor->lineNumberAreaWidth(), 0};
    }

protected:
    void paintEvent(QPaintEvent *e) override {
        m_editor->lineNumberAreaPaintEvent(e);
    }

    void mousePressEvent(QMouseEvent *e) override {
        m_editor->lineNumberAreaMousePress(e);
    }

    void mouseMoveEvent(QMouseEvent *e) override {
        m_editor->lineNumberAreaMouseMove(e);
    }

    void leaveEvent(QEvent*) override {
        m_editor->lineNumberAreaLeave();
    }

    void contextMenuEvent(QContextMenuEvent* e) override {
        m_editor->lineNumberAreaContextMenuEvent(e);
    }

private:
    CodeEditor *m_editor;
};

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
    initEditor();
}

void CodeEditor::initEditor() {
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(12);
    setFont(font);
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
    setStyleSheet("QPlainTextEdit { border: none; }");
    setLineWrapMode(QPlainTextEdit::NoWrap);

    m_lineNumberArea = new LineNumberArea(this);
    m_foldManager = new FoldManager(document(), Language::None);
    m_autoCompleter = new AutoCompleter(this);

    connect(this, &CodeEditor::blockCountChanged, this,
            &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this,
            &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this,
            &CodeEditor::onHighlightCurrentLine);
    connect(this, &CodeEditor::cursorPositionChanged, this,
            &CodeEditor::onCursorPositionChanged);

    connect(document(), &QTextDocument::contentsChange, this,
            [this](int position, int charsRemoved, int charsAdded) {
                if (!m_codeFoldingEnabled || !m_foldManager) return;

                QTextBlock block = document()->findBlock(position);
                int modifiedLine = block.blockNumber();

                if (m_foldManager->isFoldStart(modifiedLine) &&
                    m_foldManager->isFolded(modifiedLine)) {
                    m_foldManager->unfoldRegion(modifiedLine);
                    viewport()->update();
                    m_lineNumberArea->update();
                }
            });
    connect(this, &CodeEditor::textChanged, this, &CodeEditor::updateAutoCompleteWords);

    updateLineNumberAreaWidth(0);
    onHighlightCurrentLine();
}

void CodeEditor::setFilePath(const QString &path) {
    m_filePath = path;
    if (!path.isEmpty()) setLanguage(languageFromFile(path));
}

void CodeEditor::setLanguage(Language lang) {
    m_language = lang;
    if (!m_highlighter)
        m_highlighter = new SyntaxHighlighter(
            document(), lang, AppDatabase::instance().syntaxTheme());
    else
        m_highlighter->setLanguage(lang);

    if (m_foldManager && m_codeFoldingEnabled) {
        m_foldManager->setLanguage(lang);
        QTimer::singleShot(0, this, &CodeEditor::refreshFolds);
    }
}

void CodeEditor::applyTheme(const SyntaxTheme &theme) const {
    if (m_highlighter) m_highlighter->applyTheme(theme);
}

void CodeEditor::setShowLineNumbers(bool show) {
    m_showLineNumbers = show;
    m_lineNumberArea->setVisible(show);
    setViewportMargins(show ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void CodeEditor::setHighlightCurrentLine(bool hl) {
    m_doHighlightCurrentLine = hl;
    onHighlightCurrentLine();
}

void CodeEditor::setWordWrap(bool wrap, bool atWord) {
    setLineWrapMode(wrap ? QPlainTextEdit::WidgetWidth : QPlainTextEdit::NoWrap);
    if (wrap) {
        setWordWrapMode(atWord
                            ? QTextOption::WrapAtWordBoundaryOrAnywhere
                            : QTextOption::WrapAnywhere);
    }
}

void CodeEditor::setTabSize(int size) {
    m_tabSize = qMax(1, qMin(size, 16));
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * m_tabSize);
}

void CodeEditor::setCodeFoldingEnabled(bool enabled) {
    m_codeFoldingEnabled = enabled;
    if (enabled && m_foldManager) {
        m_foldManager->detectFolds();
    } else if (m_foldManager) {
        m_foldManager->clear();
    }
    m_lineNumberArea->update();
}

void CodeEditor::refreshFolds() {
    if (m_foldManager && m_codeFoldingEnabled) {
        m_foldManager->detectFolds();
        m_lineNumberArea->update();
    }
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
    if (m_autoCompleter && m_autoCompleter->isPopupVisible()) {
        if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Return ||
            event->key() == Qt::Key_Enter) {
            // Let the completer handle it
            QPlainTextEdit::keyPressEvent(event);
            return;
            }
        if (event->key() == Qt::Key_Escape) {
            m_autoCompleter->hideCompletion();
            return;
        }
    }

    if (event->key() == Qt::Key_Tab) {
        if (m_spacesInsteadOfTabs) {
            QTextCursor cur = textCursor();
            const int col = cur.columnNumber();
            const int spaces = m_tabSize - (col % m_tabSize);
            cur.insertText(QString(spaces, ' '));
            return;
        }
    } else if (event->key() == Qt::Key_Backtab) {
        QTextCursor cur = textCursor();
        cur.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        const QString leading = cur.selectedText();
        int toRemove = 0;
        for (QChar c: leading) {
            if (c == ' ' && toRemove < m_tabSize)
                ++toRemove;
            else
                break;
        }
        if (toRemove > 0) {
            cur.movePosition(QTextCursor::StartOfLine);
            for (int i = 0; i < toRemove; ++i) cur.deleteChar();
        }
        return;
    } else if ((event->key() == Qt::Key_Return ||
                event->key() == Qt::Key_Enter) &&
               m_autoIndent) {
        const QString line = textCursor().block().text();
        QString indent;
        for (QChar c: line) {
            if (c == ' ' || c == '\t')
                indent += c;
            else
                break;
        }
        QPlainTextEdit::keyPressEvent(event);
        textCursor().insertText(indent);

        if (m_codeFoldingEnabled) {
            QTimer::singleShot(100, this, &CodeEditor::refreshFolds);
        }
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    if (m_codeFoldingEnabled) {
        if (!event->text().isEmpty() ||
            event->key() == Qt::Key_Backspace ||
            event->key() == Qt::Key_Delete) {
            QTimer::singleShot(100, this, &CodeEditor::refreshFolds);
        }
    }

    if (m_autoCompleter && !event->text().isEmpty() && event->text().at(0).isLetterOrNumber()) {
        QTimer::singleShot(0, this, &CodeEditor::updateAutoCompleteWords);
    }
}

void CodeEditor::toggleBookmark(int line) {
    if (m_bookmarks.contains(line))
        m_bookmarks.remove(line);
    else
        m_bookmarks.insert(line);
    m_lineNumberArea->update();
    onHighlightCurrentLine();
}

void CodeEditor::nextBookmark() {
    if (m_bookmarks.isEmpty()) return;
    int current = textCursor().blockNumber();
    QList<int> sorted = m_bookmarks.values();
    std::sort(sorted.begin(), sorted.end());
    for (int line: sorted)
        if (line > current) {
            goToLine(line);
            return;
        }
    goToLine(sorted.first());
}

void CodeEditor::prevBookmark() {
    if (m_bookmarks.isEmpty()) return;
    const int current = textCursor().blockNumber();
    QList<int> sorted = m_bookmarks.values();
    std::sort(sorted.begin(), sorted.end());
    for (int i = sorted.size() - 1; i >= 0; --i)
        if (sorted[i] < current) {
            goToLine(sorted[i]);
            return;
        }
    goToLine(sorted.last());
}

void CodeEditor::clearBookmarks() {
    m_bookmarks.clear();
    m_lineNumberArea->update();
    onHighlightCurrentLine();
}

void CodeEditor::setBookmarks(const QSet<int> &bm) {
    m_bookmarks = bm;
    m_lineNumberArea->update();
    onHighlightCurrentLine();
}

void CodeEditor::goToLine(int lineNumber) {
    QTextBlock block = document()->findBlockByNumber(lineNumber);
    if (!block.isValid()) return;
    setTextCursor(QTextCursor(block));
    centerCursor();
}

int CodeEditor::lineNumberAreaWidth() const {
    if (!m_showLineNumbers) return 0;

    int digits = 1, max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int width = kBookmarkColumnWidth + kFoldColumnWidth +
                fontMetrics().horizontalAdvance('9') * digits + 10;
    return width;
}

void CodeEditor::resizeEvent(QResizeEvent *event) {
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(const QPaintEvent *event) const {
    if (!m_showLineNumbers) return;

    QPainter painter(m_lineNumberArea);
    QPalette pal = palette();

    painter.fillRect(event->rect(), pal.color(QPalette::AlternateBase));

    painter.setPen(QPen(pal.color(QPalette::Mid), 1));
    painter.drawLine(m_lineNumberArea->width() - 1, event->rect().top(),
                     m_lineNumberArea->width() - 1, event->rect().bottom());

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
            qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int lineH = fontMetrics().height();
    const int curLine = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            int xOffset = 0;

            // Draw bookmark indicator (if present)
            if (m_bookmarks.contains(blockNumber)) {
                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(0xf1c40f));
                const int ds = 9, dx = xOffset + (16 - ds) / 2, dy = top + (lineH - ds) / 2;
                painter.drawEllipse(dx, dy, ds, ds);
                painter.setRenderHint(QPainter::Antialiasing, false);
            }
            xOffset += kBookmarkColumnWidth;

            // Draw fold indicator (if present)
            if (m_codeFoldingEnabled && m_foldManager && m_foldManager->isFoldStart(blockNumber)) {
                bool isFolded = m_foldManager->isFolded(blockNumber);

                if (m_gutterHoverLine == blockNumber && m_gutterHoverArea == GutterArea::Fold) {
                    QColor hover = pal.color(QPalette::Highlight);
                    hover.setAlpha(35);
                    painter.fillRect(QRect(xOffset, top, kFoldColumnWidth, lineH), hover);
                }

                painter.setRenderHint(QPainter::Antialiasing, true);
                painter.setPen(QPen(pal.color(QPalette::Mid), 1));
                painter.setBrush(pal.color(QPalette::Base));

                // Draw a small box
                const int boxSize = 10;
                const int boxX = xOffset + (kFoldColumnWidth - boxSize) / 2;
                const int boxY = top + (lineH - boxSize) / 2;
                painter.drawRect(boxX, boxY, boxSize, boxSize);

                // Draw +/- inside the box
                painter.setPen(QPen(pal.color(QPalette::Text), 1.5));
                const int centerX = boxX + boxSize / 2;
                const int centerY = boxY + boxSize / 2;
                const int lineLen = 4;

                // Horizontal line
                painter.drawLine(centerX - lineLen / 2, centerY,
                                 centerX + lineLen / 2, centerY);

                // Vertical line
                if (isFolded) {
                    painter.drawLine(centerX, centerY - lineLen / 2,
                                     centerX, centerY + lineLen / 2);
                }

                painter.setRenderHint(QPainter::Antialiasing, false);
            }
            xOffset += kFoldColumnWidth;

            // Draw line number
            painter.setPen(blockNumber == curLine
                               ? pal.color(QPalette::Text)
                               : pal.color(QPalette::Disabled, QPalette::Text));

            painter.drawText(xOffset, top, m_lineNumberArea->width() - xOffset - 6, lineH,
                             Qt::AlignRight, QString::number(blockNumber + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::lineNumberAreaMousePress(const QMouseEvent *event) {
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
            qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int lineH = fontMetrics().height();

    while (block.isValid()) {
        if (event->pos().y() >= top && event->pos().y() < bottom) {
            if (m_codeFoldingEnabled && m_foldManager &&
                event->pos().x() >= kBookmarkColumnWidth &&
                event->pos().x() < kBookmarkColumnWidth + kFoldColumnWidth &&
                m_foldManager->isFoldStart(blockNumber)) {
                m_foldManager->toggleFold(blockNumber);
                viewport()->update();
                m_lineNumberArea->update();

                // Immediately check and fix cursor position if it's now in a folded region
                onCursorPositionChanged();

                return;
            }
            else if (event->pos().x() < kBookmarkColumnWidth) {
                toggleBookmark(blockNumber);
                return;
            }
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::setGutterHover(int line, GutterArea area) {
    if (m_gutterHoverLine == line && m_gutterHoverArea == area) return;
    m_gutterHoverLine = line;
    m_gutterHoverArea = area;
    if (m_lineNumberArea) m_lineNumberArea->update();
}

void CodeEditor::lineNumberAreaMouseMove(const QMouseEvent* event) {
    if (!m_showLineNumbers) return;

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
        qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid()) {
        if (event->pos().y() >= top && event->pos().y() < bottom) {
            if (m_codeFoldingEnabled && m_foldManager &&
                event->pos().x() >= kBookmarkColumnWidth &&
                event->pos().x() < kBookmarkColumnWidth + kFoldColumnWidth &&
                m_foldManager->isFoldStart(blockNumber)) {
                setGutterHover(blockNumber, GutterArea::Fold);
            }
            else if (event->pos().x() < kBookmarkColumnWidth) {
                setGutterHover(blockNumber, GutterArea::Bookmark);
            }
            else {
                setGutterHover(-1, GutterArea::None);
            }
            return;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    setGutterHover(-1, GutterArea::None);
}

void CodeEditor::lineNumberAreaLeave() {
    setGutterHover(-1, GutterArea::None);
}

void CodeEditor::lineNumberAreaContextMenuEvent(QContextMenuEvent* event) {
    if (!event) return;
    if (!m_codeFoldingEnabled || !m_foldManager) return;

    QMenu menu;

    // If the user right-clicked on a fold start line, offer toggle.
    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top =
        qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid()) {
        if (event->pos().y() >= top && event->pos().y() < bottom) break;
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }

    if (block.isValid() && m_foldManager->isFoldStart(blockNumber)) {
        QAction* toggle = menu.addAction(
            m_foldManager->isFolded(blockNumber) ? "Unfold" : "Fold");
        connect(toggle, &QAction::triggered, this, [this, blockNumber]() {
            m_foldManager->toggleFold(blockNumber);
            viewport()->update();
            m_lineNumberArea->update();
            onCursorPositionChanged();
        });
        menu.addSeparator();
    }

    QAction* foldAll = menu.addAction("Fold All");
    QAction* unfoldAll = menu.addAction("Unfold All");
    connect(foldAll, &QAction::triggered, this, [this]() {
        for (int line : m_foldManager->foldableLines()) m_foldManager->foldRegion(line);
        viewport()->update();
        m_lineNumberArea->update();
        onCursorPositionChanged();
    });
    connect(unfoldAll, &QAction::triggered, this, [this]() {
        for (int line : m_foldManager->foldableLines()) m_foldManager->unfoldRegion(line);
        viewport()->update();
        m_lineNumberArea->update();
    });

    menu.exec(event->globalPos());
}

void CodeEditor::updateLineNumberAreaWidth(int) {
    setViewportMargins(m_showLineNumbers ? lineNumberAreaWidth() : 0, 0, 0, 0);
}

void CodeEditor::onHighlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> extras;

    int currentLine = textCursor().blockNumber();

    if (!m_bookmarks.isEmpty()) {
        QTextCharFormat bookmarkFmt;
        bookmarkFmt.setBackground(QColor(241, 196, 15, 60));
        bookmarkFmt.setProperty(QTextFormat::FullWidthSelection, true);

        for (int line: m_bookmarks) {
            if (line == currentLine) continue;

            QTextBlock block = document()->findBlockByNumber(line);
            if (!block.isValid()) continue;
            QTextEdit::ExtraSelection sel;
            sel.format = bookmarkFmt;
            sel.cursor = QTextCursor(block);
            sel.cursor.clearSelection();
            extras << sel;
        }
    }

    if (!isReadOnly() && m_doHighlightCurrentLine) {
        QTextEdit::ExtraSelection sel;

        QPalette pal = palette();
        QColor highlight = pal.color(QPalette::Highlight);
        highlight.setAlpha(40);

        sel.format.setBackground(highlight);
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = textCursor();
        sel.cursor.clearSelection();
        extras << sel;
    }

    setExtraSelections(extras);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
    if (dy)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(),
                                 rect.height());
    if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

void CodeEditor::paintEvent(QPaintEvent *event) {
    QPlainTextEdit::paintEvent(event);

    // Draw underline for folded lines
    if (!m_codeFoldingEnabled || !m_foldManager) return;

    QPainter painter(viewport());

    QColor underlineColor;
    if (isSystemDarkTheme()) {
        // Dark theme
        underlineColor = QColor(180, 180, 180);
    } else {
        // Light theme
        underlineColor = QColor(100, 100, 100);
    }

    painter.setPen(QPen(underlineColor, 2, Qt::DotLine));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            // Check if this line has a folded region
            if (m_foldManager->isFoldStart(blockNumber) &&
                m_foldManager->isFolded(blockNumber)) {
                // Draw dotted underline across the entire editor width
                int lineY = bottom - 1;
                painter.drawLine(0, lineY, viewport()->width(), lineY);
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::onCursorPositionChanged() {
    if (!m_codeFoldingEnabled || !m_foldManager) return;

    QTextCursor cursor = textCursor();
    int currentLine = cursor.blockNumber();

    // Check if cursor is in a folded region
    if (m_foldManager->isLineInFoldedRegion(currentLine)) {
        // Find the fold start line
        for (auto it = m_foldManager->foldableLines().constBegin();
             it != m_foldManager->foldableLines().constEnd(); ++it) {
            FoldRegion region = m_foldManager->getFoldAt(*it);
            if (region.isFolded &&
                currentLine > region.startLine &&
                currentLine <= region.endLine) {
                // Move cursor to the fold start line
                QTextBlock foldBlock = document()->findBlockByNumber(region.startLine);
                if (foldBlock.isValid()) {
                    QTextCursor newCursor(foldBlock);
                    newCursor.movePosition(QTextCursor::EndOfLine);
                    setTextCursor(newCursor);
                }
                break;
            }
        }
    }
}

void CodeEditor::updateAutoCompleteWords() {
    if (!m_autoCompleter) return;
    m_autoCompleter->updateWordList(toPlainText());
    m_autoCompleter->showCompletion();
}
