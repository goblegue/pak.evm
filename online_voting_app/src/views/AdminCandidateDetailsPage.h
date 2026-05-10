#ifndef ADMINCANDIDATEDETAILSPAGE_H
#define ADMINCANDIDATEDETAILSPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFormLayout>
#include "models/entities/candidate.h"

class AdminCandidateDetailsPage : public QWidget {
    Q_OBJECT

public:
    explicit AdminCandidateDetailsPage(QWidget *parent = nullptr);
    void setUserMode(bool isUserMode);

    
    void setCandidate(const Candidate &candidate);

signals:
    
    void backBtnClicked();
    void candidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus);

private slots:
    void onApproveClicked();
    void onRejectClicked();

private:
    QPushButton *backBtn;
    QLabel *headerTitleLabel;

    
    QLabel *profilePicLabel;
    QLabel *symbolPicLabel;

    
    QLabel *statusLabel;
    QLabel *cnicLabel;
    QLabel *electionIdLabel;
    QLabel *partyNameLabel;
    QLabel *symbolNameLabel;
    QLabel *educationLabel;

    
    QLabel *historyLabel;
    QLabel *manifestoLabel;

    
    QPushButton *approveBtn;
    QPushButton *rejectBtn;
    QLabel *approvalCountLabel; 
    QString currentCandidateCnic; 

    void setupUi();

    
    QPixmap decodeBase64Image(const QString &base64Str, int expectedSize);
    bool m_isUserMode = false;
};

#endif 
