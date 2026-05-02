#include "AdminCreateElectionPage.h"

AdminCreateElectionPage::AdminCreateElectionPage(QWidget *parent) : QWidget(parent) {
    setupUi();
    resetForm(); // Set the initial date rules immediately
}

void AdminCreateElectionPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ==========================================
    // 1. TOP HEADER
    // ==========================================
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
    // Add a dummy widget on the right to keep the title perfectly centered
    QWidget *dummy = new QWidget(); dummy->setFixedWidth(backBtn->sizeHint().width());
    headerLayout->addWidget(dummy);

    // ==========================================
    // 2. THE FORM CARD
    // ==========================================
    QWidget *formCard = new QWidget(this);
    formCard->setStyleSheet("QWidget { background: white; border-radius: 8px; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(30);
    formLayout->setVerticalSpacing(25);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // --- Styling Helpers ---
    auto createLabel = [this](const QString &text) {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet("font-size: 16px; color: #34495E; font-weight: bold;");
        return lbl;
    };

    QString inputStyle = "QLineEdit, QDateTimeEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 10px; font-size: 16px; color: #2C3E50; background-color: #F8F9F9; }"
                         "QLineEdit:focus, QDateTimeEdit:focus { border: 2px solid #3498DB; background-color: #FFFFFF; }";

    // --- Inputs ---
    titleInput = new QLineEdit(this);
    titleInput->setPlaceholderText("e.g., General Assembly Election 2026");
    titleInput->setStyleSheet(inputStyle);

    // Date Time Pickers
    publishTimeEdit = new QDateTimeEdit(this);
    startTimeEdit = new QDateTimeEdit(this);
    endTimeEdit = new QDateTimeEdit(this);

    // Format them beautifully
    QList<QDateTimeEdit*> pickers = {publishTimeEdit, startTimeEdit, endTimeEdit};
    for(QDateTimeEdit* picker : pickers) {
        picker->setCalendarPopup(true); // Turns it into a drop-down calendar!
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

    // ==========================================
    // 3. SUBMIT BUTTON
    // ==========================================
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

    // Assemble Page
    mainLayout->addLayout(headerLayout);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(formCard);
    mainLayout->addStretch(); // Pushes card to the top

    // ==========================================
    // 4. CONNECTIONS
    // ==========================================
    connect(backBtn, &QPushButton::clicked, this, &AdminCreateElectionPage::backBtnClicked);
    connect(submitBtn, &QPushButton::clicked, this, &AdminCreateElectionPage::onSubmitClicked);

    // Dynamic Date Rules!
    connect(publishTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AdminCreateElectionPage::onPublishTimeChanged);
    connect(startTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &AdminCreateElectionPage::onStartTimeChanged);
}

// ---------------------------------------------------------
// LOGIC: Sets up the initial rules when page is opened
// ---------------------------------------------------------
void AdminCreateElectionPage::resetForm() {
    titleInput->clear();

    QDateTime now = QDateTime::currentDateTime();
    QDateTime minPublish = now.addDays(2); // "At least two days after today"

    // 1. Set Publishing Rules
    publishTimeEdit->setMinimumDateTime(minPublish);
    publishTimeEdit->setDateTime(minPublish);

    // 2. Set Start Rules (Must be after publish)
    QDateTime minStart = minPublish.addDays(5); // 1 hour after publish default
    startTimeEdit->setMinimumDateTime(minStart);
    startTimeEdit->setDateTime(minStart);

    // 3. Set End Rules (Must be after start)
    QDateTime minEnd = minStart.addDays(2); // 1 day after start default
    endTimeEdit->setMinimumDateTime(minEnd);
    endTimeEdit->setDateTime(minEnd);
}

// ---------------------------------------------------------
// LOGIC: DYNAMIC DATE CONSTRAINTS
// ---------------------------------------------------------
void AdminCreateElectionPage::onPublishTimeChanged(const QDateTime &newDateTime) {
    // If Admin moves Publish Time forward, Start Time MUST move forward too!
    if(startTimeEdit->dateTime() <= newDateTime) {
        startTimeEdit->setMinimumDateTime(newDateTime.addSecs(3600)); // Buffer of 1 hour
    } else {
        startTimeEdit->setMinimumDateTime(newDateTime);
    }
}

void AdminCreateElectionPage::onStartTimeChanged(const QDateTime &newDateTime) {
    // If Admin moves Start Time forward, End Time MUST move forward too!
    if(endTimeEdit->dateTime() <= newDateTime) {
        endTimeEdit->setMinimumDateTime(newDateTime.addSecs(3600)); // Buffer of 1 hour
    } else {
        endTimeEdit->setMinimumDateTime(newDateTime);
    }
}

// ---------------------------------------------------------
// ACTION: SUBMIT BUTTON
// ---------------------------------------------------------
void AdminCreateElectionPage::onSubmitClicked() {
    QString title = titleInput->text().trimmed();

    if(title.isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter an Election Title.");
        titleInput->setFocus();
        return;
    }

    // Safety check just in case
    if(publishTimeEdit->dateTime() >= startTimeEdit->dateTime() || startTimeEdit->dateTime() >= endTimeEdit->dateTime()) {
        QMessageBox::critical(this, "Time Error", "Chronology failed: Publish Time must be before Start Time, and Start Time must be before End Time.");
        return;
    }

    emit createElectionRequested(title, publishTimeEdit->dateTime(), startTimeEdit->dateTime(), endTimeEdit->dateTime());
}
