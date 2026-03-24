#include <QApplication>

#include "AppDatabase.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("OpenNote");
    app.setOrganizationName("OpenNote Developers");
    app.setApplicationVersion(OPENNOTE_VERSION);

    AppDatabase::instance().load();

    MainWindow window;
    window.show();

    return app.exec();
}