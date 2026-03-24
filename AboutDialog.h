#ifndef OPENNOTE_LINUX_ABOUTDIALOG_H
#define OPENNOTE_LINUX_ABOUTDIALOG_H
#pragma once

#include <QDialog>

class AboutDialog : public QDialog {
    Q_OBJECT

public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

#endif  // OPENNOTE_LINUX_ABOUTDIALOG_H
