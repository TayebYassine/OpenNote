#include "EditorTabWidget.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QMessageBox>
#include <QStyle>
#include <QTabBar>
#include <QTextStream>
#include <QToolButton>

#include "AppDatabase.h"

EditorTabWidget::EditorTabWidget(QWidget* parent) : QTabWidget(parent) {
    setTabsClosable(false);
    setMovable(true);
    setDocumentMode(true);

    setStyleSheet(
        "QTabWidget::pane  { border: none; }"
        "QTabBar::tab {"
        "  padding: 6px 14px;"
        "  border: none; border-right: 1px solid;"
        "  min-width: 80px;"
        "}"
        "QTabBar::tab:selected { border-top: 2px solid #007acc; }"
        "QTabBar::tab:hover:!selected { }");

    connect(this, &QTabWidget::tabCloseRequested, this,
            &EditorTabWidget::onTabCloseRequested);
    connect(this, &QTabWidget::currentChanged, this,
            &EditorTabWidget::onCurrentChanged);
}

void EditorTabWidget::applySettingsToEditor(CodeEditor* editor) const {
    editor->setTabSize(m_tabSize);
    editor->setAutoIndent(m_autoIndent);
}

void EditorTabWidget::setTabSize(int size) {
    m_tabSize = qMax(1, qMin(size, 16));
    for (int i = 0; i < count(); ++i)
        if (auto* ed = editorAt(i)) ed->setTabSize(m_tabSize);
}

void EditorTabWidget::setAutoIndent(bool on) {
    m_autoIndent = on;
    for (int i = 0; i < count(); ++i)
        if (auto* ed = editorAt(i)) ed->setAutoIndent(m_autoIndent);
}

CodeEditor* EditorTabWidget::currentEditor() const {
    return qobject_cast<CodeEditor*>(currentWidget());
}

CodeEditor* EditorTabWidget::editorAt(int index) const {
    return qobject_cast<CodeEditor*>(widget(index));
}

QIcon EditorTabWidget::iconForFile(const QString& filePath) const {
    if (filePath.isEmpty())
        return QIcon::fromTheme("document-new", QIcon::fromTheme("text-x-generic"));
    static QFileIconProvider provider;
    return provider.icon(QFileInfo(filePath));
}

int EditorTabWidget::findTabByPath(const QString& filePath) const {
    for (int i = 0; i < count(); ++i) {
        auto* ed = editorAt(i);
        if (ed && ed->filePath() == filePath) return i;
    }
    return -1;
}

void EditorTabWidget::updateTabTitle(int index) {
    auto* ed = editorAt(index);
    if (!ed) return;

    const QString name = ed->filePath().isEmpty()
                             ? "untitled"
                             : QFileInfo(ed->filePath()).fileName();
    const bool dirty = ed->document()->isModified();
    setTabText(index, dirty ? name + "*" : name);
    setTabIcon(index, iconForFile(ed->filePath()));
    setTabToolTip(index, ed->filePath().isEmpty() ? "New file" : ed->filePath());
}

void EditorTabWidget::attachCloseButton(int index) {
    auto* btn = new QToolButton(tabBar());
    btn->setText("✕");
    btn->setFixedSize(16, 16);
    btn->setCursor(Qt::ArrowCursor);
    btn->setStyleSheet(
        "QToolButton {"
        "  color: #888; border: none; border-radius: 3px;"
        "  font-size: 11px; padding: 0;"
        "  background: transparent;"
        "}"
        "QToolButton:hover   { color: white; background: #c0392b; }"
        "QToolButton:pressed { background: #922b21; }");
    connect(btn, &QToolButton::clicked, this, [this, btn]() {
        for (int i = 0; i < count(); ++i) {
            if (tabBar()->tabButton(i, QTabBar::RightSide) == btn) {
                closeTab(i);
                return;
            }
        }
    });
    tabBar()->setTabButton(index, QTabBar::RightSide, btn);
}

void EditorTabWidget::connectEditor(CodeEditor* editor) {
    connect(editor->document(), &QTextDocument::modificationChanged, this,
            [this, editor](bool) {
                const int idx = indexOf(editor);
                if (idx >= 0) updateTabTitle(idx);
            });

    connect(editor, &CodeEditor::cursorPositionChanged, this,
            &EditorTabWidget::onCursorMoved);
}

void EditorTabWidget::newFile() {
    auto* editor = new CodeEditor(this);
    applySettingsToEditor(editor);
    connectEditor(editor);
    const int idx = addTab(editor, QIcon::fromTheme("document-new"), "untitled");
    setCurrentIndex(idx);
    updateTabTitle(idx);
    attachCloseButton(idx);
    editor->setFocus();
}

void EditorTabWidget::openFile(const QString& filePath) {
    const int existing = findTabByPath(filePath);
    if (existing >= 0) {
        setCurrentIndex(existing);
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open Failed", "Could not open:\n" + filePath);
        return;
    }

    // Binary detection
    const QByteArray sample = file.read(8192);
    if (sample.contains('\0')) {
        QMessageBox::warning(this, "Cannot Open File",
                             "'" + QFileInfo(filePath).fileName() +
                             "' appears to be a binary file\n"
                             "and cannot be opened in the editor.");
        file.close();
        return;
    }

    file.seek(0);
    QTextStream in(&file);
    in.setAutoDetectUnicode(true);
    const QString content = in.readAll();

    auto* editor = new CodeEditor(this);
    applySettingsToEditor(editor);
    editor->setFilePath(filePath);
    editor->setPlainText(content);
    editor->document()->setModified(false);

    const auto bms = AppDatabase::instance().bookmarksForFile(filePath);
    editor->setBookmarks(QSet<int>(bms.begin(), bms.end()));

    connectEditor(editor);

    const int idx =
        addTab(editor, iconForFile(filePath), QFileInfo(filePath).fileName());
    setCurrentIndex(idx);
    updateTabTitle(idx);
    attachCloseButton(idx);
    editor->setFocus();

    AppDatabase::instance().addRecentFile(filePath);
    AppDatabase::instance().save();
}

bool EditorTabWidget::closeTab(int index) {
    auto* editor = editorAt(index);
    if (!editor) return true;

    if (editor->document()->isModified()) {
        const QString name = editor->filePath().isEmpty()
                                 ? "untitled"
                                 : QFileInfo(editor->filePath()).fileName();

        auto btn = QMessageBox::question(
            this, "Unsaved Changes",
            QString("'<b>%1</b>' has unsaved changes.<br>Save before closing?")
            .arg(name),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
            QMessageBox::Save);

        if (btn == QMessageBox::Cancel) return false;
        if (btn == QMessageBox::Save && !saveTab(index)) return false;
    }

    if (!editor->filePath().isEmpty()) {
        const auto& bms = editor->bookmarks();
        AppDatabase::instance().setBookmarks(editor->filePath(),
                                             QList<int>(bms.begin(), bms.end()));
        AppDatabase::instance().save();
    }

    QWidget* closeBtn = tabBar()->tabButton(index, QTabBar::RightSide);
    if (closeBtn) {
        tabBar()->setTabButton(index, QTabBar::RightSide, nullptr);
        delete closeBtn;
    }

    removeTab(index);
    delete editor;
    return true;
}

bool EditorTabWidget::closeAllTabs() {
    while (count() > 0)
        if (!closeTab(0)) return false;
    return true;
}

bool EditorTabWidget::saveTab(int index) {
    auto* editor = editorAt(index);
    if (!editor) return false;

    if (editor->filePath().isEmpty()) return saveTabAs(index);

    QFile file(editor->filePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Failed",
                             "Could not write to:\n" + editor->filePath());
        return false;
    }

    QTextStream out(&file);
    out << editor->toPlainText();
    editor->document()->setModified(false);
    updateTabTitle(index);
    return true;
}

bool EditorTabWidget::saveTabAs(int index) {
    auto* editor = editorAt(index);
    if (!editor) return false;

    const QString path = QFileDialog::getSaveFileName(
        this, "Save As",
        editor->filePath().isEmpty() ? QDir::homePath() : editor->filePath(),
        "All Files (*)");
    if (path.isEmpty()) return false;

    editor->setFilePath(path);
    return saveTab(index);
}

void EditorTabWidget::overrideLanguage(Language lang) {
    auto* ed = currentEditor();
    if (ed) ed->setLanguage(lang);
    emit currentFileChanged(ed ? ed->filePath() : QString(), lang);
}

void EditorTabWidget::updateRecentFiles() {
    for (int i = 0; i < count(); ++i) {
        auto* ed = editorAt(i);
        if (ed && !ed->filePath().isEmpty())
            AppDatabase::instance().addRecentFile(ed->filePath());
    }
    AppDatabase::instance().save();
}

void EditorTabWidget::onTabCloseRequested(int index) { closeTab(index); }

void EditorTabWidget::onCurrentChanged(int index) {
    auto* ed = editorAt(index);
    emit currentFileChanged(ed ? ed->filePath() : QString(),
                            ed ? ed->language() : Language::None);
    if (ed) {
        const QTextCursor c = ed->textCursor();
        emit cursorPositionChanged(c.blockNumber() + 1, c.columnNumber() + 1);
    }
}

void EditorTabWidget::onCursorMoved() {
    auto* ed = qobject_cast<CodeEditor*>(sender());
    if (!ed || ed != currentEditor()) return;
    const QTextCursor c = ed->textCursor();
    emit cursorPositionChanged(c.blockNumber() + 1, c.columnNumber() + 1);
}
