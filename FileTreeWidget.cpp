#include "FileTreeWidget.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QStatusBar>

#include "AppDatabase.h"
#include "MainWindow.h"

FileTreeWidget::FileTreeWidget(QWidget* parent) : QWidget(parent) {
    setupUi();
    // Restore the last used directory (falls back to home)
    const QString saved = AppDatabase::instance().recentDirectory();
    setRootPath(saved.isEmpty() ? QDir::homePath() : saved);
}

void FileTreeWidget::setupUi() {
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    QWidget* header = new QWidget(this);
    header->setObjectName("treeHeader");
    header->setStyleSheet("#treeHeader { border-bottom: 1px solid #3c3c3c; }");

    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 6, 8, 6);

    m_label = new QLabel("EXPLORER", header);
    m_label->setStyleSheet(
        "font-size: 11px; font-weight: bold; letter-spacing: 1px;");

    m_folderBtn = new QToolButton(header);
    m_folderBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen));
    m_folderBtn->setToolTip("Open folder…");
    m_folderBtn->setStyleSheet("QToolButton { border: none; font-size: 15px; }");

    m_refreshBtn = new QToolButton(header);
    m_refreshBtn->setIcon(
        QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
    m_refreshBtn->setToolTip("Reload All from disk");
    m_refreshBtn->setStyleSheet("QToolButton { border: none; font-size: 15px; }");

    headerLayout->addWidget(m_label);
    headerLayout->addStretch();
    headerLayout->addWidget(m_folderBtn);
    headerLayout->addWidget(m_refreshBtn);
    m_layout->addWidget(header);

    m_model = new QFileSystemModel(this);
    m_model->setRootPath(QDir::homePath());
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::AllDirs);

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);
    m_treeView->setAnimated(true);
    m_treeView->setIndentation(16);
    m_treeView->setSortingEnabled(true);
    m_treeView->sortByColumn(0, Qt::AscendingOrder);
    m_treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_treeView->hideColumn(1); // size
    m_treeView->hideColumn(2); // type
    m_treeView->hideColumn(3); // date modified
    m_treeView->header()->hide();

    m_treeView->setStyleSheet("QTreeView { border: none; font-size: 13px; }");

    m_layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, this,
            &FileTreeWidget::onItemDoubleClicked);
    connect(m_folderBtn, &QToolButton::clicked, this,
            &FileTreeWidget::onChooseFolder);
    connect(m_refreshBtn, &QToolButton::clicked, this,
            &FileTreeWidget::reloadRootPath);
}

void FileTreeWidget::setRootPath(const QString& path) {
    QModelIndex root = m_model->setRootPath(path);
    m_treeView->setRootIndex(root);
    m_label->setToolTip(path);
    AppDatabase::instance().setRecentDirectory(path);
    AppDatabase::instance().save();
}

void FileTreeWidget::reloadRootPath() {
    const QString path = m_model->rootPath();

    m_model->setRootPath("");
    QModelIndex root = m_model->setRootPath(path);
    m_treeView->setRootIndex(root);
    m_label->setToolTip(path);

    emit statusMessage("Reloaded all files from disk.", 6000);
}

void FileTreeWidget::onItemDoubleClicked(const QModelIndex& index) {
    if (m_model->isDir(index)) return;
    emit fileSelected(m_model->filePath(index));
}

void FileTreeWidget::onChooseFolder() {
    const QString dir = QFileDialog::getExistingDirectory(
        this, "Open Folder", m_model->rootPath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) setRootPath(dir);
}