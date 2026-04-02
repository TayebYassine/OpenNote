#include "FileTreeWidget.h"

#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QStatusBar>
#include <QInputDialog>
#include <QDesktopServices>
#include <qmenu.h>
#include <QProcess>

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
    m_folderBtn->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::FolderVisiting));
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
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_treeView->hideColumn(1); // size
    m_treeView->hideColumn(2); // type
    m_treeView->hideColumn(3); // date modified
    m_treeView->header()->hide();

    m_treeView->setStyleSheet("QTreeView { border: none; font-size: 16px; }");

    m_layout->addWidget(m_treeView);

    connect(m_treeView, &QTreeView::doubleClicked, this,
            &FileTreeWidget::onItemDoubleClicked);
    connect(m_folderBtn, &QToolButton::clicked, this,
            &FileTreeWidget::onChooseFolder);
    connect(m_refreshBtn, &QToolButton::clicked, this,
            &FileTreeWidget::reloadRootPath);
    connect(m_treeView, &QTreeView::customContextMenuRequested, this,
            &FileTreeWidget::onContextMenuRequested);
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

void FileTreeWidget::onContextMenuRequested(const QPoint& pos) {
    m_contextMenuIndex = m_treeView->indexAt(pos);
    if (!m_contextMenuIndex.isValid()) return;

    QMenu menu(this);

    if (isContextMenuItemFile()) {
        // File menu
        menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::DocumentOpen),
                      "Open", this, &FileTreeWidget::onOpenFile);
        menu.addAction(QIcon::fromTheme("document-open-remote"),
                      "Open With...", this,
                      &FileTreeWidget::onOpenWithDefault);
        menu.addSeparator();
        menu.addAction(QIcon::fromTheme("edit-rename"), // TODO ts
                      "Rename", this, &FileTreeWidget::onRenameItem);
        menu.addAction(QApplication::style()->standardIcon(QStyle::SP_DirLinkIcon), // TODO ts
                      "Move to Folder...", this, &FileTreeWidget::onMoveItem);
        menu.addAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon), // TODO Ts
                      "Delete", this, &FileTreeWidget::onDeleteItem);
        menu.addSeparator();
        menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::DialogInformation),
                      "Properties", this, &FileTreeWidget::onFileProperties);
    } else {
        // Folder menu
        bool isExpanded = m_treeView->isExpanded(m_contextMenuIndex);

        QAction* expandAct = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoUp),
                          "Expand", this, &FileTreeWidget::onExpandFolder);
        expandAct->setEnabled(!isExpanded);
        QAction* collapseAct = menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::GoDown),
                          "Collapse", this, &FileTreeWidget::onCollapseFolder);
        collapseAct->setEnabled(isExpanded);
        menu.addSeparator();
        menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::FolderOpen),
                      "Open in File Manager", this,
                      &FileTreeWidget::onOpenInFileManager);
        menu.addSeparator();
        menu.addAction(QIcon::fromTheme("edit-rename"),
                      "Rename", this, &FileTreeWidget::onRenameItem);
        menu.addAction(QApplication::style()->standardIcon(QStyle::SP_DirLinkIcon),
                      "Move to Folder...", this, &FileTreeWidget::onMoveItem);
        menu.addAction(QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
                      "Delete", this, &FileTreeWidget::onDeleteItem);
        menu.addSeparator();
        menu.addAction(QIcon::fromTheme(QIcon::ThemeIcon::DialogInformation),
                      "Properties", this, &FileTreeWidget::onFileProperties);
    }

    menu.exec(m_treeView->viewport()->mapToGlobal(pos));
}

bool FileTreeWidget::isContextMenuItemFile() const {
    if (!m_contextMenuIndex.isValid()) return false;
    return !m_model->isDir(m_contextMenuIndex);
}

bool FileTreeWidget::isContextMenuItemFolder() const {
    if (!m_contextMenuIndex.isValid()) return false;
    return m_model->isDir(m_contextMenuIndex);
}

QString FileTreeWidget::contextMenuItemPath() const {
    if (!m_contextMenuIndex.isValid()) return QString();
    return m_model->filePath(m_contextMenuIndex);
}

void FileTreeWidget::onOpenFile() {
    emit fileSelected(contextMenuItemPath());
}

void FileTreeWidget::onOpenWithDefault() {
    QString path = contextMenuItemPath();
    if (path.isEmpty()) return;
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void FileTreeWidget::onExpandFolder() {
    m_treeView->expand(m_contextMenuIndex);
}

void FileTreeWidget::onCollapseFolder() {
    m_treeView->collapse(m_contextMenuIndex);
}

void FileTreeWidget::onOpenInFileManager() {
    QString path = contextMenuItemPath();
    if (path.isEmpty()) return;

    // Use xdg-open to open the default file manager on Linux
    QProcess::startDetached("xdg-open", QStringList() << path);
}

void FileTreeWidget::onRenameItem() {
    QString oldPath = contextMenuItemPath();
    if (oldPath.isEmpty()) return;

    QFileInfo info(oldPath);
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename",
        QString("Rename %1:").arg(info.isDir() ? "folder" : "file"),
        QLineEdit::Normal, info.fileName(), &ok);

    if (!ok || newName.isEmpty() || newName == info.fileName()) return;

    QString newPath = info.dir().absoluteFilePath(newName);

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Rename Failed",
            "A file or folder with that name already exists.");
        return;
    }

    bool wasOpen = hasTabsWithPath(oldPath);

    if (info.isDir()) {
        QDir dir(oldPath);
        if (!dir.rename(oldPath, newPath)) {
            QMessageBox::warning(this, "Rename Failed",
                "Could not rename the folder.");
            return;
        }
    } else {
        if (!QFile::rename(oldPath, newPath)) {
            QMessageBox::warning(this, "Rename Failed",
                "Could not rename the file.");
            return;
        }
    }

    if (wasOpen) {
        updateTabPaths(oldPath, newPath);
    }

    emit statusMessage(QString("Renamed to %1").arg(newName), 4000);
    reloadRootPath();
}

void FileTreeWidget::onMoveItem() {
    QString oldPath = contextMenuItemPath();
    if (oldPath.isEmpty()) return;

    QFileInfo info(oldPath);

    QString destDir = QFileDialog::getExistingDirectory(this,
        QString("Move %1 to...").arg(info.fileName()),
        info.dir().absolutePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (destDir.isEmpty()) return;

    if (info.isDir()) {
        if (destDir.startsWith(oldPath + "/") || destDir == oldPath) {
            QMessageBox::warning(this, "Move Failed",
                "Cannot move a folder into itself or its subfolder.");
            return;
        }
    }

    QString newPath = QDir(destDir).absoluteFilePath(info.fileName());

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, "Move Failed",
            "A file or folder with that name already exists in the destination.");
        return;
    }

    bool wasOpen = hasTabsWithPath(oldPath);

    if (info.isDir()) {
        QProcess process;
        process.start("mv", QStringList() << "-T" << oldPath << newPath);
        if (!process.waitForFinished() || process.exitCode() != 0) {
            QMessageBox::warning(this, "Move Failed",
                "Could not move the folder.");
            return;
        }
    } else {
        if (!QFile::rename(oldPath, newPath)) {
            QMessageBox::warning(this, "Move Failed",
                "Could not move the file.");
            return;
        }
    }

    if (wasOpen) {
        updateTabPaths(oldPath, newPath);
    }

    emit statusMessage(QString("Moved to %1").arg(destDir), 4000);
    reloadRootPath();
}

void FileTreeWidget::onDeleteItem() {
    QString path = contextMenuItemPath();
    if (path.isEmpty()) return;

    QFileInfo info(path);
    QString itemType = info.isDir() ? "folder" : "file";
    QString warning = info.isDir() ? "" : "<br><br>Open tabs will be closed without saving.";

    auto reply = QMessageBox::question(this, "Confirm Delete",
        QString("Are you sure you want to delete the %1 '<b>%2</b>'?<br>"
                "This action cannot be undone.%3")
        .arg(itemType, info.fileName(), warning),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if (reply != QMessageBox::Yes) return;

    closeTabsWithPath(path);

    bool success;
    if (info.isDir()) {
        QDir dir(path);
        success = dir.removeRecursively();
    } else {
        success = QFile::remove(path);
    }

    if (!success) {
        QMessageBox::warning(this, "Delete Failed",
            QString("Could not delete the %1.").arg(itemType));
        return;
    }

    emit statusMessage(QString("%1 deleted").arg(info.fileName()), 4000);
    reloadRootPath();
}

void FileTreeWidget::onFileProperties() {
    QString path = contextMenuItemPath();
    if (path.isEmpty()) return;

    QFileInfo info(path);

    QStringList props;
    props << QString("<b>Name:</b> %1").arg(info.fileName());
    props << QString("<b>Path:</b> %1").arg(info.absolutePath());
    props << QString("<b>Type:</b> %1").arg(info.isDir() ? "Folder" : "File");
    props << QString("<b>Size:</b> %1").arg(info.isDir() ?
        QString("%1 items").arg(QDir(path).entryList(QDir::AllEntries | QDir::NoDotAndDotDot).count()) :
        QLocale().formattedDataSize(info.size()));
    props << QString("<b>Created:</b> %1").arg(info.birthTime().toString());
    props << QString("<b>Modified:</b> %1").arg(info.lastModified().toString());

    QMessageBox::information(this, "Properties", props.join("<br>"));
}

bool FileTreeWidget::hasTabsWithPath(const QString& path) const {
    MainWindow* mainWindow = qobject_cast<MainWindow*>(window());
    if (!mainWindow) return false;

    printf("%s", mainWindow->hasTabOpenWithPath(path) ? "true" : "false");

    return mainWindow->hasTabOpenWithPath(path);
}

void FileTreeWidget::updateTabPaths(const QString& oldPath, const QString& newPath) {
    emit filePathChanged(oldPath, newPath);
}

void FileTreeWidget::closeTabsWithPath(const QString& path) {
    emit filePathDeleted(path);
}
