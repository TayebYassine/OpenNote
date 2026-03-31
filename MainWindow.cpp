#include "MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>

#include "AboutDialog.h"
#include "AppDatabase.h"
#include "FindReplaceDialog.h"
#include "PreferencesDialog.h"

// Constructor

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("OpenNote");
    setMinimumSize(900, 600);
    resize(1200, 750);

    setupMenuBar();
    setupCentralWidget();
    setupStatusBar();

    // Autosave timer
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::onAutoSaveTick);

    // Restore recent files as tabs
    if (AppDatabase::instance().openRecentFiles()) {
        for (const QString& path : AppDatabase::instance().recentFiles()) {
            if (QFile::exists(path)) m_tabWidget->openFile(path);
        }
    }

    if (m_tabWidget->count() == 0) m_tabWidget->newFile();

    applyAllSettings();
}

// Menu Bar

void MainWindow::setupMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu("&File");

    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentNew), "&New",
                        this, &MainWindow::onNewFile, QKeySequence::New);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                        "&Open File…", this, &MainWindow::onOpenFile,
                        QKeySequence::Open);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen),
                        "Open &Folder…", this, &MainWindow::onOpenFolder,
                        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
    fileMenu->addSeparator();
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSave), "&Save",
                        this, &MainWindow::onSave, QKeySequence::Save);
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentSaveAs),
                        "Save &As…", this, &MainWindow::onSaveAs,
                        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    fileMenu->addAction("Save A&ll", this, &MainWindow::onSaveAll,
                        QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentRevert),
                        "&Revert to Saved", this, &MainWindow::onRevertFile);
    fileMenu->addSeparator();

    m_recentMenu = fileMenu->addMenu(
        QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpenRecent), "Recent &Files");
    refreshRecentFilesMenu();

    fileMenu->addSeparator();
    fileMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::WindowClose),
                        "&Close Tab", this, &MainWindow::onCloseTab,
                        QKeySequence::Close);
    fileMenu->addAction("Close All Tabs", this, &MainWindow::onCloseAllTabs);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", this, &MainWindow::onQuit, QKeySequence::Quit);

    QMenu* editMenu = menuBar()->addMenu("&Edit");

    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditUndo), "&Undo",
                        this, &MainWindow::onUndo, QKeySequence::Undo);
    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditRedo), "&Redo",
                        this, &MainWindow::onRedo, QKeySequence::Redo);
    editMenu->addSeparator();
    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditCut), "Cu&t", this,
                        &MainWindow::onCut, QKeySequence::Cut);
    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditCopy), "&Copy",
                        this, &MainWindow::onCopy, QKeySequence::Copy);
    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditPaste), "&Paste",
                        this, &MainWindow::onPaste, QKeySequence::Paste);
    editMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::EditSelectAll),
                        "Select &All", this, &MainWindow::onSelectAll,
                        QKeySequence::SelectAll);
    editMenu->addSeparator();
    editMenu->addAction("&Find and Replace…", this, &MainWindow::onFindReplace,
                        QKeySequence(Qt::CTRL | Qt::Key_H));

    QMenu* viewMenu = menuBar()->addMenu("&View");

    QAction* toggleTree = viewMenu->addAction("Toggle &File Explorer", this,
                                              &MainWindow::onToggleFileTree,
                                              QKeySequence(Qt::CTRL | Qt::Key_B));
    toggleTree->setCheckable(true);
    toggleTree->setChecked(true);
    viewMenu->addSeparator();
    viewMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomIn), "Zoom &In",
                        this, &MainWindow::onZoomIn, QKeySequence::ZoomIn);
    viewMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomOut), "Zoom &Out",
                        this, &MainWindow::onZoomOut, QKeySequence::ZoomOut);
    viewMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ZoomFitBest),
                        "&Default Zoom", this, &MainWindow::onZoomReset,
                        QKeySequence(Qt::CTRL | Qt::Key_0));
    viewMenu->addSeparator();
    m_fontMenu = viewMenu->addMenu("&Font");
    m_fontGroup = new QActionGroup(this);
    m_fontGroup->setExclusive(true);
    buildFontMenu();

    m_languageMenu = menuBar()->addMenu("&Language");
    m_langGroup = new QActionGroup(this);
    m_langGroup->setExclusive(true);
    buildLanguageMenu();

    QMenu* bmMenu = menuBar()->addMenu("&Bookmarks");
    bmMenu->addAction("&Toggle", this, &MainWindow::onToggleBookmark,
                      QKeySequence(Qt::CTRL | Qt::Key_F2));
    bmMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoNext), "&Next", this,
                      &MainWindow::onNextBookmark, QKeySequence(Qt::Key_F2));
    bmMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious), "&Previous",
                      this, &MainWindow::onPrevBookmark,
                      QKeySequence(Qt::SHIFT | Qt::Key_F2));
    bmMenu->addSeparator();
    bmMenu->addAction("Clear All", this, &MainWindow::onClearBookmarks);

    QMenu* helpMenu = menuBar()->addMenu("&Help");
    helpMenu->addAction(QIcon::fromTheme("emblem-system"), "&Preferences…", this,
                        &MainWindow::onPreferences,
                        QKeySequence(Qt::CTRL | Qt::Key_Comma));
    helpMenu->addSeparator();
    helpMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout),
                        "&About OpenNote", this, &MainWindow::onAbout);
    helpMenu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::HelpAbout),
                        "About &Qt", qApp, &QApplication::aboutQt);
}

// Central widget and status bar

void MainWindow::setupCentralWidget() {
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_fileTree = new FileTreeWidget(this);
    m_tabWidget = new EditorTabWidget(this);

    m_splitter->addWidget(m_fileTree);
    m_splitter->addWidget(m_tabWidget);
    m_splitter->setStretchFactor(0, 1);
    m_splitter->setStretchFactor(1, 4);
    m_splitter->setSizes({220, 980});
    setCentralWidget(m_splitter);

    connect(m_fileTree, &FileTreeWidget::fileSelected, this,
            &MainWindow::onFileSelected);
    connect(m_tabWidget, &EditorTabWidget::currentFileChanged, this,
            &MainWindow::onCurrentFileChanged);
    connect(m_tabWidget, &EditorTabWidget::cursorPositionChanged, this,
            &MainWindow::onCursorPositionChanged);
    connect(m_fileTree, &FileTreeWidget::statusMessage, statusBar(),
            &QStatusBar::showMessage);
}

void MainWindow::setupStatusBar() {
    m_langLabel = new QLabel("Plain Text", this);
    m_fileUnicode = new QLabel("?", this);
    m_cursorLabel = new QLabel("Ln 1, Col 1", this);
    m_langLabel->setStyleSheet("padding: 0 8px;");
    m_fileUnicode->setStyleSheet("padding: 0 8px;");
    m_cursorLabel->setStyleSheet("padding: 0 8px;");
    statusBar()->addPermanentWidget(m_langLabel);
    statusBar()->addPermanentWidget(m_fileUnicode);
    statusBar()->addPermanentWidget(m_cursorLabel);
    statusBar()->showMessage("Ready");
    statusBar()->setStyleSheet("QStatusBar::item { border: none; }");
}

// Settings application

void MainWindow::applyAllSettings() {
    const AppDatabase& db = AppDatabase::instance();

    // Font
    if (!db.editorFontFamily().isEmpty())
        applyFontToAllEditors(db.editorFontFamily(), db.editorFontSize());

    // Per-editor view and behaviour settings
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto* ed = m_tabWidget->editorAt(i);
        if (!ed) continue;
        ed->setShowLineNumbers(db.showLineNumbers());
        ed->setHighlightCurrentLine(db.highlightCurrentLine());
        ed->setWordWrap(db.wordWrap());
        ed->setTabSize(db.tabSize());
        ed->setAutoIndent(db.autoIndent());
        ed->setSpacesInsteadOfTabs(db.spacesInsteadOfTabs());
        ed->applyTheme(db.syntaxTheme());
    }

    // EditorTabWidget-level settings
    m_tabWidget->setTabSize(db.tabSize());
    m_tabWidget->setAutoIndent(db.autoIndent());

    // Status bar visibility
    statusBar()->setVisible(db.showStatusBar());

    // Autosave timer
    if (db.autoSave()) {
        m_autoSaveTimer->setInterval(db.autoSaveIntervalMin() * 60 * 1000);
        m_autoSaveTimer->start();
    }
    else {
        m_autoSaveTimer->stop();
    }
}

// Close event

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_tabWidget->closeAllTabs()) {
        AppDatabase::instance().save();
        event->accept();
    }
    else {
        event->ignore();
    }
}

// File menu slots

void MainWindow::onNewFile() { m_tabWidget->newFile(); }

void MainWindow::onOpenFile() {
    const QString path = QFileDialog::getOpenFileName(
        this, "Open File", QDir::homePath(), "All Files (*)");
    if (!path.isEmpty()) m_tabWidget->openFile(path);
}

void MainWindow::onOpenFolder() {
    const QString dir =
        QFileDialog::getExistingDirectory(this, "Open Folder", QDir::homePath());
    if (!dir.isEmpty()) m_fileTree->setRootPath(dir);
}

void MainWindow::onSave() { m_tabWidget->saveTab(m_tabWidget->currentIndex()); }

void MainWindow::onSaveAs() {
    m_tabWidget->saveTabAs(m_tabWidget->currentIndex());
}

void MainWindow::onSaveAll() {
    for (int i = 0; i < m_tabWidget->count(); ++i) m_tabWidget->saveTab(i);
    statusBar()->showMessage("All files saved.", 6000);
}

void MainWindow::onRevertFile() {
    auto* ed = m_tabWidget->currentEditor();
    if (!ed || ed->filePath().isEmpty()) return;

    if (ed->document()->isModified()) {
        if (QMessageBox::question(
            this, "Revert",
            QString("Discard changes and reload '<b>%1</b>' from disk?")
            .arg(QFileInfo(ed->filePath()).fileName()),
            QMessageBox::Yes | QMessageBox::Cancel) != QMessageBox::Yes)
            return;
    }

    QFile file(ed->filePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    ed->setPlainText(QTextStream(&file).readAll());
    ed->document()->setModified(false);
    statusBar()->showMessage("Reverted.", 6000);
}

void MainWindow::onCloseTab() {
    m_tabWidget->closeTab(m_tabWidget->currentIndex());
}

void MainWindow::onCloseAllTabs() { m_tabWidget->closeAllTabs(); }
void MainWindow::onQuit() { close(); }

void MainWindow::onOpenRecentFile(const QString& path) {
    if (QFile::exists(path)) {
        m_tabWidget->openFile(path);
    }
    else {
        QMessageBox::warning(this, "File Not Found", "Could not find:\n" + path);
        AppDatabase::instance().removeRecentFile(path);
        AppDatabase::instance().save();
        refreshRecentFilesMenu();
    }
}

// Edit menu slots

void MainWindow::onUndo() {
    if (auto* e = m_tabWidget->currentEditor()) e->undo();
}

void MainWindow::onRedo() {
    if (auto* e = m_tabWidget->currentEditor()) e->redo();
}

void MainWindow::onCut() {
    if (auto* e = m_tabWidget->currentEditor()) e->cut();
}

void MainWindow::onCopy() {
    if (auto* e = m_tabWidget->currentEditor()) e->copy();
}

void MainWindow::onPaste() {
    if (auto* e = m_tabWidget->currentEditor()) e->paste();
}

void MainWindow::onSelectAll() {
    if (auto* e = m_tabWidget->currentEditor()) e->selectAll();
}

void MainWindow::onFindReplace() {
    if (!m_findReplace) {
        m_findReplace = new FindReplaceDialog(this);
        // Keep the dialog's editor pointer in sync with the active tab
        connect(m_tabWidget, &EditorTabWidget::currentFileChanged, this, [this]() {
            m_findReplace->setEditor(m_tabWidget->currentEditor());
        });
    }
    m_findReplace->setEditor(m_tabWidget->currentEditor());
    m_findReplace->show();
    m_findReplace->raise();
    m_findReplace->activateWindow();
}

// View menu slots

void MainWindow::onToggleFileTree() {
    m_fileTree->setVisible(!m_fileTree->isVisible());
}

void MainWindow::onZoomIn() {
    if (auto* e = m_tabWidget->currentEditor()) e->zoomIn(1);
}

void MainWindow::onZoomOut() {
    if (auto* e = m_tabWidget->currentEditor()) e->zoomOut(1);
}

void MainWindow::onZoomReset() {
    if (auto* e = m_tabWidget->currentEditor()) {
        QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        f.setPointSize(12);
        e->setFont(f);
        e->setTabSize(AppDatabase::instance().tabSize());
    }
}

void MainWindow::onFontSelected(const QString& family) {
    const int size = AppDatabase::instance().editorFontSize();
    applyFontToAllEditors(family, size);
    AppDatabase::instance().setEditorFont(family, size);
    AppDatabase::instance().save();
    statusBar()->showMessage("Font changed to " + family, 4000);
}

// Bookmark slots

void MainWindow::onToggleBookmark() {
    if (auto* e = m_tabWidget->currentEditor())
        e->toggleBookmark(e->textCursor().blockNumber());
}

void MainWindow::onNextBookmark() {
    if (auto* e = m_tabWidget->currentEditor()) e->nextBookmark();
}

void MainWindow::onPrevBookmark() {
    if (auto* e = m_tabWidget->currentEditor()) e->prevBookmark();
}

void MainWindow::onClearBookmarks() {
    if (auto* e = m_tabWidget->currentEditor()) e->clearBookmarks();
}

// Language override

void MainWindow::onLanguageOverride(Language lang) {
    m_tabWidget->overrideLanguage(lang);
    m_langLabel->setText(languageName(lang));
    statusBar()->showMessage("Language set to " + languageName(lang), 6000);
}

// Preferences & About

void MainWindow::onPreferences() {
    PreferencesDialog dlg(this);
    connect(&dlg, &PreferencesDialog::settingsChanged, this,
            &MainWindow::applyAllSettings);
    dlg.exec();
}

void MainWindow::onAbout() {
    AboutDialog dlg(this);
    dlg.exec();
}

// Autosave tick

void MainWindow::onAutoSaveTick() {
    int saved = 0;
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto* ed = m_tabWidget->editorAt(i);
        if (ed && ed->document()->isModified() && !ed->filePath().isEmpty()) {
            m_tabWidget->saveTab(i);
            ++saved;
        }
    }
    if (saved > 0)
        statusBar()->showMessage(QString("Auto-saved %1 file(s).").arg(saved),
                                 3000);
}

// Status bar updates

void MainWindow::onCurrentFileChanged(const QString& filePath, Language lang) {
    setWindowTitle(filePath.isEmpty()
                       ? "OpenNote — untitled"
                       : "OpenNote — " + QFileInfo(filePath).fileName());

    m_langLabel->setText(languageName(lang));

    m_fileUnicode->setText(autoDetectFileUnicode(filePath));

    // Sync Language menu checkmark
    const QString name = languageName(lang);
    for (QAction* a : m_langGroup->actions()) a->setChecked(a->text() == name);

    // Update Find & Replace dialog if open
    if (m_findReplace) m_findReplace->setEditor(m_tabWidget->currentEditor());

    // Apply all settings to the newly activated editor
    const AppDatabase& db = AppDatabase::instance();
    if (auto* ed = m_tabWidget->currentEditor()) {
        ed->setShowLineNumbers(db.showLineNumbers());
        ed->setHighlightCurrentLine(db.highlightCurrentLine());
        ed->setWordWrap(db.wordWrap());
        ed->setTabSize(db.tabSize());
        ed->setAutoIndent(db.autoIndent());
        ed->setSpacesInsteadOfTabs(db.spacesInsteadOfTabs());
        ed->applyTheme(db.syntaxTheme());
        if (!db.editorFontFamily().isEmpty()) {
            QFont f(db.editorFontFamily());
            f.setPointSize(db.editorFontSize());
            ed->setFont(f);
        }
    }

    refreshRecentFilesMenu();
}

void MainWindow::onCursorPositionChanged(int line, int col) {
    m_cursorLabel->setText(QString("Ln %1, Col %2").arg(line).arg(col));
}

void MainWindow::onFileSelected(const QString& filePath) {
    m_tabWidget->openFile(filePath);
    statusBar()->showMessage("Opened: " + filePath, 3000);
}

// Dynamic menu builders

void MainWindow::refreshRecentFilesMenu() {
    m_recentMenu->clear();
    const QStringList recent = AppDatabase::instance().recentFiles();
    if (recent.isEmpty()) {
        QAction* empty = m_recentMenu->addAction("(no recent files)");
        empty->setEnabled(false);
        return;
    }
    for (const QString& path : recent) {
        QAction* act =
            m_recentMenu->addAction(m_tabWidget->iconForFile(path), QFileInfo(path).fileName() + "  \t" + path);
        connect(act, &QAction::triggered, this,
                [this, path]() { onOpenRecentFile(path); });
    }
    m_recentMenu->addSeparator();
    m_recentMenu->addAction(
        QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        "Clear Recent Files", this, [this]() {
            for (const QString& p : AppDatabase::instance().recentFiles())
                AppDatabase::instance().removeRecentFile(p);
            AppDatabase::instance().save();
            refreshRecentFilesMenu();
        });
}

void MainWindow::buildLanguageMenu() {
    QMap<QChar, QList<Language>> byLetter;
    for (Language lang : allLanguages())
        byLetter[languageName(lang).at(0).toUpper()].append(lang);

    QAction* plainTextAct = m_languageMenu->addAction("Plain Text");
    plainTextAct->setCheckable(true);
    plainTextAct->setActionGroup(m_langGroup);
    connect(plainTextAct, &QAction::triggered, this,
        [this]() {
            onLanguageOverride(Language::None);
        });
    m_languageMenu->addSeparator();

    for (auto it = byLetter.constBegin(); it != byLetter.constEnd(); ++it) {
        QMenu* letterMenu = m_languageMenu->addMenu(QString(it.key()));
        for (Language lang : it.value()) {
            if (lang == Language::None) continue;

            QAction* act = letterMenu->addAction(languageName(lang));
            act->setCheckable(true);
            act->setActionGroup(m_langGroup);
            connect(act, &QAction::triggered, this,
                    [this, lang]() { onLanguageOverride(lang); });
        }
    }
}

void MainWindow::buildFontMenu() {
    const QString saved = AppDatabase::instance().editorFontFamily();
    const QString system =
        QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
    const QString active = saved.isEmpty() ? system : saved;

    for (const QString& family : QFontDatabase::families()) {
        if (!QFontDatabase::isFixedPitch(family)) continue;
        QAction* act = m_fontMenu->addAction(family);
        act->setCheckable(true);
        act->setActionGroup(m_fontGroup);
        if (family == active) act->setChecked(true);
        connect(act, &QAction::triggered, this,
                [this, family]() { onFontSelected(family); });
    }
}

void MainWindow::applyFontToAllEditors(const QString& family, int pointSize) {
    QFont font(family);
    font.setPointSize(pointSize);
    font.setFixedPitch(true);
    const int tabSize = AppDatabase::instance().tabSize();
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        auto* ed = m_tabWidget->editorAt(i);
        if (ed) {
            ed->setFont(font);
            ed->setTabSize(tabSize);
        }
    }
}

QString MainWindow::autoDetectFileUnicode(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "?";
    }

    QTextStream in(&file);
    in.setAutoDetectUnicode(true);

    QString sample = in.read(4);

    QStringConverter::Encoding enc = in.encoding();
    QString encodingName = QStringConverter::nameForEncoding(enc);

    return encodingName.isEmpty() ? "?" : encodingName;
}