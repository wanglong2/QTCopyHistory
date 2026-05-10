#include "historywindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QCursor>
#include <QScreen>
#include <QApplication>
#include <QScrollBar>
#include <QPainterPath>
#include <QRegion>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QFrame>

HistoryWindow::HistoryWindow(ClipboardManager *manager, QWidget *parent)
    : QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
    , m_manager(manager)
{
    setWindowTitle("Clipboard History");
    setObjectName("HistoryWindow");
    setMinimumSize(400, 300);
    resize(480, 420);
    setAttribute(Qt::WA_TranslucentBackground);

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto *card = new QFrame(this);
    card->setObjectName("Card");
    card->setStyleSheet(
        "QFrame#Card {"
        "  background: #ffffff;"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 12px;"
        "}"
    );

    QHBoxLayout *cardLayout = new QHBoxLayout(card);
    cardLayout->setContentsMargins(10, 10, 10, 10);
    cardLayout->setSpacing(10);

    m_list = new QListWidget(card);
    m_list->setObjectName("HistoryList");
    m_list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_list->setFocusPolicy(Qt::StrongFocus);
    m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_list->verticalScrollBar()->setSingleStep(20);
    m_list->setStyleSheet(
        "QListWidget#HistoryList {"
        "  background: #f9f9f9;"
        "  border: 1px solid #e8e8e8;"
        "  border-radius: 8px;"
        "  font-size: 13px;"
        "  padding: 4px;"
        "  outline: none;"
        "}"
        "QListWidget#HistoryList::item {"
        "  padding: 8px 12px;"
        "  border-bottom: 1px solid #f0f0f0;"
        "  color: #1a1a1a;"
        "}"
        "QListWidget#HistoryList::item:last {"
        "  border-bottom: none;"
        "}"
        "QListWidget#HistoryList::item:selected {"
        "  background-color: #0078d4;"
        "  color: white;"
        "  border-radius: 4px;"
        "}"
        "QListWidget#HistoryList::item:hover:!selected {"
        "  background-color: #e8f3fa;"
        "}"
        "QScrollBar:vertical {"
        "  background: transparent;"
        "  width: 8px;"
        "  margin: 2px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: #c0c0c0;"
        "  border-radius: 4px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "  background: #a0a0a0;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "  background: none;"
        "}"
    );

    m_preview = new QLabel(card);
    m_preview->setObjectName("Preview");
    m_preview->setFixedWidth(160);
    m_preview->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_preview->setStyleSheet(
        "QLabel#Preview {"
        "  background: #f5f5f5;"
        "  border: 1px solid #e8e8e8;"
        "  border-radius: 8px;"
        "  padding: 8px;"
        "  color: #555;"
        "  font-size: 12px;"
        "}"
    );
    m_preview->setWordWrap(true);

    cardLayout->addWidget(m_list, 1);
    cardLayout->addWidget(m_preview);

    outerLayout->addWidget(card);

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(0, 0, 0, 60));
    card->setGraphicsEffect(shadow);

    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        if (row < 0 || row >= m_manager->history().size()) {
            m_preview->clear();
            return;
        }

        const ClipboardItem &item = m_manager->history().at(row);
        if (item.type == ClipType::Image) {
            QPixmap pix = QPixmap::fromImage(item.image).scaled(
                144, 144, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_preview->setPixmap(pix);
        } else {
            m_preview->setText(item.text.left(300));
        }
    });

    connect(m_manager, &ClipboardManager::historyChanged,
            this, &HistoryWindow::refreshList);

    connect(m_list, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        int row = m_list->row(item);
        if (row >= 0) onItemClicked(row);
    });
}

void HistoryWindow::showAtCursor()
{
    refreshList();
    QPoint cursorPos = QCursor::pos();
    QScreen *screen = QApplication::screenAt(cursorPos);
    if (!screen) screen = QApplication::primaryScreen();

    QRect screenRect = screen->availableGeometry();
    int x = cursorPos.x() - width() / 2;
    int y = cursorPos.y() - height() - 10;

    if (x < screenRect.left()) x = screenRect.left() + 4;
    if (x + width() > screenRect.right()) x = screenRect.right() - width() - 4;
    if (y < screenRect.top()) y = cursorPos.y() + 20;

    move(x, y);
    updateMask();
    show();
    activateWindow();
    m_list->setFocus();

    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void HistoryWindow::refreshList()
{
    int currentRow = m_list->currentRow();
    m_list->blockSignals(true);
    m_list->clear();

    const QList<ClipboardItem> &history = m_manager->history();
    for (int i = 0; i < history.size(); ++i) {
        const ClipboardItem &item = history.at(i);
        QString label;
        if (item.type == ClipType::Image) {
            label = QString("  [Image]  %1×%2    %3")
                        .arg(item.image.width())
                        .arg(item.image.height())
                        .arg(item.timestamp.toString("HH:mm:ss"));
        } else {
            QString firstLine = item.text.section('\n', 0, 0).simplified().left(50);
            if (firstLine.isEmpty()) firstLine = item.text.left(50);
            label = QString("  %1    %2").arg(firstLine, item.timestamp.toString("HH:mm:ss"));
        }
        m_list->addItem(label);
    }

    m_list->blockSignals(false);

    if (currentRow >= 0 && currentRow < m_list->count())
        m_list->setCurrentRow(currentRow);
    else if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

void HistoryWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        int row = m_list->currentRow();
        if (row >= 0) {
            onItemClicked(row);
        }
        return;
    }

    if (event->key() == Qt::Key_Delete) {
        int row = m_list->currentRow();
        if (row >= 0) {
            m_manager->removeFromHistory(row);
        }
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        hide();
        return;
    }

    if (event->key() == Qt::Key_Down && m_list->currentRow() == m_list->count() - 1) {
        m_list->setCurrentRow(0);
        return;
    }

    if (event->key() == Qt::Key_Up && m_list->currentRow() == 0) {
        m_list->setCurrentRow(m_list->count() - 1);
        return;
    }

    QWidget::keyPressEvent(event);
}

void HistoryWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange && !isActiveWindow()) {
        hide();
    }
    QWidget::changeEvent(event);
}

void HistoryWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateMask();
}

void HistoryWindow::updateMask()
{
    QPainterPath path;
    path.addRoundedRect(rect(), 12, 12);
    setMask(path.toFillPolygon().toPolygon());
}

void HistoryWindow::onItemClicked(int index)
{
    m_manager->copyToClipboard(index);
    hide();
}
