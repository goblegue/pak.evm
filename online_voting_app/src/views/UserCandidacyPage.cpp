#include "UserCandidacyPage.h"
#include "Delegates.h"
#include <QBuffer>
#include <QMessageBox>
#include <QFormLayout>
#include <QFrame>

UserCandidacyPage::UserCandidacyPage(QWidget *parent) : QWidget(parent) {
    electionModel = new ElectionListModel(this);
    setupUi();
}

void UserCandidacyPage::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(25);

    // ==========================================
    // LEFT SIDE: PUBLISHED ELECTIONS
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setMinimumWidth(350);
    leftPanel->setMaximumWidth(450);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *electionLabel = new QLabel("<b>Select Election to Run</b>", this);
    electionLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    // We use your awesome EmptyStateListView here!
    electionListView = new EmptyStateListView("There are currently no published elections.", this);
    electionListView->setModel(electionModel);
    electionListView->setMouseTracking(true); // For hover effect
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this));
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    electionListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);

    // ==========================================
    // RIGHT SIDE: FORM
    // ==========================================
    rightStackedWidget = new QStackedWidget(this);

    // --- PAGE 0: PLACEHOLDER ---
    placeholderWidget = new QWidget(this);
    QVBoxLayout *phLayout = new QVBoxLayout(placeholderWidget);
    QLabel *phLabel = new QLabel("👈 Select an election to begin your application.", this);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setStyleSheet("font-size: 18px; color: #7F8C8D; font-style: italic;");
    phLayout->addWidget(phLabel);
    rightStackedWidget->addWidget(placeholderWidget);

    // --- PAGE 1: CANDIDACY FORM ---
    formContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(formContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(15);

    QFrame *formCard = new QFrame(this);
    formCard->setStyleSheet("QFrame { background-color: white; border-radius: 8px; border: 1px solid #BDC3C7; }");
    QVBoxLayout *cardLayout = new QVBoxLayout(formCard);
    cardLayout->setContentsMargins(30, 30, 30, 30);

    formTitleLabel = new QLabel("Apply for Candidacy", this);
    formTitleLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: #2980B9; border: none; border-bottom: 2px solid #ECF0F1; padding-bottom: 10px;");
    cardLayout->addWidget(formTitleLabel);
    cardLayout->addSpacing(15);

    QFormLayout *formLayout = new QFormLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(20);

    QString inputStyle = "QLineEdit, QComboBox, QTextEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 8px; font-size: 14px; color: #2C3E50; }"
                         "QLineEdit:focus, QComboBox:focus, QTextEdit:focus { border: 2px solid #3498DB; }";
    QString labelStyle = "font-size: 14px; color: #34495E; font-weight: bold;";

    // 1. Party Name
    partyNameInput = new QLineEdit(this);
    partyNameInput->setStyleSheet(inputStyle);
    QLabel *l1 = new QLabel("Party Name:", this); l1->setStyleSheet(labelStyle);
    formLayout->addRow(l1, partyNameInput);

    // 2. Education Level
    educationCombo = new QComboBox(this);
    educationCombo->addItems({"Select Education...", "High School / Matric", "Intermediate", "Bachelor's Degree", "Master's Degree", "Doctorate / Ph.D."});
    educationCombo->setStyleSheet(inputStyle);
    QLabel *l2 = new QLabel("Education Level:", this); l2->setStyleSheet(labelStyle);
    formLayout->addRow(l2, educationCombo);

    // 3. Manifesto
    manifestoInput = new QTextEdit(this);
    manifestoInput->setStyleSheet(inputStyle);
    manifestoInput->setFixedHeight(100);
    manifestoInput->setPlaceholderText("Write your election manifesto and promises here...");
    QLabel *lManifesto = new QLabel("Manifesto:", this);
    lManifesto->setStyleSheet(labelStyle);
    formLayout->addRow(lManifesto, manifestoInput);

    // 4. Profile Image Upload
    QHBoxLayout *profileUploadLayout = new QHBoxLayout();
    uploadProfileBtn = new QPushButton("Browse Image...", this);
    uploadProfileBtn->setCursor(Qt::PointingHandCursor);
    uploadProfileBtn->setFixedSize(120, 35);
    profilePreview = new QLabel("No Image", this);
    profilePreview->setFixedSize(40, 40);
    profilePreview->setStyleSheet("border: 1px solid #BDC3C7; background: #ECF0F1; font-size: 9px;");
    profilePreview->setAlignment(Qt::AlignCenter);

    profileUploadLayout->addWidget(uploadProfileBtn);
    profileUploadLayout->addWidget(profilePreview);
    profileUploadLayout->addStretch();
    QLabel *l3 = new QLabel("Profile Photo:", this); l3->setStyleSheet(labelStyle);
    formLayout->addRow(l3, profileUploadLayout);

    // ==========================================
    // NEW: 5. Symbol Name Input
    // ==========================================
    symbolNameInput = new QLineEdit(this);
    symbolNameInput->setStyleSheet(inputStyle);
    symbolNameInput->setPlaceholderText("e.g. Eagle, Tiger, Scale");
    QLabel *lSymbol = new QLabel("Symbol Name:", this);
    lSymbol->setStyleSheet(labelStyle);
    formLayout->addRow(lSymbol, symbolNameInput);

    // 6. Symbol Image Upload
    QHBoxLayout *symbolUploadLayout = new QHBoxLayout();
    uploadSymbolBtn = new QPushButton("Browse Image...", this);
    uploadSymbolBtn->setCursor(Qt::PointingHandCursor);
    uploadSymbolBtn->setFixedSize(120, 35);
    symbolPreview = new QLabel("No Image", this);
    symbolPreview->setFixedSize(40, 40);
    symbolPreview->setStyleSheet("border: 1px solid #BDC3C7; background: #ECF0F1; font-size: 9px;");
    symbolPreview->setAlignment(Qt::AlignCenter);

    symbolUploadLayout->addWidget(uploadSymbolBtn);
    symbolUploadLayout->addWidget(symbolPreview);
    symbolUploadLayout->addStretch();
    QLabel *l4 = new QLabel("Party Symbol:", this); l4->setStyleSheet(labelStyle);
    formLayout->addRow(l4, symbolUploadLayout);

    cardLayout->addLayout(formLayout);
    cardLayout->addStretch();

    // 7. Submit Button
    submitBtn = new QPushButton("Submit Application", this);
    submitBtn->setCursor(Qt::PointingHandCursor);
    submitBtn->setFixedHeight(45);
    submitBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; margin-top: 15px; }"
                             "QPushButton:hover { background-color: #219653; }");
    cardLayout->addWidget(submitBtn);
    cardLayout->addStretch();

    rightLayout->addWidget(formCard);
    rightLayout->addStretch(); // Pushes form to the top

    rightStackedWidget->addWidget(formContainer);

    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightStackedWidget);

    // Connections
    connect(electionListView, &QListView::clicked, this, &UserCandidacyPage::onElectionClicked);
    connect(uploadProfileBtn, &QPushButton::clicked, this, &UserCandidacyPage::onUploadProfileClicked);
    connect(uploadSymbolBtn, &QPushButton::clicked, this, &UserCandidacyPage::onUploadSymbolClicked);
    connect(submitBtn, &QPushButton::clicked, this, &UserCandidacyPage::onSubmitClicked);
}

// ---------------------------------------------------------
// LOGIC
// ---------------------------------------------------------
void UserCandidacyPage::loadPublishedElections(Election* elections, int size) {
    electionModel->setElections(elections, size);
    rightStackedWidget->setCurrentIndex(0);
}

void UserCandidacyPage::clearData() {
    electionModel->setElections(nullptr, 0);
    rightStackedWidget->setCurrentIndex(0);
    resetForm();
}

void UserCandidacyPage::onElectionClicked(const QModelIndex &index) {
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();
    formTitleLabel->setText("Apply for: " + selected.getTitle());

    resetForm();
    rightStackedWidget->setCurrentIndex(1);
}

void UserCandidacyPage::resetForm() {
    partyNameInput->clear();
    educationCombo->setCurrentIndex(0);
    manifestoInput->clear();
    symbolNameInput->clear(); // <-- CLEAR NEW BOX
    base64ProfileStr.clear();
    base64SymbolStr.clear();
    profilePreview->clear(); profilePreview->setText("No Image");
    symbolPreview->clear(); symbolPreview->setText("No Image");
}

QString UserCandidacyPage::pickAndConvertImage(QLabel *previewLabel) {
    QString filePath = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg)");
    if(filePath.isEmpty()) return "";

    QPixmap pixmap(filePath);
    if(pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Could not load image.");
        return "";
    }

    previewLabel->setPixmap(pixmap.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");
    return QString(byteArray.toBase64());
}

void UserCandidacyPage::onUploadProfileClicked() {
    base64ProfileStr = pickAndConvertImage(profilePreview);
}

void UserCandidacyPage::onUploadSymbolClicked() {
    base64SymbolStr = pickAndConvertImage(symbolPreview);
}

void UserCandidacyPage::onSubmitClicked() {
    // 1. Validation! (Added check for Symbol Name!)
    if(partyNameInput->text().trimmed().isEmpty() ||
        educationCombo->currentIndex() == 0 ||
        manifestoInput->toPlainText().trimmed().isEmpty() ||
        symbolNameInput->text().trimmed().isEmpty() || // <-- CHECK NEW BOX
        base64ProfileStr.isEmpty() ||
        base64SymbolStr.isEmpty())
    {
        QMessageBox::warning(this, "Missing Information", "Please fill in all fields and upload both images before submitting.");
        return;
    }

    // 2. Build the Candidate Object
    Candidate newCandidate;
    newCandidate.setElectionId(currentSelectedElectionId);
    newCandidate.setPartyName(partyNameInput->text().trimmed());
    newCandidate.setEducationLevel(educationCombo->currentText());
    newCandidate.setManifesto(manifestoInput->toPlainText().trimmed());

    // Grab the actual symbol name they typed instead of the old hack!
    newCandidate.setSymbolName(symbolNameInput->text().trimmed());

    newCandidate.setProfileImageBase64(base64ProfileStr);
    newCandidate.setSymbolBase64(base64SymbolStr);
    newCandidate.setStatus(ApprovalStatus::Pending);

    // 3. Emit to MainWindow
    emit submitApplicationRequested(newCandidate);
}

