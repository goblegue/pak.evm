#ifndef RESULT_CONTROLLER_H
#define RESULT_CONTROLLER_H

#include "models/entities/result.h"
#include "models/repositories/electionrepository.h"
// Assuming IPollResultRepository is defined in its own header or inside poll_result.h
// #include "models/repositories/pollresultrepository.h" 

#include <QString>
#include <QByteArray>
#include <optional>

class IPollResultRepository; // Forward declaration

class ResultController {
private:
    IPollResultRepository* m_resultRepo;
    IElectionRepository* m_electionRepo; // We need this to update the election to "ResultsAnnounced"
    
    ResultController();
    ~ResultController();

public:
    ResultController(const ResultController&) = delete;
    void operator=(const ResultController&) = delete;

    static ResultController& getInstance();
    void injectRepositories(IPollResultRepository* resultRepo, IElectionRepository* electionRepo);

    // STEP 1: Load from USB, Decrypt, and Preview (Does NOT save to DB yet)
    std::optional<Result> loadAndPreviewResultFile(const QString &filePath,
                                                   const QByteArray &publicKey,
                                                   const QByteArray &privateKey);

    // STEP 2: Officially save the previewed result to MongoDB and close the election
    bool commitFinalResults(const Result &result);

    // Fetch existing results for viewing past elections
    std::optional<Result> getElectionResults(const QString &electionId);
};

#endif // RESULT_CONTROLLER_H