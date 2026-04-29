#include <QApplication>
#include <QChar>
#include <QEvent>
#include <QFileOpenEvent>
#include <QString>
#include <QStringList>

#include "AppDatabase.h"
#include "MainWindow.h"
#include "SingleInstanceController.h"

#include <unistd.h>

namespace {
QString sanitizeServerName(QString s) {
    s = s.toLower();
    QString out;
    out.reserve(s.size());
    for (QChar c : s) {
        if (c.isLetterOrNumber()) out.append(c);
        else out.append('_');
    }
    while (out.contains("__")) out.replace("__", "_");
    return out;
}
}  // namespace

class OpenNoteApplication final : public QApplication {
    Q_OBJECT

public:
    using QApplication::QApplication;

signals:
    void filesOpenRequested(const QStringList& paths);

protected:
    bool event(QEvent* e) override {
        if (e && e->type() == QEvent::FileOpen) {
            auto* foe = static_cast<QFileOpenEvent*>(e);
            const QString path = foe->file();
            if (!path.isEmpty()) emit filesOpenRequested({path});
            return true;
        }
        return QApplication::event(e);
    }
};

int main(int argc, char* argv[]) {
    OpenNoteApplication app(argc, argv);
    app.setApplicationName("OpenNote");
    app.setOrganizationName("OpenNote Developers");
    app.setApplicationVersion(OPENNOTE_VERSION);

    AppDatabase::instance().load();

    QStringList cliPaths;
    cliPaths.reserve(qMax(0, argc - 1));
    bool afterDoubleDash = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (!afterDoubleDash && arg == "--") {
            afterDoubleDash = true;
            continue;
        }
        if (!afterDoubleDash && arg.startsWith("-")) continue;
        cliPaths.append(arg);
    }

    const QString serverName = sanitizeServerName(
        QString("%1_%2_%3")
            .arg(QCoreApplication::organizationName(),
                 QCoreApplication::applicationName(),
                 QString::number(::getuid())));

    SingleInstanceController instance(serverName);
    if (instance.tryForwardToPrimary(cliPaths)) return 0;
    (void)instance.startPrimary();

    MainWindow window;
    QObject::connect(&instance, &SingleInstanceController::filesReceived,
                     &window, &MainWindow::openFiles);
    QObject::connect(&app, &OpenNoteApplication::filesOpenRequested,
                     &window, &MainWindow::openFiles);
    window.show();
    window.openFiles(cliPaths);

    return app.exec();
}

#include "main.moc"