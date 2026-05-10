#ifndef QR_H
#define QR_H

#include <QImage>
#include <QString>

class Token;
class QR
{
public:
    static QString preparePayload(const Token &voter);

    static QImage generateQRCode(const QString &payload);
};
#endif 