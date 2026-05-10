#include "AdminCreateElectionPage.h"

AdminCreateElectionPage::AdminCreateElectionPage(QWidget *parent) : QWidget(parent) {
    setupUi();
    resetForm(); 
}

void AdminCreateElectionPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backBtn = new QPushButton("← Back", this);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("QPushButton { border: none; color: #2980B9; font-weight: bold; font-size: 18px; }"
                           "QPushButton:hover { color: #1A5276; text-decoration: underline; }");

    QLabel *headerTitleLabel = new QLabel("Create New Election", this);
    headerTitleLabel->setAlignment(Qt::AlignCenter);
    headerTitleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #2C3E50;");

    headerLayout->addWidget(backBtn, 0, Qt::AlignLeft);
    headerLayout->addStretch(1);
    headerLayout->addWidget(headerTitleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch(1);
    
    QWidget *dummy = new QWidget(); dummy->setFixedWidth(backBtn->sizeHint().width());
    headerLayout->addWidget(dummy);

    
    QWidget *formCard = new QWidget(this);
    formCard->setStyleSheet("QWidget { background: white; border-radius: 8px; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(30);
    formLayout->setVerticalSpacing(25);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    
    auto createLabel = [this](const QString &text) {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet("font-size: 16px; color: #34495E; font-weight: bold;");
        return lbl;
    };

    QString inputStyle = "QLineEdit, QDateTimeEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px; font-size: 16px; color: #2C3E50; background-color: #F8F9F9; }"
                         "QLineEdit:focus, QDateTimeEdit:focus { border: 2px solid #3498DB; background-color: #FFFFFF; }";

    
    titleInput = new QLineEdit(this);
    titleInput->setPlaceholderText("e.g., General Assembly Election 2026");
    titleInput->setStyleSheet(inputStyle);

    
    publishTimeEdit = new QDateTimeEdit(this);
    startTimeEdit = new QDateTimeEdit(this);
    endTimeEdit = new QDateTimeEdit(this);

    
    QList<QDateTimeEdit*> pickers = {publishTimeEdit, startTimeEdit, endTimeEdit};
    for(QDateTimeEdit* picker : pickers) {
        picker->setCalendarPopup(true); 
        picker->setDisplayFormat("MMM dd, yyyy  -  hh:mm AP");
        picker->setStyleSheet(inputStyle);
        picker->setCursor(Qt::PointingHandCursor);
    }

    formLayout->addRow(createLabel("Election Title:"), titleInput);
    formLayout->addRow(new QLabel(" "), new QLabel(" ")); // Spacer
    formLayout->addRow(createLabel("Publishing Time:"), publishTimeEdit);
    formLayout->addRow(createLabel("Voting Start Time:"), startTimeEdit);
    formLayout->addRow(createLabel("Voting End Time:"), endTimeEdit);

    cardLayout->addLayout(formLayout);

    
    QHBoxLayout *actionLayout = new QHBoxLayout();
    submitBtn = new QPushButton("✔ Create Election (Save as Draft)", this);
    submitBtn->setCursor(Qt::PointingHandCursor);
    submitBtn->setFixedSize(350, 50);
    submitBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; border-radius: 6px; font-size: 18px; font-weight: bold; }"
                             "QPushButton:hover { background-color: #219653; }");

    actionLayout->addStretch();
    actionLayout->addWidget(submitBtn);
    actionLayout->addStretch();

    cardLayout->addSpacing(30);
    cardLayout->addLayout(actionLayout);

    
    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(formCard);
    mainLayout->addStretch(); 

    
    connect(backBtn, &QPushButton::clicked, this, &AdminCreateElectionPage::backBtnClicked);
    connect(submitBtn, &QPushButton::clicked, this, &AdminCreateElectionPage::onSubmitClicked);

    
    connect(publishTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AdminCreateElectionPage::onPublishTimeChanged);
    connect(startTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AdminCreateElectionPage::onStartTimeChanged);
}


void AdminCreateElectionPage::resetForm() {
    titleInput->clear();

    QDateTime now = QDateTime::currentDateTime();
    QDateTime minPublish = now.addDays(2); 

    
    publishTimeEdit->setMinimumDateTime(minPublish);
    publishTimeEdit->setDateTime(minPublish);

    
    QDateTime minStart = minPublish.addDays(5); 
    startTimeEdit->setMinimumDateTime(minStart);
    startTimeEdit->setDateTime(minStart);

    
    QDateTime minEnd = minStart.addDays(2); 
    endTimeEdit->setMinimumDateTime(minEnd);
    endTimeEdit->setDateTime(minEnd);
}


void AdminCreateElectionPage::onPublishTimeChanged(const QDateTime &newDateTime) {

    if(startTimeEdit->dateTime() <= newDateTime) {
        startTimeEdit->setMinimumDateTime(newDateTime.addSecs(3600)); 
    } else {
        startTimeEdit->setMinimumDateTime(newDateTime);
    }
}

void AdminCreateElectionPage::onStartTimeChanged(const QDateTime &newDateTime) {
    
    if(endTimeEdit->dateTime() <= newDateTime) {
        endTimeEdit->setMinimumDateTime(newDateTime.addSecs(3600)); 
    } else {
        endTimeEdit->setMinimumDateTime(newDateTime);
    }
}


void AdminCreateElectionPage::onSubmitClicked() {
    QString title = titleInput->text().trimmed();

    if(title.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter an Election Title.");
        titleInput->setFocus();
        return;
    }

    
    if(publishTimeEdit->dateTime() >= startTimeEdit->dateTime() || startTimeEdit->dateTime() >= endTimeEdit->dateTime()) {
        QMessageBox::critical(this, "Time Error", "Chronology failed: Publish Time must be before Start Time, and Start Time must be before End Time.");
        return;
    }

    emit createElectionRequested(title, publishTimeEdit->dateTime(), startTimeEdit->dateTime(), endTimeEdit->dateTime());
}
