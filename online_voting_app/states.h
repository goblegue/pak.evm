#ifndef STATES_H
#define STATES_H

enum class ElectionState { Draft, Published, VotingOpen, VotingClosed, ResultsAnnounced };

enum class ApprovalStatus { Pending, Approved, Rejected };

#endif // STATES_H
