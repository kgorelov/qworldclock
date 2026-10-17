#include <QApplication>
#include <QIcon>

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QWorldClock"));
    app.setOrganizationName(QStringLiteral("QWorldClock"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/qworldclock.png")));

    qworldclock::MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
