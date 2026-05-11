#include "views/UserResultsPage.h"
#include <QFrame>
#include "Delegates.h"
#include "controllers/candidateController.h"
#include "controllers/resultController.h"

UserResultsPage::UserResultsPage(QWidget *parent) : QWidget(parent) {
    electionModel = new ElectionListModel(this);
    setupUi();
}

void UserResultsPage::setupUi() {
    this->setStyleSheet("background-color: #f5f7fb;");
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(20);
    
    // ==========================================
    // LEFT SIDE: ELECTION LIST (30%)
    // ==========================================
    QWidget *leftPanel = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *electionLabel = new QLabel("<b>Completed Elections</b>", this);
    electionLabel->setStyleSheet("font-size: 22px; color: #2C3E50;"); 
    
    electionListView = new QListView(this);
    electionListView->setModel(electionModel);
    electionListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    electionListView->setSpacing(10);
    electionListView->setItemDelegate(new ElectionDelegate(this)); 
    electionListView->setStyleSheet("QListView { border: none; background: transparent; outline: none; }");
    
    // FIX: Hide the ugly horizontal scrollbar!
    electionListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
    
    leftLayout->addWidget(electionLabel);
    leftLayout->addWidget(electionListView);
    
    // ==========================================
    // RIGHT SIDE: STACKED WIDGET (70%)
    // ==========================================
    rightStackedWidget = new QStackedWidget(this);

    // --- PAGE 0: PLACEHOLDER ---
    placeholderWidget = new QWidget(this);
    QVBoxLayout *phLayout = new QVBoxLayout(placeholderWidget);
    QLabel *phLabel = new QLabel("👈 Select a completed election to view official results.", this);
    phLabel->setAlignment(Qt::AlignCenter);
    phLabel->setStyleSheet("font-size: 18px; color: #7F8C8D; font-style: italic;");
    phLayout->addWidget(phLabel);
    rightStackedWidget->addWidget(placeholderWidget); // Index 0

    // --- PAGE 1: ACTUAL RESULTS ---
    resultsContainer = new QWidget(this);
    resultsContainer->setStyleSheet("QWidget#resultsContainer { background-color: white; border: 2px solid #BDC3C7; border-radius: 10px; }");
    resultsContainer->setObjectName("resultsContainer");
    QVBoxLayout *rightLayout = new QVBoxLayout(resultsContainer);
    rightLayout->setContentsMargins(40, 40, 40, 40); 
    rightLayout->setSpacing(25); 
    
    // Header
    rightTitleLabel = new QLabel("Election Results", this);
    rightTitleLabel->setStyleSheet("font-size: 28px; font-weight: bold; color: #2980B9; border: none; border-bottom: 2px solid #ECF0F1; padding-bottom: 10px;");
    rightTitleLabel->setWordWrap(true);
    
    // Chart Area
    QLabel *graphTitle = new QLabel("<b>Final Voting Tally</b>", this);
    graphTitle->setStyleSheet("font-size: 22px; color: #34495E; border: none;");
    
    chartScrollArea = new QScrollArea(this);
    chartScrollArea->setWidgetResizable(true);
    chartScrollArea->setMinimumHeight(350); 
    chartScrollArea->setStyleSheet("QScrollArea { border: 2px solid #ECF0F1; border-radius: 8px; background-color: #FDFEFE; }");
    
    chartWidget = new QWidget();
    chartWidget->setStyleSheet("background-color: transparent; border: none;");
    chartLayout = new QVBoxLayout(chartWidget);
    chartLayout->setAlignment(Qt::AlignTop);
    chartLayout->setSpacing(15); 
    chartScrollArea->setWidget(chartWidget);
    
    // Stats Area
    QFrame *statsFrame = new QFrame(this);
    statsFrame->setStyleSheet("QFrame { background-color: #F8F9F9; border: 2px solid #3498DB; border-radius: 8px; }");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsFrame);
    statsLayout->setContentsMargins(20, 20, 20, 20);
    statsLayout->setSpacing(15);
    
    QString statStyle = "font-size: 18px; color: #2C3E50; font-weight: bold; border: none;";
    winnerLabel = new QLabel("🏆 Winner: -", this);
    totalTokensLabel = new QLabel("🎫 Total Tokens Scanned: -", this);
    pollClosedLabel = new QLabel("⏱ Polling Closed At: -", this);
    
    winnerLabel->setStyleSheet("font-size: 26px; color: #27AE60; font-weight: 900; border: none;"); 
    totalTokensLabel->setStyleSheet(statStyle);
    pollClosedLabel->setStyleSheet(statStyle);
    
    statsLayout->addWidget(winnerLabel);
    statsLayout->addWidget(totalTokensLabel);
    statsLayout->addWidget(pollClosedLabel);
    
    rightLayout->addWidget(rightTitleLabel);
    rightLayout->addWidget(graphTitle);
    rightLayout->addWidget(chartScrollArea, 1); 
    rightLayout->addWidget(statsFrame);

    rightStackedWidget->addWidget(resultsContainer); // Index 1
    
    // ==========================================
    // ASSEMBLE MAIN LAYOUT (STRICT 30 / 70 SPLIT)
    // ==========================================
    mainLayout->addWidget(leftPanel, 3);
    mainLayout->addWidget(rightStackedWidget, 7);
    
    connect(electionListView, &QListView::clicked, this, &UserResultsPage::onElectionClicked);
}

void UserResultsPage::loadElections(Election *elections, int size)
{
    electionModel->setElections(elections, size);
    rightStackedWidget->setCurrentIndex(0); // Show placeholder initially!
}

void UserResultsPage::onElectionClicked(const QModelIndex &index)
{
    Election selected = electionModel->getElectionAt(index.row());
    currentSelectedElectionId = selected.getId();

    rightTitleLabel->setText("Official Results: " + selected.getTitle());
    clearResults();

    // Fetch the Official Results directly from the Database!
    auto resultOpt = ResultController::getInstance().getElectionResults(currentSelectedElectionId);

    if (resultOpt.has_value()) {
        displayResults(resultOpt.value());
        rightStackedWidget->setCurrentIndex(1); // Show the results container!
    } else {
        QMessageBox::warning(this,
                             "Not Found",
                             "Official results for this election have not been published yet.");
        rightStackedWidget->setCurrentIndex(0); // Revert to placeholder
    }

    emit electionSelectedForResults(currentSelectedElectionId);
}

// THE CRASH-FREE CLEAR RESULTS LOGIC
void UserResultsPage::clearResults()
{
    if (!chartLayout)
        return; // Safety check

    QLayoutItem *child;
    while ((child = chartLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            child->widget()->hide();
            child->widget()->deleteLater(); // Safe Qt Deletion!
        }
        delete child;
    }

    if (winnerLabel)
        winnerLabel->setText("🏆 Winner: -");
    if (totalTokensLabel)
        totalTokensLabel->setText("🎫 Total Votes Cast: -");
    if (pollClosedLabel)
        pollClosedLabel->setText("⏱ Polling Closed At: -");
}

void UserResultsPage::displayResults(const Result &res)
{
    clearResults();

    int totalVotes = res.getTotalVotesCast();
    totalTokensLabel->setText(QString("🎫 Total Votes Cast: %1").arg(totalVotes));

    pollClosedLabel->setText("⏱ Polling Closed At: "
                             + res.getPollClosedAt().toString("MMM dd, yyyy - hh:mm AP"));

    int maxVotes = -1;
    QString winnerName = "Tie / Undecided";

    CandidateTally *tally = res.getTally();
    int tallyCount = res.getTallyCount();

    for (int i = 0; i < tallyCount; ++i) {
        int votes = tally[i].totalVotes;
        QString cnic = tally[i].candidateCnic;

        // Fetch Real Name from Database
        QString candName = getCandidateName(cnic, res.getElectionId());

        if (votes > maxVotes) {
            maxVotes = votes;
            winnerName = candName;
        }

        double percentage = totalVotes > 0 ? ((double) votes / totalVotes) * 100.0 : 0;

        // --- Create Thick Graph Row ---
        QWidget *rowWidget = new QWidget();
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 5, 0, 5);

        QLabel *nameLbl = new QLabel(candName, rowWidget);
        nameLbl->setFixedWidth(180);
        nameLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #34495E; border: none;");

        QProgressBar *bar = new QProgressBar(rowWidget);
        bar->setRange(0, totalVotes);
        bar->setValue(votes);
        bar->setTextVisible(false);
        bar->setFixedHeight(40);
        bar->setStyleSheet(
            "QProgressBar { border: 1px solid #BDC3C7; border-radius: 6px; background: #ECF0F1; }"
            "QProgressBar::chunk { background-color: #2980B9; border-radius: 5px; }");

        QLabel *pctLbl = new QLabel(QString::number(percentage, 'f', 1) + "% ("
                                        + QString::number(votes) + " Votes)",
                                    rowWidget);
        pctLbl->setFixedWidth(150);
        pctLbl->setStyleSheet("font-size: 16px; font-weight: bold; color: #7F8C8D; border: none;");

        rowLayout->addWidget(nameLbl);
        rowLayout->addWidget(bar);
        rowLayout->addWidget(pctLbl);

        chartLayout->addWidget(rowWidget);
    }

    winnerLabel->setText("🏆 WINNER: " + winnerName);
}

// Fetches the Real Name from the Backend Controller
QString UserResultsPage::getCandidateName(const QString &cnic, const QString &electionId)
{
    int size = 0;
    Candidate *candidates = CandidateController::getInstance()
                                .getCandidatesByElectionAndStatus(electionId,
                                                                  ApprovalStatus::Approved,
                                                                  size);

    QString name = cnic; // Fallback
    if (candidates) {
        for (int i = 0; i < size; ++i) {
            if (candidates[i].getUserCnic() == cnic) {
                name = candidates[i].getName();
                if (name.isEmpty())
                    name = candidates[i].getPartyName(); // Secondary fallback
                break;
            }
        }
        delete[] candidates; // Memory safety!
    }
    return name;
}