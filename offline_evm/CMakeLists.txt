cmake_minimum_required(VERSION 3.16)
project(OfflineVotingKiosk VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Automation
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 1. Find Qt6 Components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# 2. Libsodium (Keep this for the Crypto Lead and memory wiping)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# 3. Define Source Files
set(PROJECT_SOURCES
    src/main.cpp

    # --- YOUR DATABASE FILES ---
    src/model/DatabaseManager.h
    src/model/DatabaseManager.cpp
    src/model/candidaterepo.h
    src/model/candidaterepo.cpp

    # --- LEAD'S INTERFACES (Entities) ---
    src/models/entities/tokens.h
    src/models/entities/voting_record.h
    src/models/entities/system_config.h
    src/models/entities/poll_worker.h
    src/models/entities/candidates.h
    src/models/entities/audit_log.h

    # --- LEAD'S SERVICES ---
    src/services/crypto/crypto_engine.h
    src/services/crypto/crypto_engine.cpp

    # --- VIEWS (UI) ---
    src/views/mainwindow.h
    src/views/mainwindow.cpp
    src/views/mainwindow.ui
)

# 4. Create the Executable
add_executable(OfflineVotingKiosk ${PROJECT_SOURCES}
    src/model/voterepository.h src/model/voterepository.cpp
    src/model/auditlogrepository.h src/model/auditlogrepository.cpp
    src/model/workerrepository.h src/model/workerrepository.cpp
    src/model/configrepo.h src/model/configrepo.cpp
    src/model/tokenrepo.h src/model/tokenrepo.cpp)

# 5. Link Libraries
target_link_libraries(OfflineVotingKiosk PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    PkgConfig::SODIUM
)
