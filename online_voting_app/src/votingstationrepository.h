#ifndef VOTINGSTATIONREPOSITORY_H
#define VOTINGSTATIONREPOSITORY_H

#include "./voting_station/voting_station.h"
#include "databasemanager.h"

class votingstationrepository : public IVotingStationRepository
{
private:
    mongocxx::collection m_collection;

public:
    votingstationrepository();
    ~votingstationrepository() override = default;

    bool insertStation(const VotingStation &station) override;
    VotingStation *getAllActiveStations(int &vSSize) override;
    VotingStation *getStationsByCity(const QString &city, int &vSSize) override;
};
#endif // VOTINGSTATIONREPOSITORY_H
