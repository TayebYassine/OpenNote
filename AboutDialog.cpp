#include "AboutDialog.h"

#include <QApplication>
#include <QDialogButtonBox>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent, Qt::Dialog | Qt::WindowCloseButtonHint) {
    setWindowTitle("About OpenNote");
    setFixedSize(400, 340);

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0, 0, 0, 0);

    auto* tabs = new QTabWidget(this);

    auto* aboutPage = new QWidget;
    auto* aboutLayout = new QVBoxLayout(aboutPage);
    aboutLayout->setSpacing(8);
    aboutLayout->setContentsMargins(24, 20, 24, 20);
    aboutLayout->setAlignment(Qt::AlignHCenter);

    auto* nameLabel = new QLabel("OpenNote", aboutPage);
    nameLabel->setAlignment(Qt::AlignHCenter);
    nameLabel->setStyleSheet("font-size: 22px; font-weight: bold;");
    aboutLayout->addWidget(nameLabel);

    auto* versionLabel = new QLabel(
        QString("Version %1").arg(QApplication::applicationVersion()), aboutPage);
    versionLabel->setAlignment(Qt::AlignHCenter);
    versionLabel->setStyleSheet("font-size: 13px; color: #aaa;");
    aboutLayout->addWidget(versionLabel);

    auto* tagLabel = new QLabel(
        "OpenNote is a lightweight, open-source text editor\n"
        "inspired by Notepad++, built for Linux.",
        aboutPage);
    tagLabel->setAlignment(Qt::AlignHCenter);
    tagLabel->setWordWrap(true);
    tagLabel->setStyleSheet("font-size: 12px; margin-top: 6px;");
    aboutLayout->addWidget(tagLabel);

    auto* qtLabel = new QLabel(
        QString("Built with Qt %1  |  C++ 17").arg(QT_VERSION_STR), aboutPage);
    qtLabel->setAlignment(Qt::AlignHCenter);
    qtLabel->setStyleSheet("font-size: 11px; color: #888; margin-top: 4px;");
    aboutLayout->addWidget(qtLabel);

    auto* copyrightLabel =
        new QLabel("Copyright © 2026 Tayeb Yassine", aboutPage);
    copyrightLabel->setAlignment(Qt::AlignHCenter);
    copyrightLabel->setStyleSheet("font-size: 11px; color: #888;");
    aboutLayout->addWidget(copyrightLabel);

    auto* licenseLabel = new QLabel(
        R"(<a href="https://github.com/TayebYassine/OpenNote/blob/master/LICENSE" style="color:#5bc0de;">)"
        "MIT License</a>",
        aboutPage);
    licenseLabel->setOpenExternalLinks(true);
    licenseLabel->setAlignment(Qt::AlignHCenter);
    licenseLabel->setStyleSheet("font-size: 11px;");
    aboutLayout->addWidget(licenseLabel);

    aboutLayout->addStretch();
    tabs->addTab(aboutPage, "About");

    auto* creditsPage = new QWidget;
    auto* creditsLayout = new QVBoxLayout(creditsPage);
    creditsLayout->setContentsMargins(20, 16, 20, 16);

    auto* creditsLabel = new QLabel(
        "<b>OpenNote</b> is written in C++ 17 and built on top of the following open-source technologies:<br><br>"
        "• <b>Qt 6</b>: Cross-platform application framework, <a href='https://www.qt.io/licensing'>qt.io/licensing</a>"
        "<br><br>"
        "Icons from the system icon theme, provided by Qt.<br><br>"
        "Developers:<br>"
        "• Tayeb Yassine: <a href='https://github.com/TayebYassine'>GitHub</a>",
        creditsPage);
    creditsLabel->setWordWrap(true);
    creditsLabel->setOpenExternalLinks(true);
    creditsLabel->setStyleSheet("font-size: 12px; line-height: 1.5;");
    creditsLayout->addWidget(creditsLabel);
    creditsLayout->addStretch();
    tabs->addTab(creditsPage, "Credits");

    root->addWidget(tabs);

    auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    btnBox->setContentsMargins(12, 8, 12, 12);
    root->addWidget(btnBox);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::accept);
}
