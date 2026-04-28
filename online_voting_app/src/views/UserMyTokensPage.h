#ifndef USERMYTOKENSPAGE_H
#define USERMYTOKENSPAGE_H

#include <QWidget>
#include <QListView>
#include <QLabel>
#include <QVBoxLayout>
#include "models/entities/voters.h"
#include "models/Models.h"

class UserMyTokensPage : public QWidget {
    Q_OBJECT

public:
    explicit UserMyTokensPage(QWidget *parent = nullptr);
    void loadTokens(Token* tokens, int size);

signals:
    // Emits the full token object so MainWindow can send the email
    void emailTokenRequested(Token token);

private slots:
    void onTokenBoxClicked(const QModelIndex &index);
    void onSendEmailClicked(const QModelIndex &index);

private:
    QListView *tokenListView;
    TokenListModel *tokenModel;

    void setupUi();
};

#endif // USERMYTOKENSPAGE_H
