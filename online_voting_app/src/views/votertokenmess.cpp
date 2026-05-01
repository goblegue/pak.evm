#include "votertokenmess.h"
#include "ui_votertokenmess.h"
#include <QByteArray>
#include <QPixmap>
#include <QMessageBox>

VoterTokenMess::VoterTokenMess(const QString &base64Image, const QString &electionId, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VoterTokenMess)
{
    ui->setupUi(this);

    // 1. Setup the window properties
    this->setWindowTitle("Secure Voting Token");
    this->setFixedSize(400, 200); // Lock the window size

    // 2. Set the text details
    ui->detailsLabel->setText(QString("<b>Election ID:</b> %1<br>Please save this QR Code to scan at the physical voting booth.").arg(electionId));

    ui->electionidLabel->setText(QString("<b>Election ID:</b> %1<br>").arg(electionId));

    // 3. Load the Base64 Image
    loadBase64Image(base64Image);

    // 4. Connect the OK button to close the dialog
    connect(ui->okButton, &QPushButton::clicked, this, &QDialog::accept);
}

VoterTokenMess::~VoterTokenMess()
{
    delete ui;
}

void VoterTokenMess::loadBase64Image(const QString &base64String)
{
    // Step A: Convert the QString Base64 to a QByteArray
    // We strip out any "data:image/png;base64," header if your backend sent it
    QString cleanBase64 = base64String;
    if (cleanBase64.contains(",")) {
        cleanBase64 = cleanBase64.split(",").last();
    }

    QByteArray imageData = QByteArray::fromBase64(cleanBase64.toUtf8());

    // Step B: Load the raw bytes into a QPixmap
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        // Success! Set it to the label
        // We scale it smoothly to fit the label size we defined in Qt Designer
        ui->qrImageLabel->setPixmap(pixmap.scaled(ui->qrImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        // Fallback if the Base64 string is corrupted or invalid
        ui->qrImageLabel->setText("Error: Could not load token image.");
        ui->qrImageLabel->setAlignment(Qt::AlignCenter);
    }
}
