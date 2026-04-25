cmake_minimum_required(VERSION 3.19)
project(online_voting_app LANGUAGES CXX)

# 1. Require Qt and PkgConfig
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets Network)
find_package(PkgConfig REQUIRED)

# Automatically handles C++17/C++20, AUTOMOC, AUTORCC, and AUTOUIC
set(CMAKE_AUTORCC ON)
qt_standard_project_setup()

# 2. Define the executable and ALL source files
qt_add_executable(online_voting_app
    WIN32 MACOSX_BUNDLE

    # --- CORE UI FILES ---
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    resources.qrc

    # --- SMTP LIBRARY FILES ---
    smtp_lib/emailaddress.cpp
    smtp_lib/emailaddress.h
    smtp_lib/mimeattachment.cpp
    smtp_lib/mimeattachment.h
    smtp_lib/mimebytearrayattachment.cpp
    smtp_lib/mimebytearrayattachment.h
    smtp_lib/mimefile.cpp
    smtp_lib/mimefile.h
    smtp_lib/mimehtml.cpp
    smtp_lib/mimehtml.h
    smtp_lib/mimeinlinefile.cpp
    smtp_lib/mimeinlinefile.h
    smtp_lib/mimemessage.cpp
    smtp_lib/mimemessage.h
    smtp_lib/mimepart.cpp
    smtp_lib/mimepart.h
    smtp_lib/mimetext.cpp
    smtp_lib/mimetext.h
    smtp_lib/smtpclient.cpp
    smtp_lib/smtpclient.h
    smtp_lib/quotedprintable.cpp
    smtp_lib/quotedprintable.h
    smtp_lib/mimemultipart.cpp
    smtp_lib/mimemultipart.h
    smtp_lib/mimecontentencoder.cpp
    smtp_lib/mimecontentencoder.h
    smtp_lib/mimebase64encoder.cpp
    smtp_lib/mimebase64encoder.h
    smtp_lib/mimeqpencoder.cpp
    smtp_lib/mimeqpencoder.h
    smtp_lib/mimeqpformatter.cpp
    smtp_lib/mimeqpformatter.h
    smtp_lib/mimebase64formatter.cpp
    smtp_lib/mimebase64formatter.h
    smtp_lib/mimecontentformatter.cpp
    smtp_lib/mimecontentformatter.h
    smtp_lib/smtpmime_global.h
    smtp_lib/SmtpMime

    qr_lib/qrcodegen.cpp
    qr_lib/qrcodegen.hpp

    src/email/emailservice.h src/email/emailservice.cpp
    src/crypto/cryptoengine.h src/crypto/cryptoengine.cpp
    src/user_management/AuthManager/auth_manager.h src/user_management/AuthManager/auth_manager.cpp
    src/QR/qr.h src/QR/qr.cpp
    src/states.h
    src/user_management/user/user.h
    src/user_management/admin/admin.h
    src/election/election.h src/election/electionController.h src/election/electionController.cpp
    src/candidate/candidate.h src/candidate/candidateController.h src/candidate/candidateController.cpp
    src/voters/voters.h
    src/voting_station/voting_station.h
    src/otp/otp.h
    system_bootstrapper.h system_bootstrapper.cpp

    src/databasemanager.h
    src/databasemanager.cpp

    src/userrepository.cpp
     src/userrepository.h
     src/adminrepository.cpp
     src/adminrepository.h
     src/electionrepository.cpp
     src/electionrepository.h
     src/candidaterepository.cpp
     src/candidaterepository.h
     src/otprepository.cpp
     src/otprepository.h
     src/voterrepository.cpp
     src/voterrepository.h
     src/votingstationrepository.cpp
     src/votingstationrepository.h
     src/Models.h
     src/AdminCandidatePage.h
     src/AdminCandidatePage.cpp
     src/admindelegates.h




     src/electiondelegate.h
     src/electionmodel.h

)

# 3. Tell CMake where to look for the SMTP headers
target_include_directories(online_voting_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/smtp_lib)

# 4. Define the macro to fix SMTP DLL export issues
target_compile_definitions(online_voting_app PRIVATE SMTP_MIME_LIBRARY)

# ---------------------------------------------------------------------
# THE ARCHITECT'S CROSS-PLATFORM DATABASE FIX
# This dynamically supports both Arch Linux (v2.x) and Ubuntu/WSL (v1.x)
# ---------------------------------------------------------------------

# 1. Find the C++ Wrappers (Standard across all OS)
pkg_check_modules(MONGOCXX REQUIRED IMPORTED_TARGET libmongocxx)
pkg_check_modules(BSONCXX REQUIRED IMPORTED_TARGET libbsoncxx)

# 2. OS-Detection Logic: Look for Arch's v2 driver first
find_library(ARCH_MONGOC_V2 NAMES mongoc2)

if(ARCH_MONGOC_V2)
    # If found, we are on Arch Linux
    message(STATUS "🐘 Architect: Detected Arch Linux (MongoDB v2.x)")
    set(OS_SPECIFIC_MONGO_C_DRIVERS mongoc2 bson2)
else()
    # If NOT found, we default to Ubuntu/WSL
    message(STATUS "🐧 Architect: Detected Ubuntu/WSL (MongoDB v1.x)")
    set(OS_SPECIFIC_MONGO_C_DRIVERS mongoc-1.0 bson-1.0)
endif()

# 3. Link all targets dynamically
target_link_libraries(online_voting_app
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        PkgConfig::MONGOCXX
        PkgConfig::BSONCXX
        ${OS_SPECIFIC_MONGO_C_DRIVERS}  # <-- This variable holds the correct OS files!
        sodium
)

# =====================================================================
# AUTOMATED DEPLOYMENT SETUP
# =====================================================================
install(TARGETS online_voting_app
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
