#include "clipboardmanager.h"
#include <QApplication>
#include <QMimeData>

ClipboardManager::ClipboardManager(QObject *parent)
    : QObject(parent)
    , m_clipboard(QApplication::clipboard())
{
}

void ClipboardManager::start()
{
    connect(m_clipboard, &QClipboard::dataChanged,
            this, &ClipboardManager::onClipboardChanged);
}

void ClipboardManager::onClipboardChanged()
{
    const QMimeData *mime = m_clipboard->mimeData();
    if (!mime) return;

    ClipboardItem item;
    item.timestamp = QDateTime::currentDateTime();

    if (mime->hasImage()) {
        item.type = ClipType::Image;
        item.image = qvariant_cast<QImage>(mime->imageData());
        if (item.image.isNull()) return;
    } else if (mime->hasText()) {
        QString text = mime->text();
        if (text.isEmpty()) return;
        item.type = ClipType::Text;
        item.text = text;
    } else {
        return;
    }

    addItem(item);
}

void ClipboardManager::addItem(const ClipboardItem &item)
{
    for (int i = 0; i < m_history.size(); ++i) {
        const ClipboardItem &existing = m_history.at(i);
        if (existing.type != item.type) continue;

        if (item.type == ClipType::Text) {
            if (existing.text == item.text) {
                moveToTop(i);
                return;
            }
        } else if (item.type == ClipType::Image) {
            if (existing.image.size() == item.image.size()
                && existing.image.bytesPerLine() == item.image.bytesPerLine()
                && existing.image.format() == item.image.format()) {
                if (existing.image == item.image) {
                    moveToTop(i);
                    return;
                }
            }
        }
    }

    m_history.prepend(item);
    while (m_history.size() > MAX_HISTORY)
        m_history.removeLast();

    emit historyChanged();
}

void ClipboardManager::moveToTop(int index)
{
    if (index <= 0) return;
    ClipboardItem item = m_history.takeAt(index);
    m_history.prepend(item);
    emit historyChanged();
}

void ClipboardManager::copyToClipboard(int index)
{
    if (index < 0 || index >= m_history.size()) return;

    const ClipboardItem &item = m_history.at(index);
    auto guard = m_clipboard->mimeData();
    Q_UNUSED(guard);

    if (item.type == ClipType::Text) {
        m_clipboard->setText(item.text);
    } else if (item.type == ClipType::Image) {
        m_clipboard->setImage(item.image);
    }

    if (index != 0)
        moveToTop(index);
}

void ClipboardManager::removeFromHistory(int index)
{
    if (index < 0 || index >= m_history.size()) return;
    m_history.removeAt(index);
    emit historyChanged();
}

void ClipboardManager::clearHistory()
{
    m_history.clear();
    emit historyChanged();
}
