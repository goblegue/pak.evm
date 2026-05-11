#include "models/repositories/resultrepository.h"
#include "models/databasemanager.h"
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

ResultRepository::ResultRepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Results"];
}

bool ResultRepository::insertPollResult(const Result &result)
{
    try {
        auto builder = document{};

        // 1. Build the base fields
        builder << "election_id" << result.getElectionId().toStdString() << "poll_opened_at"
                << static_cast<int64_t>(result.getPollOpenedAt().toMSecsSinceEpoch())
                << "poll_closed_at"
                << static_cast<int64_t>(result.getPollClosedAt().toMSecsSinceEpoch())
                << "total_votes_cast" << result.getTotalVotesCast() << "audit_status"
                << result.getAuditStatus().toStdString() << "final_ledger_hash"
                << result.getFinalLedgerHash().toStdString();

        // 2. Build the BSON Array from our dynamic CandidateTally array
        auto array_builder = bsoncxx::builder::stream::array{};
        CandidateTally *tallyList = result.getTally();
        int count = result.getTallyCount();

        for (int i = 0; i < count; ++i) {
            array_builder << open_document << "candidate_cnic"
                          << tallyList[i].candidateCnic.toStdString() << "total_votes"
                          << tallyList[i].totalVotes << close_document;
        }

        // 3. Attach the array to the main document and save
        builder << "tally" << array_builder;

        m_collection.insert_one(builder << finalize);
        return true;

    } catch (...) {
        return false; // Will trigger if election_id is not unique
    }
}

std::optional<Result> ResultRepository::getResultByElection(const QString &electionId)
{
    auto filter = document{} << "election_id" << electionId.toStdString() << finalize;
    auto doc_opt = m_collection.find_one(filter.view());

    if (!doc_opt) {
        return std::nullopt;
    }

    auto view = doc_opt->view();
    Result result;

    // 1. Map simple fields
    result.setElectionId(QString::fromUtf8(view["election_id"].get_string().value.data()));
    result.setPollOpenedAt(QDateTime::fromMSecsSinceEpoch(view["poll_opened_at"].get_int64().value));
    result.setPollClosedAt(QDateTime::fromMSecsSinceEpoch(view["poll_closed_at"].get_int64().value));
    result.setTotalVotesCast(view["total_votes_cast"].get_int32().value);
    result.setAuditStatus(QString::fromUtf8(view["audit_status"].get_string().value.data()));
    result.setFinalLedgerHash(
        QString::fromUtf8(view["final_ledger_hash"].get_string().value.data()));

    // 2. Map the BSON Array back to our dynamic array
    auto tally_element = view["tally"];
    if (tally_element && tally_element.type() == bsoncxx::type::k_array) {
        bsoncxx::array::view array_view = tally_element.get_array().value;

        // Count how many items are in the array
        int arraySize = std::distance(array_view.begin(), array_view.end());

        if (arraySize > 0) {
            CandidateTally *tempTally = new CandidateTally[arraySize];
            int i = 0;

            for (auto &&element : array_view) {
                auto cand_doc = element.get_document().view();
                tempTally[i].candidateCnic = QString::fromUtf8(
                    cand_doc["candidate_cnic"].get_string().value.data());
                tempTally[i].totalVotes = cand_doc["total_votes"].get_int32().value;
                i++;
            }

            // Pass the array to the model (which uses our Rule of Three safety)
            result.setTallyData(tempTally, arraySize);

            // Clean up the temporary local buffer
            delete[] tempTally;
        }
    }

    return result;
}