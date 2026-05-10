#include <QJsonObject>
#include <QJsonDocument>
#include <QPainter>

#include "services/QR/qr.h"
#include "models/entities/voters.h"

#include "../../../qr_lib/qrcodegen.hpp"

using namespace qrcodegen;

QString QR::preparePayload(const Token &voter)
{
    
    QJsonObject payloadObj;
    payloadObj["cnic"] = voter.getUserCnic();
    payloadObj["electionId"] = voter.getElectionId();
    payloadObj["issuedAt"] = voter.getIssuedAt().toString(Qt::ISODate);
    payloadObj["signature"] = voter.getTokenSignature();
    QJsonDocument doc(payloadObj);
    return doc.toJson(QJsonDocument::Indented);
}

QImage QR::generateQRCode(const QString &payload)
{
    QrCode qr = QrCode::encodeText(payload.toStdString().c_str(), QrCode::Ecc::MEDIUM);

    int scale = 10;
    int border = 4;

    int size = qr.getSize();
    int imgSize = (size + border * 2) * scale;

    QImage image(imgSize, imgSize, QImage::Format_RGB32);
    image.fill(Qt::white);

    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);

    for (int x = 0; x < size; x++)
    {
        for (int y = 0; y < size; y++)
        {
            if (qr.getModule(x, y))
            {
                int rectX = (x + border) * scale;
                int rectY = (y + border) * scale;

                painter.drawRect(rectX, rectY, scale, scale);
            }
        }
    }
    return image;
}
