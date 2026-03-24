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

private
    slots :
    
    void onItemDoubleClicked(const QModelIndex& index);

    void onChooseFolder();

private:
    void setupUi();

    QVBoxLayout* m_layout = nullptr;
    QLabel* m_label = nullptr;
    QToolButton* m_folderBtn = nullptr;
    QToolButton* m_refreshBtn = nullptr;
    QTreeView* m_treeView = nullptr;
    QFileSystemModel* m_model = nullptr;
};

#endif  // OPENNOTE_LINUX_FILETREEWIDGET_H