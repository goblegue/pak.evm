#ifndef CONFIGREPO_H
#define CONFIGREPO_H

#include "../models/entities/system_config.h"
#include <optional>

class ConfigRepository : public IConfigRepository {
public:
    bool saveConfig(const SystemConfig &config) override;
    std::optional<SystemConfig> getConfig() override;
};

#endif
