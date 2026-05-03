#include "qr.h"
#include "../voters/voters.h"

#include <QPainter>

#include "../../qr_lib/qrcodegen.hpp"

using namespace qrcodegen;

QString QR::preparePayload(const Voters &voter)
{
    // Simple payload preparation using a JSON-like format
    QString payload = QString("{\"id\":\"%1\",\"electionId\":\"%2\",\"issuedAt\":\"%3\",\"signature\":\"%4\"}")
                          .arg(voter.getId())
                          .arg(voter.getElectionId())
                          .arg(voter.getIssuedAt().toString(Qt::ISODate))
                          .arg(voter.getTokenSignature());
    return payload;
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
