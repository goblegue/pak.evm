#include "AdminCandidateDetailsPage.h"
#include "controllers/auth_manager.h"

#include <QByteArray>

AdminCandidateDetailsPage::AdminCandidateDetailsPage(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void AdminCandidateDetailsPage::setupUi() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);

    // ==========================================
    // 1. TOP HEADER (Back Btn, Center Title, Status)
    // ==========================================
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backBtn = new QPushButton("← Back", this);
    backBtn->setCursor(Qt::PointingHandCursor);
    backBtn->setStyleSheet("QPushButton { border: none; color: #2980B9; font-weight: bold; font-size: 18px; }"
                           "QPushButton:hover { color: #1A5276; text-decoration: underline; }");

    headerTitleLabel = new QLabel("Candidate Details", this);
    headerTitleLabel->setAlignment(Qt::AlignCenter);
    headerTitleLabel->setStyleSheet("font-size: 26px; font-weight: bold; color: #2C3E50;"); // BIGGER FONT

    statusLabel = new QLabel("STATUS", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setFixedSize(120, 35);
    statusLabel->setStyleSheet("font-size: 14px; font-weight: bold;");

    headerLayout->addWidget(backBtn, 0, Qt::AlignLeft);
    headerLayout->addStretch(1);
    headerLayout->addWidget(headerTitleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch(1);
    headerLayout->addWidget(statusLabel, 0, Qt::AlignRight);

    // ==========================================
    // 2. SCROLL AREA
    // ==========================================
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget *scrollContent = new QWidget(scrollArea);
    scrollContent->setStyleSheet("QWidget { background: white; border-radius: 8px; }");
    QVBoxLayout *contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(25);

    // ==========================================
    // 3. TOP PROFILE SECTION (SIDE-BY-SIDE FIX)
    // ==========================================
    // We use an HBoxLayout to put Images on the Left, and Details on the Right!
    QHBoxLayout *topProfileLayout = new QHBoxLayout();

    // --- Left Side: Images ---
    QVBoxLayout *imagesLayout = new QVBoxLayout();
    profilePicLabel = new QLabel("No Profile", this);
    profilePicLabel->setFixedSize(150, 150); // INCREASED SIZE
    profilePicLabel->setAlignment(Qt::AlignCenter);
    profilePicLabel->setStyleSheet("border: 3px solid #BDC3C7; border-radius: 75px;");

    symbolPicLabel = new QLabel("No Symbol", this);
    symbolPicLabel->setFixedSize(100, 100); // INCREASED SIZE
    symbolPicLabel->setAlignment(Qt::AlignCenter);
    symbolPicLabel->setStyleSheet("border: 2px solid #BDC3C7; border-radius: 8px;");

    imagesLayout->addWidget(profilePicLabel, 0, Qt::AlignHCenter);
    imagesLayout->addSpacing(15);
    imagesLayout->addWidget(symbolPicLabel, 0, Qt::AlignHCenter);
    imagesLayout->addStretch();

    // --- Right Side: Form Details ---
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(40);
    formLayout->setVerticalSpacing(20);
    formLayout->setLabelAlignment(Qt::AlignLeft);

    auto createKeyLabel =[this](const QString &text) {
        QLabel *lbl = new QLabel(text, this);
        lbl->setStyleSheet("font-size: 16px; color: #7F8C8D; font-weight: bold;"); // INCREASED FONT
        return lbl;
    };

    auto createValueLabel = [this]() {
        QLabel *lbl = new QLabel("-", this);
        lbl->setStyleSheet("font-size: 18px; color: #2C3E50; font-weight: bold;"); // INCREASED FONT
        lbl->setWordWrap(true);
        return lbl;
    };

    cnicLabel = createValueLabel();
    electionIdLabel = createValueLabel();
    partyNameLabel = createValueLabel();
    symbolNameLabel = createValueLabel();
    educationLabel = createValueLabel();

    formLayout->addRow(createKeyLabel("CNIC Number:"), cnicLabel);
    formLayout->addRow(createKeyLabel("Election ID:"), electionIdLabel);
    formLayout->addRow(createKeyLabel("Party Name:"), partyNameLabel);
    formLayout->addRow(createKeyLabel("Symbol Name:"), symbolNameLabel);
    formLayout->addRow(createKeyLabel("Education Level:"), educationLabel);

    // Merge Images and Form side-by-side
    topProfileLayout->addLayout(imagesLayout, 1); // Images take 1 part space
    topProfileLayout->addSpacing(40);
    topProfileLayout->addLayout(formLayout, 3);   // Text takes 3 parts space (Fills the empty void!)

    // ==========================================
    // 4. LONG TEXT SECTION (Grey Boxes)
    // ==========================================
    // QLabel *historyTitle = new QLabel("Previous Political History", this);
    // historyTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2980B9; border-bottom: 2px solid #ECF0F1; padding-bottom: 5px;");

    // historyLabel = new QLabel("-", this);
    // historyLabel->setWordWrap(true);
    // historyLabel->setStyleSheet("font-size: 16px; color: #34495E; line-height: 1.6; background-color: #F8F9F9; padding: 15px; border-radius: 6px;"); // Grey Box

    QLabel *manifestoTitle = new QLabel("Election Manifesto", this);
    manifestoTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #2980B9; border-bottom: 2px solid #ECF0F1; padding-bottom: 5px; margin-top: 15px;");

    manifestoLabel = new QLabel("-", this);
    manifestoLabel->setWordWrap(true);
    manifestoLabel->setStyleSheet("font-size: 16px; color: #34495E; line-height: 1.6; background-color: #F8F9F9; padding: 15px; border-radius: 6px;"); // Grey Box

    // Assemble Content inside Scroll Area
    contentLayout->addLayout(topProfileLayout);
    // contentLayout->addWidget(historyTitle);
    // contentLayout->addWidget(historyLabel);
    contentLayout->addWidget(manifestoTitle);
    contentLayout->addWidget(manifestoLabel);
    contentLayout->addStretch();

    scrollArea->setWidget(scrollContent);

    // ==========================================
    // 5. ACTION BUTTONS (Approve / Reject)
    // ==========================================
    QHBoxLayout *actionLayout = new QHBoxLayout();

    approvalCountLabel = new QLabel("", this);
    approvalCountLabel->setStyleSheet("color: #7F8C8D; font-size: 14px; font-weight: bold; font-style: italic;");

    rejectBtn = new QPushButton("✖ Reject Candidate", this);
    rejectBtn->setCursor(Qt::PointingHandCursor);
    rejectBtn->setStyleSheet("QPushButton { background-color: #E74C3C; color: white; padding: 12px 25px; border-radius: 6px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #C0392B; }");

    approveBtn = new QPushButton("✔ Approve Candidate", this);
    approveBtn->setCursor(Qt::PointingHandCursor);
    approveBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; padding: 12px 25px; border-radius: 6px; font-size: 16px; font-weight: bold; } QPushButton:hover { background-color: #219653; }");

    actionLayout->addWidget(approvalCountLabel);
    actionLayout->addStretch();
    actionLayout->addWidget(rejectBtn);
    actionLayout->addWidget(approveBtn);

    // ==========================================
    // 6. FINAL ASSEMBLY & CONNECTIONS
    // ==========================================
    mainLayout->addLayout(headerLayout);
    mainLayout->addWidget(scrollArea);
    mainLayout->addLayout(actionLayout);

    connect(backBtn, &QPushButton::clicked, this, &AdminCandidateDetailsPage::backBtnClicked);
    connect(approveBtn, &QPushButton::clicked, this, &AdminCandidateDetailsPage::onApproveClicked);
    connect(rejectBtn, &QPushButton::clicked, this, &AdminCandidateDetailsPage::onRejectClicked); // This was missing in your old code!
}
// -------------------------------------------------------------------
// DATA INJECTION: Populates the UI with the Candidate's data
// -------------------------------------------------------------------
void AdminCandidateDetailsPage::setCandidate(const Candidate &candidate) {
    headerTitleLabel->setText("Candidate: " + candidate.getName());

    currentCandidateCnic = candidate.getUserCnic();

    // Handle Text
    cnicLabel->setText(candidate.getUserCnic());
    electionIdLabel->setText(candidate.getElectionId());
    partyNameLabel->setText(candidate.getPartyName());
    symbolNameLabel->setText(candidate.getSymbolName());
    educationLabel->setText(candidate.getEducationLevel());
    // historyLabel->setText(candidate.getPreviousHistory().isEmpty() ? "No history provided." : candidate.getPreviousHistory());
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

    QString currentAdminCnic = AuthManager::getInstance().getCurrentUser()->getCnic();
    if (candidate.getStatus() == ApprovalStatus::Approved || 
        candidate.getStatus() == ApprovalStatus::Rejected ||
        candidate.hasAdminVoted(currentAdminCnic)) {
        approveBtn->hide();
        rejectBtn->hide();
    } else {
        approveBtn->show();
        rejectBtn->show();
    }

    if (m_isUserMode) {
        approveBtn->hide();
        rejectBtn->hide();
        approvalCountLabel->hide();
    } else {
        if (candidate.getStatus() == ApprovalStatus::Approved || candidate.getStatus() == ApprovalStatus::Rejected) {
            approveBtn->hide();
            rejectBtn->hide();
        } else {
            approveBtn->show();
            rejectBtn->show();
        }
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

void AdminCandidateDetailsPage::setUserMode(bool isUserMode) {
    m_isUserMode = isUserMode;
    if (m_isUserMode) {
        approveBtn->hide();
        rejectBtn->hide();
        approvalCountLabel->hide();
    }
}
