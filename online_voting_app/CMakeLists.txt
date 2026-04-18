

cmake_minimum_required(VERSION 3.19)
project(online_voting_app LANGUAGES CXX)

# 1. Added Sql and Network to the required Qt components
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets Network)
find_package(PkgConfig REQUIRED)
# Automatically handles C++17/C++20, AUTOMOC, AUTORCC, and AUTOUIC
qt_standard_project_setup()

# 2. Define the executable and ALL source files
qt_add_executable(online_voting_app
    WIN32 MACOSX_BUNDLE

    # --- CORE UI FILES ---
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    databasemanager.h
    databasemanager.cpp

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

    src/AuthManager/auth_manager.h src/AuthManager/auth_manager.cpp
    src/QR/qr.h src/QR/qr.cpp

    src/states.h
    src/user.h
    src/admin.h
    src/election.h
    src/candidate.h
    src/voters.h
    src/voting_station.h
    src/otp.h

)

# 3. Tell CMake where to look for the SMTP headers (Modern Target-based approach)
target_include_directories(online_voting_app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/smtp_lib)

# 4. Define the macro to fix SMTP DLL export issues
target_compile_definitions(online_voting_app PRIVATE SMTP_MIME_LIBRARY)

pkg_check_modules(MONGOCXX REQUIRED libmongocxx)
pkg_check_modules(BSONCXX REQUIRED libbsoncxx)

target_include_directories(online_voting_app PRIVATE
${MONGOCXX_INCLUDE_DIRS}
${BSONCXX_INCLUDE_DIRS}
)

# 5. Link all necessary libraries (Qt Modules + libsodium)
target_link_libraries(online_voting_app
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        ${MONGOCXX_LIBRARIES}
        ${BSONCXX_LIBRARIES}
        sodium
)

# =====================================================================
# FUTURE PLACEHOLDERS (For Backend & Database Integration)
# Your Database dev will uncomment and use these when ready.
# =====================================================================

# Attempt to find the driver via pkg-config if the package isn't found
# find_package(mongocxx REQUIRED)
#target_link_libraries(online_voting_app PRIVATE mongo::mongocxx_shared)


# =====================================================================
# AUTOMATED DEPLOYMENT SETUP (Do not touch)
# This packages your app so it runs on PCs that don't have Qt installed.
# =====================================================================
install(TARGETS online_voting_app
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

#qt_generate_deploy_app_script(
   # TARGET online_voting_app
    #OUTPUT_SCRIPT deploy_script
    #NO_UNSUPPORTED_PLATFORM_ERROR)
#install(SCRIPT ${deploy_script})
# Attempt to find the driver via pkg-config


# We use 'QUIET' so it doesn't crash the whole project if the driver isn't installed yet



