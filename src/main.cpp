#include <QApplication>
#include <QIcon>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ui/MainWindow.hpp"

int main(int argc, char *argv[]) {
#ifdef _WIN32
    HANDLE hMutex = CreateMutexA(NULL, FALSE, "qworldclock_mutex");
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QWorldClock"));
    app.setOrganizationName(QStringLiteral("QWorldClock"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/qworldclock.png")));

    qworldclock::MainWindow mainWindow;
    mainWindow.show();

    int result = app.exec();

#ifdef _WIN32
    if (hMutex != NULL) {
        CloseHandle(hMutex);
    }
#endif

    return result;
}
