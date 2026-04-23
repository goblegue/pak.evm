#include "votingstationrepository.h"

#include <bsoncxx/builder/stream/document.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

votingstationrepository::votingstationrepository()
{
    m_collection = DatabaseManager::getInstance().getDatabase()["Voting_Stations"];
}

bool votingstationrepository::insertStation(const VotingStation &station)
{
    try
    {
        auto builder = document{}
                       << "id" << station.getId().toStdString()
                       << "stationName" << station.getStationName().toStdString()
                       << "city" << station.getCity().toStdString()
                       << "address" << station.getAddress().toStdString()
                       << "isActive" << station.isActive();

        m_collection.insert_one(builder << finalize);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

VotingStation *votingstationrepository::getAllActiveStations(int &vSSize)
{
    auto filter = document{} << "isActive" << true << finalize;

    vSSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (vSSize == 0)
        return nullptr;

    VotingStation *stations = new VotingStation[vSSize];
    auto cursor = m_collection.find(filter.view());
    int i = 0;
    for (auto &&doc : cursor) {
        stations[i].setId(QString::fromUtf8(doc["id"].get_string().value.data()));
        stations[i].setStationName(QString::fromUtf8(doc["stationName"].get_string().value.data()));
        stations[i].setCity(QString::fromUtf8(doc["city"].get_string().value.data()));
        stations[i].setAddress(QString::fromUtf8(doc["address"].get_string().value.data()));
        stations[i].setIsActive(doc["isActive"].get_bool().value);
        i++;
    }
    return stations;
}

VotingStation *votingstationrepository::getStationsByCity(const QString &city, int &vSSize)
{
    auto filter = document{} << "city" << city.toStdString() << "isActive" << true << finalize;

    vSSize = static_cast<int>(m_collection.count_documents(filter.view()));
    if (vSSize == 0)
        return nullptr;

    VotingStation *stations = new VotingStation[vSSize];
    auto cursor = m_collection.find(filter.view());

    int i = 0;
    for (auto &&doc : cursor)
    {
        stations[i].setId(QString::fromUtf8(doc["id"].get_string().value.data()));
        stations[i].setStationName(QString::fromUtf8(doc["stationName"].get_string().value.data()));
        stations[i].setCity(QString::fromUtf8(doc["city"].get_string().value.data()));
        stations[i].setAddress(QString::fromUtf8(doc["address"].get_string().value.data()));
        stations[i].setIsActive(doc["isActive"].get_bool().value);
        i++;
    }
    return stations;
}
