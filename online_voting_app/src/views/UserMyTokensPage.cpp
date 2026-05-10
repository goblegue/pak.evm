#include "UserMyTokensPage.h"
#include "Delegates.h"

UserMyTokensPage::UserMyTokensPage(QWidget *parent) : QWidget(parent) {
    tokenModel = new TokenListModel(this);
    setupUi();
}

void UserMyTokensPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    QLabel *titleLabel = new QLabel("<b>My Digital Tokens</b>", this);
    titleLabel->setStyleSheet("font-size: 24px; color: #2C3E50;");

    tokenListView = new QListView(this);
    tokenListView->setMouseTracking(true);
    tokenListView->setModel(tokenModel);
    tokenListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenListView->setSpacing(15);

    TokenAccordionDelegate *delegate = new TokenAccordionDelegate(this);
    tokenListView->setItemDelegate(delegate);
    tokenListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(tokenListView);

    
    connect(delegate, &TokenAccordionDelegate::tokenClicked, this, &UserMyTokensPage::onTokenBoxClicked);
    connect(delegate, &TokenAccordionDelegate::sendEmailClicked, this, &UserMyTokensPage::onSendEmailClicked);
}

void UserMyTokensPage::loadTokens(Token* tokens, int size) {
    tokenModel->setTokens(tokens, size);
}

void UserMyTokensPage::onTokenBoxClicked(const QModelIndex &index) {
    
    QString tokenId = tokenModel->getTokenAt(index.row()).getId();
    tokenModel->toggleExpanded(tokenId);
}

void UserMyTokensPage::onSendEmailClicked(const QModelIndex &index) {
    
    Token selectedToken = tokenModel->getTokenAt(index.row());
    emit emailTokenRequested(selectedToken);
}
