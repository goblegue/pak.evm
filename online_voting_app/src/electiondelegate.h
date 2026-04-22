#ifndef ELECTIONDELEGATE_H
#define ELECTIONDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>

class ElectionDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ElectionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // 1. Define the size of the Card
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(option.rect.width(), 100); // 100 pixels tall
    }

    // 2. Draw the Card
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        // Fetch the data from the Model
        QString title = index.data(Qt::UserRole + 2).toString(); // TitleRole
        QString details = index.data(Qt::UserRole + 3).toString(); // DetailsRole
        int statusEnum = index.data(Qt::UserRole + 4).toInt(); // StatusRole

        // Set up the Card Box
        QRect rect = option.rect.adjusted(10, 5, -10, -5); // Add margins

        // Hover and Selection Effects
        if (option.state & QStyle::State_Selected || option.state & QStyle::State_MouseOver) {
            painter->setBrush(QColor("#f0f4fa")); // Highlight color
            painter->setPen(QPen(QColor("#3b5998"), 2)); // Blue Border
        } else {
            painter->setBrush(QColor("#ffffff"));
            painter->setPen(QPen(QColor("#d1d9e6"), 1)); // Grey Border
        }
        painter->drawRoundedRect(rect, 10, 10); // Draw the card with 10px rounded corners

        // Draw Title
        painter->setPen(QColor("#1a2845"));
        QFont titleFont = painter->font();
        titleFont.setBold(true);
        titleFont.setPointSize(14);
        painter->setFont(titleFont);
        painter->drawText(rect.adjusted(15, 15, -15, -15), Qt::AlignLeft | Qt::AlignTop, title);

        // Draw Details
        painter->setPen(QColor("#555555"));
        QFont detailsFont = painter->font();
        detailsFont.setBold(false);
        detailsFont.setPointSize(10);
        painter->setFont(detailsFont);
        painter->drawText(rect.adjusted(15, 45, -15, -15), Qt::AlignLeft | Qt::AlignTop, details);

        // Draw Status Badge Logic
        QString statusText;
        QColor badgeColor;
        switch(statusEnum) {
        case 0: statusText = "DRAFT"; badgeColor = QColor("#95a5a6"); break;
        case 1: statusText = "PUBLISHED"; badgeColor = QColor("#3498db"); break;
        case 2: statusText = "VOTING OPEN"; badgeColor = QColor("#2ecc71"); break;
        default: statusText = "CLOSED"; badgeColor = QColor("#e74c3c"); break;
        }

        // Draw the Badge Box
        QRect badgeRect(rect.right() - 110, rect.top() + 15, 95, 25);
        painter->setBrush(badgeColor);
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(badgeRect, 12, 12);

        // Draw Badge Text
        painter->setPen(Qt::white);
        QFont badgeFont = painter->font();
        badgeFont.setBold(true);
        badgeFont.setPointSize(9);
        painter->setFont(badgeFont);
        painter->drawText(badgeRect, Qt::AlignCenter, statusText);

        painter->restore();
    }
};

#endif // ELECTIONDELEGATE_H
