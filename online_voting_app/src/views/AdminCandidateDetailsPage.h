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

    // Call this right before showing the page to populate all the text/images
    void setCandidate(const Candidate &candidate);

signals:
    // Tells the MainWindow to go back to the Candidates List
    void backBtnClicked();
    void candidateStatusChangeRequested(QString targetCnic, ApprovalStatus newStatus);

private slots:
    void onApproveClicked();
    void onRejectClicked();

private:
    QPushButton *backBtn;
    QLabel *headerTitleLabel;

    // Images
    QLabel *profilePicLabel;
    QLabel *symbolPicLabel;

    // Details
    QLabel *statusLabel;
    QLabel *cnicLabel;
    QLabel *electionIdLabel;
    QLabel *partyNameLabel;
    QLabel *symbolNameLabel;
    QLabel *educationLabel;

    // Long Text
    QLabel *historyLabel;
    QLabel *manifestoLabel;

    // to approve or reject candidate
    QPushButton *approveBtn;
    QPushButton *rejectBtn;
    QLabel *approvalCountLabel; // To show "1 Admin has approved"
    QString currentCandidateCnic; // To remember who we are looking at

    void setupUi();

    // Helper function to turn Base64 strings back into Qt Images
    QPixmap decodeBase64Image(const QString &base64Str, int expectedSize);
    bool m_isUserMode = false;
};

#endif // ADMINCANDIDATEDETAILSPAGE_H
