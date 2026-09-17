#include <QApplication>

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QWorldClock"));
    app.setOrganizationName(QStringLiteral("QWorldClock"));

    qworldclock::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
