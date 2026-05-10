#ifndef USERCANDIDACYPAGE_H
#define USERCANDIDACYPAGE_H

#include <QWidget>
#include <QListView>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QFileDialog>
#include <QTextEdit>
#include "models/entities/election.h"
#include "models/entities/candidate.h"
#include "models/Models.h"

class UserCandidacyPage : public QWidget {
    Q_OBJECT

public:
    explicit UserCandidacyPage(QWidget *parent = nullptr);
    void loadPublishedElections(Election* elections, int size);

signals:
    
    void submitApplicationRequested(Candidate newCandidate);

private slots:
    void onElectionClicked(const QModelIndex &index);
    void onUploadProfileClicked();
    void onUploadSymbolClicked();
    void onSubmitClicked();

private:
    QListView *electionListView;
    ElectionListModel *electionModel;

    QStackedWidget *rightStackedWidget;
    QWidget *placeholderWidget;
    QWidget *formContainer;

    QLabel *formTitleLabel;
    QLineEdit *partyNameInput;
    QComboBox *educationCombo;
    QTextEdit *manifestoInput;

    
    QPushButton *uploadProfileBtn;
    QLabel *profilePreview;
    QString base64ProfileStr;

    QPushButton *uploadSymbolBtn;
    QLabel *symbolPreview;
    QString base64SymbolStr;

    QPushButton *submitBtn;

    QString currentSelectedElectionId;

    void setupUi();

    
    QString pickAndConvertImage(QLabel *previewLabel);
    void resetForm();
};

#endif 