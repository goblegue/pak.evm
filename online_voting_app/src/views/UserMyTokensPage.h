#ifndef USERMYTOKENSPAGE_H
#define USERMYTOKENSPAGE_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QFrame>
#include "models/entities/voters.h"
#include "models/Models.h"

class UserMyTokensPage : public QWidget {
    Q_OBJECT

public:
    explicit UserMyTokensPage(QWidget *parent = nullptr);
    void loadTokens(Token* tokens, int size);

private slots:
    void onTokenClicked(const QModelIndex &index);

private:
    QListView *tokenListView;
    TokenListModel *tokenModel;

    QStackedWidget *rightStackedWidget;
    QWidget *placeholderWidget;
    QWidget *ticketContainer;

    // Digital Ticket Labels
    QLabel *ticketElectionIdLabel;
    QLabel *ticketIssueDateLabel;
    QLabel *ticketStationLabel;
    QLabel *hashStringLabel;

    void setupUi();
};

#endif // USERMYTOKENSPAGE_H
