#ifndef WORKERREPO_H
#define WORKERREPO_H

#include "../models/entities/poll_worker.h"

class WorkerRepository : public IWorkerRepository {
public:
    bool insertWorker(const PollWorker &worker) override;
    std::optional<PollWorker> getWorkerByUsername(const QString &username) override;
    int getAdminCount() override;
};

#endif
