#include "AdminCandidateDetailsPage.h"
#include <QByteArray>

AdminCandidateDetailsPage::AdminCandidateDetailsPage(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void AdminCandidateDetailsPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // ==========================================
    // 1. TOP HEADER (Back Btn, Center Title, Status)
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backBtn = new QPushButton("← Back", this);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("QPushButton { border: none; color: #2980B9; font-weight: bold; font-size: 16px; }"
                           "QPushButton:hover { color: #1A5276; text-decoration: underline; }");

    headerTitleLabel = new QLabel("Candidate Details", this);
    headerTitleLabel->setAlignment(Qt::AlignCenter);
    headerTitleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2C3E50;");

    statusLabel = new QLabel("STATUS", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedSize(100, 30); // Status Badge

    headerLayout->addWidget(backBtn, 0, Qt::AlignLeft);
    headerLayout->addStretch(1);
    headerLayout->addWidget(headerTitleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch(1);
    headerLayout->addWidget(statusLabel, 0, Qt::AlignRight);

    // ==========================================
    // 2. SCROLL AREA (For long manifestos)
    // ==========================================
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // Crucial! Makes inner widget stretch
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    // The container that goes INSIDE the scroll area
    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("QWidget { background: white; border-radius: 8px; }");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(20);

    // ==========================================
    // 3. IMAGES SECTION (Profile + Symbol)
    // ==========================================
    QHBoxLayout *imagesLayout = new QHBoxLayout();
    profilePicLabel = new QLabel("No Profile", this);
    profilePicLabel->setFixedSize(120, 120);
    profilePicLabel->setAlignment(Qt::AlignCenter);
    profilePicLabel->setStyleSheet("border: 2px solid #BDC3C7; border-radius: 60px;"); // Circle

    symbolPicLabel = new QLabel("No Symbol", this);
    symbolPicLabel->setFixedSize(80, 80);
    symbolPicLabel->setAlignment(Qt::AlignCenter);
    symbolPicLabel->setStyleSheet("border: 1px solid #BDC3C7;");

    imagesLayout->addWidget(profilePicLabel);
    imagesLayout->addSpacing(20);
    imagesLayout->addWidget(symbolPicLabel);
    imagesLayout->addStretch();

    // ==========================================
    // 4. SHORT DETAILS (Form Layout)
    // ==========================================
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(30);
    formLayout->setVerticalSpacing(15);
    formLayout->setLabelAlignment(Qt::AlignRight);

    // Helper macro for styling labels
    auto createValueLabel = [this]() {
        QLabel *lbl = new QLabel("-", this);
        lbl->setStyleSheet("font-size: 14px; color: #34495E; font-weight: bold;");
        return lbl;
    };

    cnicLabel = createValueLabel();
    electionIdLabel = createValueLabel();
    partyNameLabel = createValueLabel();
    symbolNameLabel = createValueLabel();
    educationLabel = createValueLabel();

    formLayout->addRow("<b>CNIC:</b>", cnicLabel);
    formLayout->addRow("<b>Election ID:</b>", electionIdLabel);
    formLayout->addRow("<b>Party Name:</b>", partyNameLabel);
    formLayout->addRow("<b>Symbol Name:</b>", symbolNameLabel);
    formLayout->addRow("<b>Education:</b>", educationLabel);

    // ==========================================
    // 5. LONG TEXT SECTION (History & Manifesto)
    // ==========================================
    QLabel *historyTitle = new QLabel("<b>Previous Political History</b>", this);
    historyTitle->setStyleSheet("font-size: 16px; color: #2980B9; margin-top: 10px;");
    historyLabel = new QLabel("-", this);
    historyLabel->setWordWrap(true); // CRITICAL: Wraps text to next line
    historyLabel->setStyleSheet("color: #2C3E50; line-height: 1.5;");

    QLabel *manifestoTitle = new QLabel("<b>Election Manifesto</b>", this);
    manifestoTitle->setStyleSheet("font-size: 16px; color: #2980B9; margin-top: 10px;");
    manifestoLabel = new QLabel("-", this);
    manifestoLabel->setWordWrap(true);
    manifestoLabel->setStyleSheet("color: #2C3E50; line-height: 1.5;");

    // Assemble the Scroll Content
    contentLayout->addLayout(imagesLayout);
    contentLayout->addLayout(formLayout);
    contentLayout->addWidget(historyTitle);
    contentLayout->addWidget(historyLabel);
    contentLayout->addWidget(manifestoTitle);
    contentLayout->addWidget(manifestoLabel);
    contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);

    // Assemble Main Page
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(scrollArea);

    // Connect Back Button
    connect(backBtn, &QPushButton::clicked, this, &AdminCandidateDetailsPage::backBtnClicked);

    // ==========================================
    // 6. ACTION BUTTONS (Approve / Reject)
    // ==========================================
    QHBoxLayout *actionLayout = new QHBoxLayout();

    approvalCountLabel = new QLabel("", this);
    approvalCountLabel->setStyleSheet("color: #7F8C8D; font-weight: bold; font-style: italic;");

    rejectBtn = new QPushButton("✖ Reject Candidate", this);
    rejectBtn->setCursor(Qt::PointingHandCursor);
    rejectBtn->setStyleSheet("QPushButton { background-color: #E74C3C; color: white; padding: 10px 20px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #C0392B; }");

    approveBtn = new QPushButton("✔ Approve Candidate", this);
    approveBtn->setCursor(Qt::PointingHandCursor);
    approveBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; padding: 10px 20px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #219653; }");

    actionLayout->addWidget(approvalCountLabel);
    actionLayout->addStretch();
    actionLayout->addWidget(rejectBtn);
    actionLayout->addWidget(approveBtn);

    // Add everything to the main layout
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(actionLayout); // ADD THIS HERE

    // Connect the buttons
    connect(approveBtn, &QPushButton::clicked, this, &AdminCandidateDetailsPage::onApproveClicked);

}

// -------------------------------------------------------------------
// DATA INJECTION: Populates the UI with the Candidate's data
// -------------------------------------------------------------------
void AdminCandidateDetailsPage::setCandidate(const Candidate &candidate) {
    headerTitleLabel->setText("Candidate: " + candidate.getPartyName());

    currentCandidateCnic = candidate.getUserCnic();

    // Handle Text
    cnicLabel->setText(candidate.getUserCnic());
    electionIdLabel->setText(candidate.getElectionId());
    partyNameLabel->setText(candidate.getPartyName());
    symbolNameLabel->setText(candidate.getSymbolName());
    educationLabel->setText(candidate.getEducationLevel());
    historyLabel->setText(candidate.getPreviousHistory().isEmpty() ? "No history provided." : candidate.getPreviousHistory());
    manifestoLabel->setText(candidate.getManifesto().isEmpty() ? "No manifesto provided." : candidate.getManifesto());

    // Handle Status Badge Colors
    ApprovalStatus status = candidate.getStatus();
    if(status == ApprovalStatus::Pending) {
        statusLabel->setText("PENDING");
        statusLabel->setStyleSheet("background-color: #F39C12; color: white; font-weight: bold; border-radius: 4px;");
    } else if(status == ApprovalStatus::Approved) {
        statusLabel->setText("APPROVED");
        statusLabel->setStyleSheet("background-color: #27AE60; color: white; font-weight: bold; border-radius: 4px;");
    } else {
        statusLabel->setText("REJECTED");
        statusLabel->setStyleSheet("background-color: #C0392B; color: white; font-weight: bold; border-radius: 4px;");
    }

    // Handle Images
    profilePicLabel->setPixmap(decodeBase64Image(candidate.getProfileImageBase64(), 120));
    symbolPicLabel->setPixmap(decodeBase64Image(candidate.getSymbolBase64(), 80));

    // Check how many admins have approved already using your custom OOP function!
    int currentApprovals = const_cast<Candidate&>(candidate).getStatusCount(ApprovalStatus::Approved);
    int currentRejections = const_cast<Candidate&>(candidate).getStatusCount(ApprovalStatus::Rejected);

    if (currentApprovals > 0) {
        approvalCountLabel->setText(QString("⏳ %1 Admin(s) have requested Approval.").arg(currentApprovals));
    } else if (currentRejections > 0) {
        approvalCountLabel->setText(QString("⚠️ %1 Admin(s) have requested Rejection.").arg(currentRejections));
    } else {
        approvalCountLabel->setText("No admin actions yet.");
    }

    // Optional: Hide buttons if already finalized
    if (candidate.getStatus() == ApprovalStatus::Approved || candidate.getStatus() == ApprovalStatus::Rejected) {
        approveBtn->hide();
        rejectBtn->hide();
    } else {
        approveBtn->show();
        rejectBtn->show();
    }
}


// Helper: Converts Base64 string to a Qt Image
QPixmap AdminCandidateDetailsPage::decodeBase64Image(const QString &base64Str, int expectedSize) {
    if(base64Str.isEmpty()) return QPixmap(); // Returns empty if no image

    QByteArray imageBytes = QByteArray::fromBase64(base64Str.toUtf8());
    QPixmap pixmap;
    pixmap.loadFromData(imageBytes);

    // Scale it down nicely so it fits the UI box perfectly
    return pixmap.scaled(expectedSize, expectedSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
}

// Emits the signal with the exact data your CandidateController needs!
void AdminCandidateDetailsPage::onApproveClicked() {
    emit candidateStatusChangeRequested(currentCandidateCnic, ApprovalStatus::Approved);
}

void AdminCandidateDetailsPage::onRejectClicked() {
    emit candidateStatusChangeRequested(currentCandidateCnic, ApprovalStatus::Rejected);
}
