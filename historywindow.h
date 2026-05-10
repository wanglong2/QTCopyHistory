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

    void showAtCursor();

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
