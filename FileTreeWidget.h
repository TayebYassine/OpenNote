#ifndef OPENNOTE_LINUX_FILETREEWIDGET_H
#define OPENNOTE_LINUX_FILETREEWIDGET_H
#pragma once

#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidget>

class FileTreeWidget : public QWidget {
    Q_OBJECT

public:
    explicit FileTreeWidget(QWidget* parent = nullptr);

    ~FileTreeWidget() override = default;

    void setRootPath(const QString& path);

    void reloadRootPath();

    signals :
    
    void fileSelected(const QString& filePath);

    void statusMessage(const QString& message, int timeoutMs);

    void filePathChanged(const QString& oldPath, const QString& newPath);

    void filePathDeleted(const QString& path);

private
    slots :
    
    void onItemDoubleClicked(const QModelIndex& index);

    void onChooseFolder();

    void onContextMenuRequested(const QPoint& pos);

    void onOpenFile();
    void onOpenWithDefault();
    void onRenameItem();
    void onMoveItem();
    void onDeleteItem();
    void onFileProperties();

    void onExpandFolder();
    void onCollapseFolder();
    void onOpenInFileManager();

private:
    void setupUi();

    bool isContextMenuItemFile() const;
    bool isContextMenuItemFolder() const;
    QString contextMenuItemPath() const;
    void updateTabPaths(const QString& oldPath, const QString& newPath);
    void closeTabsWithPath(const QString& path);
    bool hasTabsWithPath(const QString& path) const;

    QVBoxLayout* m_layout = nullptr;
    QLabel* m_label = nullptr;
    QToolButton* m_folderBtn = nullptr;
    QToolButton* m_refreshBtn = nullptr;
    QTreeView* m_treeView = nullptr;
    QFileSystemModel* m_model = nullptr;
    QModelIndex m_contextMenuIndex;
};

#endif  // OPENNOTE_LINUX_FILETREEWIDGET_H