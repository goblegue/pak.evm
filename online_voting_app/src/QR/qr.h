#ifndef QR_H
#define QR_H

#include <QImage>
#include <QString>


class Voters;
class QR
{
public:
    static QString preparePayload(const Voters &voter);
    
    static QImage generateQRCode(const QString &payload);
    
};
#endif // QR_H