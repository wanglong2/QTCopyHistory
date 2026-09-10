#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include "clipboardmanager.h"

class HistoryWindow : public QWidget {
    Q_OBJECT
public:
    explicit HistoryWindow(ClipboardManager *manager, QWidget *parent = nullptr);

    Q_CLASSINFO("D-Bus Interface", "io.github.QTCopyHistory")
public slots:
    void showAtCursor();
signals:
    void pasteRequested();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onItemClicked(int index);
    void refreshList();

private:
    ClipboardManager *m_manager;
    QListWidget *m_list;
    QLabel *m_preview;

    void updateMask();
};

#endif // HISTORYWINDOW_H
