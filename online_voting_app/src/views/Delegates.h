#ifndef DELEGATES_H
#define DELEGATES_H

#include <QImage>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QMouseEvent>
#include "models/Models.h"
#include <QApplication>
#include "views/AdminCandidatePage.h"


class ElectionDelegate : public QStyledItemDelegate
{
public:
    explicit ElectionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), 55);
    }

    
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;

        
        painter->setBrush(Qt::white);

        
        QColor mainColor = QColor("#2980B9");

        
        if (option.state & QStyle::State_Selected)
        {
            painter->setBrush(QColor("#EAF2F8"));
            mainColor = QColor("#1A5276");       
        }
        else if (option.state & QStyle::State_MouseOver) {
            painter->setBrush(QColor("#F4F6F7"));
        }

        painter->setPen(QPen(mainColor, 2)); 

        
        painter->drawRoundedRect(rect, 8, 8);

        
        painter->setPen(mainColor);
        QString text = index.data(Qt::DisplayRole).toString();

        
        QRect textRect = rect.adjusted(15, 0, -15, 0);

        
        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

        painter->restore();
    }
};


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
        if (option.state & QStyle::State_MouseOver) painter->setBrush(QColor("#F4F6F7"));


        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        
        painter->setPen(statusColor);
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect = rect.adjusted(15, 5, -80, -5); 

        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);

        
        QString b64Image = index.data(CandidateSymbolImageRole).toString();
        if (!b64Image.isEmpty()) {
            QByteArray imgData = QByteArray::fromBase64(b64Image.toUtf8());
            QPixmap pixmap;
            if (pixmap.loadFromData(imgData)) {
                
                QRect imgRect(rect.right() - 75, rect.top() + 17, 60, 60);
                QPixmap scaledPix = pixmap.scaled(imgRect.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

                
                QPoint centerTarget = imgRect.center() - scaledPix.rect().center();
                painter->drawPixmap(centerTarget, scaledPix);
            }
        }

        painter->restore();
    }
};


class AdminDelegate : public QStyledItemDelegate
{
    Q_OBJECT
signals:
    void actionButtonClicked(const QModelIndex &index, QPoint globalPos) const;

public:
    explicit AdminDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        return QSize(option.rect.width(), 85);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            int statusInt = index.data(AdminStatusRole).toInt();

            if (static_cast<ApprovalStatus>(statusInt) == ApprovalStatus::Pending) {
                bool alreadyVoted = index.data(AdminVotedRole).toBool();
                int localVote = index.data(LocalVoteRole).toInt();

                
                if (!alreadyVoted && localVote == 0) {
                    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                    QRect btnRect(option.rect.right() - 135, option.rect.bottom() - 35, 120, 25);

                    if (btnRect.contains(mouseEvent->pos())) {
                        emit actionButtonClicked(index, mouseEvent->globalPosition().toPoint());
                        return true;
                    }
                }
            }
        }
        return QStyledItemDelegate::editorEvent(event, model, option, index);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        QRect rect = option.rect;
        painter->setBrush(Qt::white);

        int statusInt = index.data(AdminCustomRoles::AdminStatusRole).toInt();
        ApprovalStatus status = static_cast<ApprovalStatus>(statusInt);

        QColor statusColor;
        switch (status) {
        case ApprovalStatus::Pending: statusColor = QColor("#F39C12"); break;
        case ApprovalStatus::Approved: statusColor = QColor("#27AE60"); break;
        case ApprovalStatus::Rejected: statusColor = QColor("#C0392B"); break;
        default: statusColor = QColor("#7F8C8D"); break;
        }

        if (option.state & QStyle::State_Selected) painter->setBrush(QColor("#F8F9F9"));
        else if (option.state & QStyle::State_MouseOver) painter->setBrush(QColor("#F4F6F7"));

        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        painter->setPen(statusColor);
        QString text = index.data(Qt::DisplayRole).toString();
        QRect textRect = rect.adjusted(15, 5, -150, -5);

        QFont font = option.font; font.setBold(true); painter->setFont(font);
        painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextWordWrap, text);

        
        if (static_cast<ApprovalStatus>(statusInt) == ApprovalStatus::Pending) {
            bool alreadyVoted = index.data(AdminVotedRole).toBool();
            int localVoteInt = index.data(LocalVoteRole).toInt();

            QRect btnRect(rect.right() - 135, rect.bottom() - 35, 120, 25);

            
            if (alreadyVoted || localVoteInt != 0) {
                painter->setBrush(QColor("#ECF0F1"));
                painter->setPen(QPen(QColor("#BDC3C7"), 1));
                painter->drawRoundedRect(btnRect, 4, 4);

                painter->setPen(QColor("#7F8C8D"));
                QFont btnFont = option.font; btnFont.setBold(true); btnFont.setPointSize(9);
                painter->setFont(btnFont);
                painter->drawText(btnRect, Qt::AlignCenter, "✔ Voted");

            } else {
                
                QColor btnBgColor = Qt::white;
                if (option.state & QStyle::State_MouseOver && btnRect.contains(option.widget->mapFromGlobal(QCursor::pos()))) {
                    btnBgColor = QColor("#ECF0F1");
                }

                painter->setBrush(btnBgColor);
                painter->setPen(QPen(QColor("#3498DB"), 2));
                painter->drawRoundedRect(btnRect, 4, 4);

                painter->setPen(QColor("#3498DB"));
                QFont btnFont = option.font; btnFont.setPointSize(9); btnFont.setBold(true);
                painter->setFont(btnFont);
                painter->drawText(btnRect, Qt::AlignCenter, "Change Status ▾");
            }
        }
        painter->restore();
    }
};


class ElectionAccordionDelegate : public QStyledItemDelegate {
    Q_OBJECT
signals:
    void electionClicked(const QModelIndex &index) const;
    void actionButtonClicked(const QModelIndex &index, QPoint globalPos) const;
    void getConfigButtonClicked(const QModelIndex &index) const; // NEW SIGNAL

public:
    explicit ElectionAccordionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool isExpanded = index.data(ElectionExpandedRole).toBool();
        return QSize(option.rect.width(), isExpanded ? 130 : 60);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            bool isExpanded = index.data(ElectionExpandedRole).toBool();
            int statusInt = index.data(ElectionStatusRole).toInt();
            ElectionState status = static_cast<ElectionState>(statusInt);

            if (isExpanded) {
                
                bool hasStatusBtn = (status == ElectionState::Pending);
                int configBtnOffset = hasStatusBtn ? 290 : 145;

                
                QRect configBtnRect(option.rect.right() - configBtnOffset, option.rect.bottom() - 40, 140, 28);
                if (configBtnRect.contains(mouseEvent->pos())) {
                    emit getConfigButtonClicked(index);
                    return true;
                }

                
                if (hasStatusBtn) {
                    bool alreadyVoted = index.data(ElectionVotedRole).toBool();
                    int localVote = index.data(LocalVoteRole).toInt();

                    if(!alreadyVoted && localVote == 0) {
                        QRect btnRect(option.rect.right() - 145, option.rect.bottom() - 40, 130, 28);
                        if (btnRect.contains(mouseEvent->pos())) {
                            emit actionButtonClicked(index, mouseEvent->globalPosition().toPoint());
                            return true;
                        }
                    }
                }
            }
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
        case ElectionState::Drafted: statusColor = QColor("#95A5A6"); statusText = "Draft"; break;
        case ElectionState::Pending: statusColor = QColor("#F39C12"); statusText = "Pending"; break;
        case ElectionState::Rejected: statusColor = QColor("#E74C3C"); statusText = "Rejected"; break;
        case ElectionState::Published: statusColor = QColor("#3498DB"); statusText = "Published"; break;
        case ElectionState::VotingOpen: statusColor = QColor("#2ECC71"); statusText = "Voting Open"; break;
        case ElectionState::VotingClosed: statusColor = QColor("#F39C12"); statusText = "Voting Closed"; break;
        case ElectionState::ResultsAnnounced: statusColor = QColor("#9B59B6"); statusText = "Results Announced"; break;
        default: statusColor = QColor("#34495E"); statusText = "Unknown"; break;
        }

        if (option.state & QStyle::State_Selected) painter->setBrush(QColor("#F8F9F9"));

        painter->setPen(QPen(statusColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        QString title = index.data(Qt::DisplayRole).toString();
        QRect titleRect = rect.adjusted(15, 10, -150, isExpanded ? -90 : 0);
        QFont titleFont = option.font; titleFont.setBold(true); titleFont.setPointSize(11);
        painter->setFont(titleFont); painter->setPen(QColor("#2C3E50"));
        painter->drawText(titleRect, Qt::AlignLeft | (isExpanded ? Qt::AlignTop : Qt::AlignVCenter), title);

        QRect statusRect = rect.adjusted(0, 10, -15, isExpanded ? -90 : 0);
        painter->setPen(statusColor);
        painter->drawText(statusRect, Qt::AlignRight | (isExpanded ? Qt::AlignTop : Qt::AlignVCenter), statusText);

        if (isExpanded) {
            QString startTime = "Start: " + index.data(ElectionStartTimeRole).toString();
            QString endTime = "End: " + index.data(ElectionEndTimeRole).toString();

            painter->setPen(QColor("#7F8C8D"));
            QFont detailFont = option.font; detailFont.setPointSize(9);
            painter->setFont(detailFont);
            painter->drawText(rect.adjusted(15, 45, -15, 0), Qt::AlignLeft | Qt::AlignTop, startTime);
            painter->drawText(rect.adjusted(15, 70, -15, 0), Qt::AlignLeft | Qt::AlignTop, endTime);

            bool hasStatusBtn = (status == ElectionState::Pending);
            int configBtnOffset = hasStatusBtn ? 290 : 145;

           
            QRect configBtnRect(rect.right() - configBtnOffset, rect.bottom() - 40, 140, 28);
            QColor configBgColor = Qt::white;
            if (option.state & QStyle::State_MouseOver && configBtnRect.contains(option.widget->mapFromGlobal(QCursor::pos()))) {
                configBgColor = QColor("#ECF0F1");
            }
            painter->setBrush(configBgColor);
            painter->setPen(QPen(QColor("#2C3E50"), 2)); 
            painter->drawRoundedRect(configBtnRect, 4, 4);

            painter->setPen(QColor("#2C3E50"));
            QFont btnFont = option.font; btnFont.setBold(true); btnFont.setPointSize(9);
            painter->setFont(btnFont);
            painter->drawText(configBtnRect, Qt::AlignCenter, "Get Configuration");

            
            if (hasStatusBtn) {
                bool alreadyVoted = index.data(ElectionVotedRole).toBool();
                int localVoteInt = index.data(LocalVoteRole).toInt();
                QRect btnRect(rect.right() - 145, rect.bottom() - 40, 130, 28);

                if (alreadyVoted || localVoteInt != 0) {
                    painter->setBrush(QColor("#ECF0F1"));
                    painter->setPen(QPen(QColor("#BDC3C7"), 1));
                    painter->drawRoundedRect(btnRect, 4, 4);
                    painter->setPen(QColor("#7F8C8D"));
                    painter->setFont(btnFont);
                    painter->drawText(btnRect, Qt::AlignCenter, "✔ Voted");
                } else {
                    QColor btnBgColorStatus = Qt::white;
                    if (option.state & QStyle::State_MouseOver && btnRect.contains(option.widget->mapFromGlobal(QCursor::pos()))) {
                        btnBgColorStatus = QColor("#ECF0F1");
                    }
                    painter->setBrush(btnBgColorStatus);
                    painter->setPen(QPen(QColor("#3498DB"), 2));
                    painter->drawRoundedRect(btnRect, 4, 4);
                    painter->setPen(QColor("#3498DB"));
                    painter->setFont(btnFont);
                    painter->drawText(btnRect, Qt::AlignCenter, "Change Status ▾");
                }
            }
        }
        painter->restore();
    }
};



class TokenAccordionDelegate : public QStyledItemDelegate {
    Q_OBJECT
signals:
    void tokenClicked(const QModelIndex &index) const;
    void sendEmailClicked(const QModelIndex &index) const;

public:
    explicit TokenAccordionDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        bool isExpanded = index.data(TokenExpandedRole).toBool();
        return QSize(option.rect.width(), isExpanded ? 240 : 65);
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            bool isExpanded = index.data(TokenExpandedRole).toBool();

            if (isExpanded) {
                
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
        else if (option.state & QStyle::State_MouseOver && !isExpanded) painter->setBrush(QColor("#F4F6F7"));


        painter->setPen(QPen(borderColor, 2));
        painter->drawRoundedRect(rect, 8, 8);

        
        QString title = "🎟️ " + index.data(Qt::DisplayRole).toString();
        QRect titleRect = rect.adjusted(15, 15, -15, isExpanded ? -200 : 0);
        QFont titleFont = option.font;
        titleFont.setBold(true);
        titleFont.setPointSize(12);
        painter->setFont(titleFont);
        painter->setPen(QColor("#2C3E50"));
        painter->drawText(titleRect, Qt::AlignLeft | Qt::AlignTop, title);

        
        if (isExpanded) {
            painter->setPen(QPen(QColor("#ECF0F1"), 1, Qt::DashLine));
            painter->drawLine(rect.left() + 15, rect.top() + 45, rect.right() - 15, rect.top() + 45);

            
            QString issueDate = "Issued: " + index.data(TokenIssueDateRole).toString();
            QString station = "Station: " + index.data(TokenStationRole).toString();

            
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
#endif 
