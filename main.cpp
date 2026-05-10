#include "clipboardmanager.h"
#include "historywindow.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QLocalServer>
#include <QLocalSocket>

static const char *SERVER_NAME = "QTCopyHistory_Instance";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("ClipboardHistory");

    bool showOnly = false;
    for (int i = 1; i < argc; ++i) {
        if (QString::fromLocal8Bit(argv[i]) == "--show") {
            showOnly = true;
            break;
        }
    }

    auto tryShowExisting = []() -> bool {
        QLocalSocket socket;
        socket.connectToServer(SERVER_NAME);
        if (socket.waitForConnected(500)) {
            socket.write("show");
            socket.waitForBytesWritten(500);
            socket.disconnectFromServer();
            return true;
        }
        return false;
    };

    if (showOnly) {
        if (tryShowExisting())
            return 0;
        return 0;
    }

    if (tryShowExisting())
        return 0;

    ClipboardManager manager;
    manager.start();

    HistoryWindow historyWindow(&manager, nullptr);

    QLocalServer localServer;
    localServer.removeServer(SERVER_NAME);
    localServer.listen(SERVER_NAME);

    QObject::connect(&localServer, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = localServer.nextPendingConnection();
        if (client) {
            client->waitForReadyRead(300);
            if (client->readAll().contains("show"))
                historyWindow.showAtCursor();
            client->deleteLater();
        }
    });

    QSystemTrayIcon tray(QIcon::fromTheme("edit-copy", QIcon(":/icons/copy.png")));
    QMenu trayMenu;

    QAction showAction("Show History");
    QAction quitAction("Quit");

    QObject::connect(&showAction, &QAction::triggered, [&]() {
        historyWindow.showAtCursor();
    });

    QObject::connect(&quitAction, &QAction::triggered, &app, &QApplication::quit);

    trayMenu.addAction(&showAction);
    trayMenu.addSeparator();
    trayMenu.addAction(&quitAction);
    tray.setContextMenu(&trayMenu);

    QObject::connect(&tray, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger ||
            reason == QSystemTrayIcon::DoubleClick) {
            historyWindow.showAtCursor();
        }
    });

    tray.show();
    return app.exec();
}
