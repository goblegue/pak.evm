#ifndef DELEGATES_H
#define DELEGATES_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QPixmap>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QCursor>
#include "Models.h"

// ==========================================
// VOTING BALLOT CANDIDATE DELEGATE
// ==========================================
class VotingCandidateDelegate : public QStyledItemDelegate {
    Q_OBJECT
signals:
    void voteButtonClicked(const QModelIndex &index) const;

public:
    explicit VotingCandidateDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // FIX: Subtract 80 to account for margins, spacing, AND the scrollbar!
        int width = (option.widget->width() - 80) / 2;
        return QSize(width, 160); // 160px tall box
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

            // Updated coordinates for VOTE button (matches paint function below)
            QRect btnRect(option.rect.right() - 110, option.rect.bottom() - 45, 90, 32);

            if (btnRect.contains(mouseEvent->pos())) {
                emit voteButtonClicked(index);
                return true;
            }
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect.adjusted(10, 10, 0, 0);

        // 1. Draw Outer Box
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor("#7A1A1A"), 2)); // Deep Red Border
        painter->drawRoundedRect(rect, 8, 8);

        // Fetch Candidate Data
        QString username = index.data(CandidateNameRole).toString();
        QString cnic = "CNIC: " + index.data(CandidateCnicRole).toString();
        QString party = "Party: " + index.data(CandidatePartyRole).toString();
        QString symbol = index.data(CandidateSymbolNameRole).toString();
        QString b64Profile = index.data(CandidateProfileImageRole).toString();
        QString b64Symbol = index.data(CandidateSymbolImageRole).toString();

        // 2. Draw Profile Image (Left)
        QRect profileRect(rect.left() + 15, rect.top() + 30, 80, 80);
        QPixmap profilePixmap;
        if (!b64Profile.isEmpty() && profilePixmap.loadFromData(QByteArray::fromBase64(b64Profile.toUtf8()))) {
            QPixmap circularPixmap(profilePixmap.size());
            circularPixmap.fill(Qt::transparent);
            QPainter p(&circularPixmap);
            p.setRenderHint(QPainter::Antialiasing);
            QPainterPath path;
            path.addEllipse(0, 0, profilePixmap.width(), profilePixmap.height());
            p.setClipPath(path);
            p.drawPixmap(0,0,profilePixmap);
            painter->drawPixmap(profileRect, circularPixmap.scaled(profileRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            painter->setBrush(QColor("#ECF0F1")); painter->setPen(QPen(QColor("#BDC3C7"), 1));
            painter->drawEllipse(profileRect); // Grey placeholder
        }

        // 3. Draw Text: Username, CNIC, Party (Middle)
        int textX = rect.left() + 115;

        painter->setPen(QColor("#2C3E50"));
        QFont nameFont = option.font; nameFont.setBold(true); nameFont.setPointSize(14);
        painter->setFont(nameFont);
        painter->drawText(textX, rect.top() + 45, username); // Candidate Username

        painter->setPen(QColor("#34495E"));
        QFont subFont = option.font; subFont.setPointSize(11);
        painter->setFont(subFont);
        painter->drawText(textX, rect.top() + 75, cnic);  // CNIC: 42101...
        painter->drawText(textX, rect.top() + 100, party); // Party: Democratic...

        // 4. Draw Symbol Image & Name (Right side, above Vote button)
        QRect symbolImgRect(rect.right() - 85, rect.top() + 25, 50, 50);
        QPixmap symbolPixmap;
        if (!b64Symbol.isEmpty() && symbolPixmap.loadFromData(QByteArray::fromBase64(b64Symbol.toUtf8()))) {
            painter->drawPixmap(symbolImgRect, symbolPixmap.scaled(symbolImgRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            painter->setBrush(QColor("#ECF0F1")); painter->setPen(QPen(QColor("#BDC3C7"), 1));
            painter->drawRoundedRect(symbolImgRect, 4, 4);
        }

        // Symbol Name under the image
        painter->setPen(QColor("#7F8C8D"));
        QFont symbolFont = option.font; symbolFont.setPointSize(9);
        painter->setFont(symbolFont);
        painter->drawText(QRect(rect.right() - 110, rect.top() + 80, 90, 20), Qt::AlignCenter, symbol);

        // 5. Draw the "VOTE" Button (Bottom Right)
        QRect btnRect(rect.right() - 110, rect.bottom() - 45, 90, 32);

        QColor btnBgColor = Qt::white;
        QColor btnTextColor = QColor("#7A1A1A"); // Red text

        if (option.state & QStyle::State_MouseOver && btnRect.contains(option.widget->mapFromGlobal(QCursor::pos()))) {
            btnBgColor = QColor("#FFE5E5"); // Light Red Hover!
        }

        painter->setBrush(btnBgColor);
        painter->setPen(QPen(QColor("#7A1A1A"), 2));
        painter->drawRoundedRect(btnRect, 4, 4);

        painter->setPen(btnTextColor);
        QFont btnFont = option.font; btnFont.setBold(true); btnFont.setPointSize(12);
        painter->setFont(btnFont);
        painter->drawText(btnRect, Qt::AlignCenter, "VOTE");

        painter->restore();
    }
};
#endif // DELEGATES_H
