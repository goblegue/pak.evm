#ifndef VOTERTOKENMESS_H
#define VOTERTOKENMESS_H

#include <QDialog>

namespace Ui {
class VoterTokenMess;
}

class VoterTokenMess : public QDialog
{
    Q_OBJECT

public:
    explicit VoterTokenMess(const QString &base64Image, const QString &electionId,const QString &tokenId, QWidget *parent = nullptr);
    ~VoterTokenMess();

private:
    Ui::VoterTokenMess *ui;
    void loadBase64Image(const QString &base64String);
};

#endif // VOTERTOKENMESS_H
