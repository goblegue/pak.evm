#ifndef STATES_H
#define STATES_H
#include <QString>
enum class ElectionState { Draft, Published, VotingOpen, VotingClosed, ResultsAnnounced };

enum class ApprovalStatus { Pending, Approved, Rejected };

struct StatusChangeRequest
{
    ApprovalStatus status;
    QString requestById;
};

#endif // STATES_H
