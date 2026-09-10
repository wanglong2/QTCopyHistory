#include "historywindow.h"
#include <QApplication>
#include <QClipboard>
#include <QScreen>
#include <QTest>
#include <QSignalSpy>
class InteractionTest : public QObject {
    Q_OBJECT
private slots:
    void selectionAndPaste() {
        ClipboardManager manager;
        manager.start();
        QApplication::clipboard()->setText("first");
        QApplication::clipboard()->setText("second");
        HistoryWindow window(&manager);
        QSignalSpy paste(&window, &HistoryWindow::pasteRequested);
        window.showAtCursor();
        auto *list = window.findChild<QListWidget *>();
        QCOMPARE(list->currentRow(), 0);
        QTest::keyClick(list, Qt::Key_Down);
        QCOMPARE(list->currentRow(), 1);
        QTest::keyClick(list, Qt::Key_Down);
        QCOMPARE(list->currentRow(), 0);
        QTest::keyClick(list, Qt::Key_Up);
        QCOMPARE(list->currentRow(), 1);
        QTest::keyClick(list, Qt::Key_Return);
        QCOMPARE(QApplication::clipboard()->text(), QString("first"));
        QCOMPARE(manager.history().first().text, QString("first"));
        QCOMPARE(manager.history().size(), 2);
        QCOMPARE(paste.count(), 1);
        QVERIFY(!window.isVisible());
        window.showAtCursor();
        QVERIFY(QApplication::primaryScreen()->availableGeometry().contains(window.geometry()));
        QTest::keyClick(list, Qt::Key_Escape);
        QCOMPARE(paste.count(), 1);
        manager.clearHistory();
        window.showAtCursor();
        QTest::keyClick(list, Qt::Key_Down);
        QTest::keyClick(list, Qt::Key_Return);
        QCOMPARE(paste.count(), 1);
    }
};
QTEST_MAIN(InteractionTest)
#include "interaction_test.moc"
