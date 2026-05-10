#ifndef CLIPBOARDMANAGER_H
#define CLIPBOARDMANAGER_H

#include <QObject>
#include <QClipboard>
#include <QImage>
#include <QList>
#include <QVariant>
#include <QDateTime>

enum class ClipType { Text, Image };

struct ClipboardItem {
    ClipType type;
    QString text;
    QImage image;
    QDateTime timestamp;
};

class ClipboardManager : public QObject {
    Q_OBJECT
public:
    explicit ClipboardManager(QObject *parent = nullptr);

    void start();
    const QList<ClipboardItem> &history() const { return m_history; }
    void copyToClipboard(int index);
    void removeFromHistory(int index);
    void clearHistory();

signals:
    void historyChanged();

private slots:
    void onClipboardChanged();

private:
    QClipboard *m_clipboard;
    QList<ClipboardItem> m_history;
    static const int MAX_HISTORY = 50;

    void addItem(const ClipboardItem &item);
    void moveToTop(int index);
};

#endif // CLIPBOARDMANAGER_H
