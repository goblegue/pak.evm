cmake_minimum_required(VERSION 3.16)
project(OfflineVotingKiosk VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt Automation: Handles .ui files and Signal/Slot MOC automatically
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 1. Find Qt6 Components
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets Sql Network Multimedia MultimediaWidgets)
# 2. Use PkgConfig to find Libsodium and SQLCipher
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
pkg_check_modules(SQLCIPHER REQUIRED IMPORTED_TARGET sqlcipher)

# 3. Define Source Files (Add new files here as you create them)
set(PROJECT_SOURCES
    src/main.cpp

    # Models
    src/models/entities/tokens.h
    src/models/entities/voting_record.h
    src/models/entities/system_config.h
    src/models/entities/poll_worker.h
    src/models/entities/candidates.h
    src/models/entities/audit_log.h


    # Views (UI)
    src/views/mainwindow.h
    src/views/mainwindow.cpp
    src/views/mainwindow.ui

    # Controllers
    src/controllers/auth_manager.h
    src/controllers/auth_manager.cpp
    src/controllers/election_controller.h
    src/controllers/election_controller.cpp


    # Services
    src/services/crypto/crypto_engine.h
    src/services/crypto/crypto_engine.cpp

)

# 4. Create the Executable
add_executable(OfflineVotingKiosk ${PROJECT_SOURCES}
    src/views/OfflineSetupWizard.h src/views/OfflineSetupWizard.cpp
    src/views/EvmScanPage.h src/views/EvmScanPage.cpp
    src/Resource.qrc
    src/views/PreElectionPage.h src/views/PreElectionPage.cpp
    src/views/PostElectionPage.h src/views/PostElectionPage.cpp
    src/views/Models.h
    src/views/Delegates.h
    src/views/EvmVotingPage.h src/views/EvmVotingPage.cpp)

# 5. Link Libraries
target_link_libraries(OfflineVotingKiosk PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    Qt::Network
    Qt::Multimedia
    Qt::MultimediaWidgets
    PkgConfig::SODIUM      # Links Libsodium (Argon2, Ed25519)
    PkgConfig::SQLCIPHER   # Links SQLCipher (Encrypted SQLite)
)

# 6. Include Paths (So you can #include "models/Candidate.h" easily)
target_include_directories(OfflineVotingKiosk PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
