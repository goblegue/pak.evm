#ifndef DELEGATES_H
#define DELEGATES_H

#include <QImage>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include "models/Models.h"
#include "views/AdminCandidatePage.h"

// ==========================================
// ELECTION BOX DELEGATE
// ==========================================
class ElectionDelegate : public QStyledItemDelegate
{
public:
    explicit ElectionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // 1. Give the box a nice height
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), 55);
    }

    // 2. Draw the rounded box
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;

        // Background always white
        painter->setBrush(Qt::white);

        // Border and Text color (Blue for Elections)
        QColor mainColor = QColor("#2980B9");

        // Highlight logic if user clicks the election
        if (option.state & QStyle::State_Selected)
        {
            painter->setBrush(QColor("#EAF2F8")); // Light blue background when selected
            mainColor = QColor("#1A5276");        // Darker blue text/border
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
// CANDIDATE STATUS BOX DELEGATE (UPDATED)
// ==========================================
class CandidateDelegate : public QStyledItemDelegate {
public:
    explicit CandidateDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(option.rect.width(), 95); // Made it taller to fit the image and new text!
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        painter->setBrush(Qt::white);

        int statusInt = index.data(CandidateStatusRole).toInt();
        ApprovalStatus status = static_cast<ApprovalStatus>(statusInt);

        QColor statusColor;
        switch(status) {
        case ApprovalStatus::Pending: statusColor = QColor("#F39C12"); break;
        case ApprovalStatus::Approved: statusColor = QColor("#27AE60"); break;
        case ApprovalStatus::Rejected: statusColor = QColor("#C0392B"); break;
        default: statusColor = QColor("#7F8C8D"); break;
        }

        if (option.state & QStyle::State_Selected) painter->setBrush(QColor("#F8F9F9"));

        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        // 1. Draw Text (Leaving space on the right for the image)
        painter->setPen(statusColor);
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect = rect.adjusted(15, 5, -80, -5); // -80 cuts off the right side so text doesn't overlap the image

        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);

        // 2. Draw Symbol Image
        QString b64Image = index.data(CandidateSymbolImageRole).toString();
        if (!b64Image.isEmpty()) {
            QByteArray imgData = QByteArray::fromBase64(b64Image.toUtf8());
            QPixmap pixmap;
            if (pixmap.loadFromData(imgData)) {
                // Draw a 60x60 image on the far right
                QRect imgRect(rect.right() - 75, rect.top() + 17, 60, 60);
                QPixmap scaledPix = pixmap.scaled(imgRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

                // Centers the image exactly in the 60x60 box
                QPoint centerTarget = imgRect.center() - scaledPix.rect().center();
                painter->drawPixmap(centerTarget, scaledPix);
            }
        }

        painter->restore();
    }
};

// ==========================================
// ADMIN STATUS BOX DELEGATE
// ==========================================
class AdminDelegate : public QStyledItemDelegate
{
    Q_OBJECT // Required for custom signals!

        signals :
        // Emitted when the user clicks the "Change Status" button
        void actionButtonClicked(const QModelIndex &index, QPoint globalPos) const;

public:
    explicit AdminDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // Make the box taller to fit the button
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), 85);
    }

    // 1. UPDATE EDITOR EVENT (Only allow clicks if Pending)
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {

            // Check if the admin is actually Pending!
            int statusInt = index.data(AdminStatusRole).toInt();
            if (static_cast<ApprovalStatus>(statusInt) == ApprovalStatus::Pending) {

                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                QRect btnRect(option.rect.right() - 135, option.rect.bottom() - 35, 120, 25);

                if (btnRect.contains(mouseEvent->pos())) {
                    int localVote = index.data(LocalVoteRole).toInt();
                    if (localVote == 0) {
                        emit actionButtonClicked(index, mouseEvent->globalPosition().toPoint());
                    }
                    return true;
                }
            }
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        painter->setBrush(Qt::white);

        int statusInt = index.data(AdminCustomRoles::AdminStatusRole).toInt();
        ApprovalStatus status = static_cast<ApprovalStatus>(statusInt);

        QColor statusColor;
        switch (status)
        {
        case ApprovalStatus::Pending:
            statusColor = QColor("#F39C12");
            break;
        case ApprovalStatus::Approved:
            statusColor = QColor("#27AE60");
            break;
        case ApprovalStatus::Rejected:
            statusColor = QColor("#C0392B");
            break;
        default:
            statusColor = QColor("#7F8C8D");
            break;
        }

        if (option.state & QStyle::State_Selected)
            painter->setBrush(QColor("#F8F9F9"));

        // Draw main border
        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        // Draw main Text
        painter->setPen(statusColor);
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect = rect.adjusted(15, 5, -150, -5); // Leave space on the right for the button

        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);

        // ==========================================
        // DRAW THE ACTION BUTTON
        // ==========================================
        if (static_cast<ApprovalStatus>(statusInt) == ApprovalStatus::Pending){
        QRect btnRect(rect.right() - 135, rect.bottom() - 35, 120, 25);
        int localVoteInt = index.data(LocalVoteRole).toInt();

        QColor btnColor;
        QString btnText;

        // Color Logic Based on your requirements
        if (localVoteInt == 1)
        {
            btnColor = QColor("#27AE60"); // Green
            btnText = "Approved";
        }
        else if (localVoteInt == 2)
        {
            btnColor = QColor("#C0392B"); // Red
            btnText = "Rejected";
        }
        else
        {
            btnColor = QColor("#2980B9"); // Sidebar Dark Blue
            btnText = "Change Status ▾";
        }

        painter->setBrush(QColor("#FFFFFF")); // White background
        painter->setPen(QPen(btnColor, 2));   // Colored border
        painter->drawRoundedRect(btnRect, 4, 4);

        painter->setPen(btnColor); // Colored text
        QFont btnFont = option.font;
        btnFont.setPointSize(9);
        btnFont.setBold(true);
        painter->setFont(btnFont);
        painter->drawText(btnRect, Qt::AlignCenter, btnText);
        }

        painter->restore();
    }
};

// ==========================================
// ELECTION ACCORDION DELEGATE
// ==========================================
class ElectionAccordionDelegate : public QStyledItemDelegate {
    Q_OBJECT
signals:
    void electionClicked(const QModelIndex &index) const;

public:
    explicit ElectionAccordionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // Dynamic Height: 60px collapsed, 120px expanded
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool isExpanded = index.data(ElectionExpandedRole).toBool();
        return QSize(option.rect.width(), isExpanded ? 120 : 60);
    }

    // Catch the mouse click anywhere on the box to trigger the expand/collapse
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            emit electionClicked(index);
            return true;
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        bool isExpanded = index.data(ElectionExpandedRole).toBool();
        painter->setBrush(Qt::white);

        int statusInt = index.data(ElectionStatusRole).toInt();
        ElectionState status = static_cast<ElectionState>(statusInt);

        QColor statusColor;
        QString statusText;
        switch(status) {
        case ElectionState::Draft: statusColor = QColor("#95A5A6"); statusText = "Draft"; break; // Gray
        case ElectionState::Rejected: statusColor = QColor("#E74C3C"); statusText = "Rejected"; break; // Red
        case ElectionState::Published: statusColor = QColor("#3498DB"); statusText = "Published"; break; // Blue
        case ElectionState::VotingOpen: statusColor = QColor("#2ECC71"); statusText = "Voting Open"; break; // Green
        case ElectionState::VotingClosed: statusColor = QColor("#F39C12"); statusText = "Voting Closed"; break; // Orange
        case ElectionState::ResultsAnnounced: statusColor = QColor("#9B59B6"); statusText = "Results Announced"; break; // Purple
        default: statusColor = QColor("#34495E"); statusText = "Unknown"; break;
        }

        if (option.state & QStyle::State_Selected) painter->setBrush(QColor("#F8F9F9"));

        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        // --- TITLE ---
        QString title = index.data(Qt::DisplayRole).toString();
        QRect titleRect = rect.adjusted(15, 10, -150, isExpanded ? -80 : 0);
        QFont titleFont = option.font;
        titleFont.setBold(true);
        titleFont.setPointSize(11);
        painter->setFont(titleFont);
        painter->setPen(QColor("#2C3E50")); // Dark grey for title
        painter->drawText(titleRect, Qt::AlignLeft | (isExpanded ? Qt::AlignTop : Qt::AlignVCenter), title);

        // --- STATUS TEXT (Top Right) ---
        QRect statusRect = rect.adjusted(0, 10, -15, isExpanded ? -80 : 0);
        painter->setPen(statusColor);
        painter->drawText(statusRect, Qt::AlignRight | (isExpanded ? Qt::AlignTop : Qt::AlignVCenter), statusText);

        // --- EXPANDED DETAILS ---
        if (isExpanded) {
            QString startTime = "Start: " + index.data(ElectionStartTimeRole).toString();
            QString endTime = "End: " + index.data(ElectionEndTimeRole).toString();

            painter->setPen(QColor("#7F8C8D")); // Light grey text for times
            QFont detailFont = option.font;
            detailFont.setPointSize(9);
            painter->setFont(detailFont);

            QRect startRect = rect.adjusted(15, 45, -15, 0);
            QRect endRect = rect.adjusted(15, 70, -15, 0);

            painter->drawText(startRect, Qt::AlignLeft | Qt::AlignTop, startTime);
            painter->drawText(endRect, Qt::AlignLeft | Qt::AlignTop, endTime);


        }

        painter->restore();
    }
};

// ==========================================
// TOKEN ACCORDION DELEGATE (RIGHT-SIDE QR LAYOUT)
// ==========================================
class TokenAccordionDelegate : public QStyledItemDelegate {
    Q_OBJECT
signals:
    void tokenClicked(const QModelIndex &index) const;
    void sendEmailClicked(const QModelIndex &index) const;

public:
    explicit TokenAccordionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    // Increased expanded height to 240px to fit QR and Button stacked perfectly
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool isExpanded = index.data(TokenExpandedRole).toBool();
        return QSize(option.rect.width(), isExpanded ? 240 : 65);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            bool isExpanded = index.data(TokenExpandedRole).toBool();

            if (isExpanded) {
                // NEW BUTTON COORDINATES (Stacked under the QR code on the right)
                QRect btnRect(option.rect.right() - 150, option.rect.top() + 190, 130, 30);
                if (btnRect.contains(mouseEvent->pos())) {
                    emit sendEmailClicked(index);
                    return true;
                }
            }

            emit tokenClicked(index);
            return true;
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        bool isExpanded = index.data(TokenExpandedRole).toBool();

        painter->setBrush(Qt::white);
        QColor borderColor = QColor("#3498DB");

        if (option.state & QStyle::State_Selected && !isExpanded) painter->setBrush(QColor("#EAF2F8"));

        painter->setPen(QPen(borderColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        // --- TITLE ---
        QString title = "🎟️ " + index.data(Qt::DisplayRole).toString();
        QRect titleRect = rect.adjusted(15, 15, -15, isExpanded ? -200 : 0);
        QFont titleFont = option.font;
        titleFont.setBold(true);
        titleFont.setPointSize(12);
        painter->setFont(titleFont);
        painter->setPen(QColor("#2C3E50"));
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, title);

        // --- EXPANDED DETAILS ---
        if (isExpanded) {
            painter->setPen(QPen(QColor("#ECF0F1"), 1, Qt::DashLine));
            painter->drawLine(rect.left() + 15, rect.top() + 45, rect.right() - 15, rect.top() + 45);

            // 1. DRAW TEXT DETAILS (Moved to the LEFT side)
            QString issueDate = "Issued: " + index.data(TokenIssueDateRole).toString();
            QString station = "Station: " + index.data(TokenStationRole).toString();

            // We can show more of the hash now because it has the whole left side!
            QString signature = "Hash: " + index.data(TokenSignatureRole).toString().left(45) + "...";

            painter->setPen(QColor("#34495E"));
            QFont detailFont = option.font; detailFont.setPointSize(10);
            painter->setFont(detailFont);

            painter->drawText(rect.left() + 20, rect.top() + 70, station);
            painter->drawText(rect.left() + 20, rect.top() + 100, issueDate);

            painter->setPen(QColor("#E74C3C"));
            QFont monoFont("Courier New"); monoFont.setPointSize(10); monoFont.setBold(true);
            painter->setFont(monoFont);
            painter->drawText(rect.left() + 20, rect.top() + 140, signature);

            // 2. DRAW REAL QR CODE IMAGE (Moved to the RIGHT side)
            QRect qrRect(rect.right() - 150, rect.top() + 60, 130, 120);

            QImage qrImage = qvariant_cast<QImage>(index.data(TokenQRCodeRole));

            if (!qrImage.isNull()) {
                QImage scaledQr = qrImage.scaled(qrRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
                painter->drawImage(qrRect.topLeft(), scaledQr);
                painter->setBrush(Qt::NoBrush);
                painter->setPen(QPen(QColor("#BDC3C7"), 1));
                painter->drawRoundedRect(qrRect, 4, 4);
            } else {
                painter->setBrush(QColor("#F4F6F6"));
                painter->setPen(QPen(QColor("#BDC3C7"), 2));
                painter->drawRoundedRect(qrRect, 4, 4);

                painter->setPen(QColor("#7F8C8D"));
                QFont qrFont = option.font; qrFont.setPointSize(8); qrFont.setBold(true);
                painter->setFont(qrFont);
                painter->drawText(qrRect, Qt::AlignCenter, "QR CODE\nFAILED");
            }

            // 3. DRAW "SEND TO EMAIL" BUTTON (Stacked right UNDER the QR Code!)
            QRect btnRect(rect.right() - 150, rect.top() + 190, 130, 30);
            painter->setBrush(QColor("#27AE60"));
            painter->setPen(Qt::NoPen);
            painter->drawRoundedRect(btnRect, 4, 4);

            painter->setPen(Qt::white);
            QFont btnFont = option.font; btnFont.setPointSize(9); btnFont.setBold(true);
            painter->setFont(btnFont);
            painter->drawText(btnRect, Qt::AlignCenter, "✉ Send to Email");
        }

        painter->restore();
    }
};
#endif // DELEGATES_H
