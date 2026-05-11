#ifndef RESULTREPOSITORY_H
#define RESULTREPOSITORY_H

#include "models/entities/result.h"
#include <mongocxx/collection.hpp>
#include <optional>

class ResultRepository : public IPollResultRepository
{
private:
    mongocxx::collection m_collection;

public:
    ResultRepository();
    ~ResultRepository() override = default;

    bool insertPollResult(const Result &result) override;
    std::optional<Result> getResultByElection(const QString &electionId) override;
};

#endif // RESULTREPOSITORY_H