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

    
    this->setWindowTitle("Secure Voting Token");
    this->setFixedSize(400, 200); 

    
    ui->detailsLabel->setText(QString("<b>Election ID:</b> %1<br>Please save this QR Code to scan at the physical voting booth.").arg(electionId));

    ui->electionidLabel->setText(QString("<b>Election ID:</b> %1<br>").arg(electionId));

    
    loadBase64Image(base64Image);

    
    connect(ui->okButton, &QPushButton::clicked, this, &QDialog::accept);
}

VoterTokenMess::~VoterTokenMess()
{
    delete ui;
}

void VoterTokenMess::loadBase64Image(const QString &base64String)
{
    
    QString cleanBase64 = base64String;
    if (cleanBase64.contains(",")) {
        cleanBase64 = cleanBase64.split(",").last();
    }

    QByteArray imageData = QByteArray::fromBase64(cleanBase64.toUtf8());

    
    QPixmap pixmap;
    if (pixmap.loadFromData(imageData)) {
        
        ui->qrImageLabel->setPixmap(pixmap.scaled(ui->qrImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        
        ui->qrImageLabel->setText("Error: Could not load token image.");
        ui->qrImageLabel->setAlignment(Qt::AlignCenter);
    }
}
