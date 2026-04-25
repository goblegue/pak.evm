#ifndef DELEGATES_H
#define DELEGATES_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include "models/Models.h"

// ==========================================
// ELECTION BOX DELEGATE
// ==========================================
class ElectionDelegate : public QStyledItemDelegate {
public:
    explicit ElectionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // 1. Give the box a nice height
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(option.rect.width(), 55);
    }

    // 2. Draw the rounded box
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;

        // Background always white
        painter->setBrush(Qt::white);

        // Border and Text color (Blue for Elections)
        QColor mainColor = QColor("#2980B9");

        // Highlight logic if user clicks the election
        if (option.state & QStyle::State_Selected) {
            painter->setBrush(QColor("#EAF2F8")); // Light blue background when selected
            mainColor = QColor("#1A5276"); // Darker blue text/border
        }

        painter->setPen(QPen(mainColor, 2)); // 2px thick border

        // Draw the rounded rectangle (8px corner radius)
        painter->drawRoundedRect(rect, 8, 8);

        // Draw the text
        painter->setPen(mainColor);
        QString text = index.data(Qt::DisplayRole).toString();

        // Adjust the text rect so it has margins inside the box
        QRect textRect = rect.adjusted(15, 0, -15, 0);

        // Draw text vertically centered, aligned left, bold font
        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

        painter->restore();
    }
};

// ==========================================
// CANDIDATE STATUS BOX DELEGATE
// ==========================================
class CandidateDelegate : public QStyledItemDelegate {
public:
    explicit CandidateDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(option.rect.width(), 65); // Slightly taller for multi-line text
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        painter->setBrush(Qt::white);

        // Fetch the status from our custom Role!
        int statusInt = index.data(CandidateStatusRole).toInt();
        ApprovalStatus status = static_cast<ApprovalStatus>(statusInt);

        QColor statusColor;
        switch(status) {
        case ApprovalStatus::Pending:
            statusColor = QColor("#F39C12"); // Golden Yellow (easier to read than bright yellow)
            break;
        case ApprovalStatus::Approved:
            statusColor = QColor("#27AE60"); // Emerald Green
            break;
        case ApprovalStatus::Rejected:
            statusColor = QColor("#C0392B"); // Deep Red
            break;
        default:
            statusColor = QColor("#7F8C8D"); // Grey fallback
            break;
        }

        if (option.state & QStyle::State_Selected) {
            painter->setBrush(QColor("#F8F9F9")); // Very light grey on hover/select
        }

        // Draw border matching the status color
        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        // Draw Text
        painter->setPen(statusColor); // Text color matches border
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect = rect.adjusted(15, 5, -15, -5);

        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);

        painter->restore();
    }
};

#endif // DELEGATES_H
