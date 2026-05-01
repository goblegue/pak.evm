cmake_minimum_required(VERSION 3.16)
# Back to purely C++
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
    src/model/DatabaseManager.h
    src/model/DatabaseManager.cpp

    # Views (UI)
    src/views/mainwindow.h
    src/views/mainwindow.cpp
    src/views/mainwindow.ui
)

# 4. Create the Executable
add_executable(OfflineVotingKiosk ${PROJECT_SOURCES}
    src/model/candidaterepo.h src/model/candidaterepo.cpp)

# 5. Link Libraries (Removed raw C dependencies)
target_link_libraries(OfflineVotingKiosk PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    PkgConfig::SODIUM
)
