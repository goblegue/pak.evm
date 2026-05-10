#include "UserCandidacyPage.h"
#include "Delegates.h"
#include <QBuffer>
#include <QMessageBox>
#include <QFormLayout>
#include <QFrame>

UserCandidacyPage::UserCandidacyPage(QWidget *parent) : QWidget(parent)
{
    electionModel = new ElectionListModel(this);
    setupUi();
}

void UserCandidacyPage::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(25);

    // ==========================================
    // LEFT SIDE
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setMinimumWidth(200);
    leftPanel->setMaximumWidth(400);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *electionLabel = new QLabel("<b>Select Election to Run</b>", this);
    electionLabel->setStyleSheet("font-size: 20px; color: #2C3E50;");

    electionListView = new QListView(this);
    electionListView->setMouseTracking(true);
    electionListView->setModel(electionModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this));
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    electionListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);

    // ==========================================
    // RIGHT SIDE
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

    QString inputStyle = "QLineEdit, QComboBox { border: 2px solid #BDC3C7; border-radius: 6px; padding: 8px; font-size: 14px; color: #2C3E50; }"
                         "QLineEdit:focus, QComboBox:focus { border: 2px solid #3498DB; }"
                         "QTextEdit { border: 2px solid #BDC3C7; border-radius: 6px; padding: 8px; font-size: 14px; color: #2C3E50; }"
                         "QTextEdit:focus { border: 2px solid #3498DB; }";
    QString labelStyle = "font-size: 14px; color: #34495E; font-weight: bold;";

    // ==========================================
    // CREATE ALL WIDGETS FIRST
    // ==========================================
    partyNameInput = new QLineEdit(this);
    partyNameInput->setStyleSheet(inputStyle);

    educationCombo = new QComboBox(this);
    educationCombo->addItems({"Select Education...", "High School / Matric", "Intermediate",
                              "Bachelor's Degree", "Master's Degree", "Doctorate / Ph.D."});
    educationCombo->setStyleSheet(inputStyle);

    manifestoInput = new QTextEdit(this);
    manifestoInput->setStyleSheet(inputStyle);
    manifestoInput->setMinimumHeight(150);
    manifestoInput->setPlaceholderText("Write your election manifesto and promises here...");

    uploadProfileBtn = new QPushButton("Browse Image...", this);
    uploadProfileBtn->setCursor(Qt::PointingHandCursor);
    uploadProfileBtn->setFixedSize(150, 40);

    profilePreview = new QLabel("No Image", this);
    profilePreview->setFixedSize(80, 80);
    profilePreview->setStyleSheet("border: 1px solid #BDC3C7; background: #ECF0F1; font-size: 9px;");
    profilePreview->setAlignment(Qt::AlignCenter);

    uploadSymbolBtn = new QPushButton("Browse Image...", this);
    uploadSymbolBtn->setCursor(Qt::PointingHandCursor);
    uploadSymbolBtn->setFixedSize(150, 40);

    symbolPreview = new QLabel("No Image", this);
    symbolPreview->setFixedSize(80, 80);
    symbolPreview->setStyleSheet("border: 1px solid #BDC3C7; background: #ECF0F1; font-size: 9px;");
    symbolPreview->setAlignment(Qt::AlignCenter);

    submitBtn = new QPushButton("Submit Application", this);
    submitBtn->setCursor(Qt::PointingHandCursor);
    submitBtn->setFixedHeight(45);
    submitBtn->setStyleSheet("QPushButton { background-color: #27AE60; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; margin-top: 15px; }"
                             "QPushButton:hover { background-color: #219653; }");

    // ==========================================
    // NOW BUILD THE LAYOUT
    // ==========================================
    QGridLayout *formLayout = new QGridLayout();
    formLayout->setHorizontalSpacing(20);
    formLayout->setVerticalSpacing(35);
    formLayout->setColumnStretch(1, 1);

    // Row 0 — Party Name
    QLabel *l1 = new QLabel("Party Name:", this);
    l1->setStyleSheet(labelStyle);
    formLayout->addWidget(l1, 0, 0, Qt::AlignVCenter);
    formLayout->addWidget(partyNameInput, 0, 1);

    // Row 1 — Education
    QLabel *l2 = new QLabel("Education Level:", this);
    l2->setStyleSheet(labelStyle);
    formLayout->addWidget(l2, 1, 0, Qt::AlignVCenter);
    formLayout->addWidget(educationCombo, 1, 1);

    // Row 2 — Manifesto
    QLabel *lManifesto = new QLabel("Manifesto:", this);
    lManifesto->setStyleSheet(labelStyle);
    formLayout->addWidget(lManifesto, 2, 0, Qt::AlignTop);
    formLayout->addWidget(manifestoInput, 2, 1);

    // Row 3 — Profile Photo
    QLabel *l3 = new QLabel("Profile Photo:", this);
    l3->setStyleSheet(labelStyle);
    QHBoxLayout *profileUploadLayout = new QHBoxLayout();
    profileUploadLayout->addWidget(uploadProfileBtn);
    profileUploadLayout->addStretch();
    profileUploadLayout->addWidget(profilePreview);
    QWidget *profileUploadWidget = new QWidget(this);
    profileUploadWidget->setLayout(profileUploadLayout);
    formLayout->addWidget(l3, 3, 0, Qt::AlignVCenter);
    formLayout->addWidget(profileUploadWidget, 3, 1);

    // Row 4 — Party Symbol
    QLabel *l4 = new QLabel("Party Symbol:", this);
    l4->setStyleSheet(labelStyle);
    QHBoxLayout *symbolUploadLayout = new QHBoxLayout();
    symbolUploadLayout->addWidget(uploadSymbolBtn);
    symbolUploadLayout->addStretch();
    symbolUploadLayout->addWidget(symbolPreview);
    QWidget *symbolUploadWidget = new QWidget(this);
    symbolUploadWidget->setLayout(symbolUploadLayout);
    formLayout->addWidget(l4, 4, 0, Qt::AlignVCenter);
    formLayout->addWidget(symbolUploadWidget, 4, 1);

    cardLayout->addLayout(formLayout, 1);
    cardLayout->addStretch();
    cardLayout->addWidget(submitBtn);

    rightLayout->addWidget(formCard, 1);
    rightStackedWidget->addWidget(formContainer);

    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightStackedWidget, 2);

    // Connections
    connect(electionListView, &QListView::clicked, this, &UserCandidacyPage::onElectionClicked);
    connect(uploadProfileBtn, &QPushButton::clicked, this, &UserCandidacyPage::onUploadProfileClicked);
    connect(uploadSymbolBtn, &QPushButton::clicked, this, &UserCandidacyPage::onUploadSymbolClicked);
    connect(submitBtn, &QPushButton::clicked, this, &UserCandidacyPage::onSubmitClicked);
}

// ---------------------------------------------------------
// LOGIC
// ---------------------------------------------------------
void UserCandidacyPage::loadPublishedElections(Election *elections, int size)
{
    electionModel->setElections(elections, size);
    rightStackedWidget->setCurrentIndex(0);
}

void UserCandidacyPage::onElectionClicked(const QModelIndex &index)
{
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();
    formTitleLabel->setText("Apply for: " + selected.getTitle());

    resetForm();
    rightStackedWidget->setCurrentIndex(1);
}

void UserCandidacyPage::resetForm()
{
    partyNameInput->clear();
    educationCombo->setCurrentIndex(0);
    manifestoInput->clear();
    base64ProfileStr.clear();
    base64SymbolStr.clear();
    profilePreview->clear();
    profilePreview->setText("No Image");
    symbolPreview->clear();
    symbolPreview->setText("No Image");
}

// THE IMAGE CONVERTER
QString UserCandidacyPage::pickAndConvertImage(QLabel *previewLabel)
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.jpeg)");
    if (filePath.isEmpty())
        return ""; // User cancelled

    QPixmap pixmap(filePath);
    if (pixmap.isNull())
    {
        QMessageBox::warning(this, "Error", "Could not load image.");
        return "";
    }

    // Set tiny preview on the UI
    previewLabel->setPixmap(pixmap.scaled(previewLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // ==========================================
    // --- DATABASE IMAGE COMPRESSION ALGORITHM ---
    // ==========================================

    // 1. RESIZE: Scale the massive image down to a 256x256 thumbnail max.
    // (This alone reduces a 4K image down to a tiny fraction of its size)
    QPixmap compressedPixmap = pixmap.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 2. SETUP BUFFER
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    // 3. COMPRESS TO JPG:
    // Change "PNG" to "JPG" and add the '70' quality parameter (out of 100).
    // PNG ignores quality settings and stays huge. JPG compresses aggressively.
    compressedPixmap.save(&buffer, "JPG", 70);

    // Return the compressed string!
    return QString(byteArray.toBase64());
}

void UserCandidacyPage::onUploadProfileClicked()
{
    base64ProfileStr = pickAndConvertImage(profilePreview);
}

void UserCandidacyPage::onUploadSymbolClicked()
{
    base64SymbolStr = pickAndConvertImage(symbolPreview);
}

void UserCandidacyPage::onSubmitClicked()
{
    // 1. Validation!
    if (partyNameInput->text().trimmed().isEmpty() ||
        educationCombo->currentIndex() == 0 ||
        manifestoInput->toPlainText().trimmed().isEmpty() ||
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
    newCandidate.setProfileImageBase64(base64ProfileStr);
    newCandidate.setSymbolBase64(base64SymbolStr);
    newCandidate.setSymbolName(partyNameInput->text().trimmed() + " Symbol"); // Defaulting symbol name to Party Name
    newCandidate.setStatus(ApprovalStatus::Pending);

    // 3. Emit to MainWindow
    emit submitApplicationRequested(newCandidate);
}
