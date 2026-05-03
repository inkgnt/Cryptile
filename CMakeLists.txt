cmake_minimum_required(VERSION 3.20)
project(CryptPass VERSION 0.1 LANGUAGES CXX C)

# ==========================================================
# Compiler setup
# ==========================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)

# ==========================================================
# Find packages
# ==========================================================
find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(OpenSSL REQUIRED)

# ==========================================================
# SQLCipher Library
# ==========================================================
add_subdirectory(src/sqlcipher)

# ==========================================================
# Sources
# ==========================================================
set(CORE_SOURCES
    src/crypto/crypto.cpp
    src/keymanager/keymanager.cpp
    src/storage/database_manager.cpp

)

set(UI_SOURCES
    ui/root/root_view.cpp

    ui/vault/vault_view.cpp
    ui/setup/setup_view.cpp
    ui/unlock/unlock_view.cpp

    ui/common/password_entry_dialog.cpp
    ui/vault/password_entry_widget.cpp

    ui/common/dialog.cpp
)

# ==========================================================
# Target
# ==========================================================

qt_add_executable(CryptPass
    src/main.cpp

    ${CORE_SOURCES}
    ${UI_SOURCES}

    resources/resource.qrc

)

target_compile_definitions(CryptPass PRIVATE SQLITE_HAS_CODEC=1)
target_include_directories(CryptPass PRIVATE

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/src/sqlcipher
)

# ==========================================================
# Linking
# ==========================================================
target_link_libraries(CryptPass PRIVATE
    Qt6::Widgets
    sqlcipher
)

target_link_libraries(CryptPass PUBLIC
    OpenSSL::Crypto
)


# ==========================================================
# Deployment
# ==========================================================
if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::windeployqt IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET CryptPass POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:CryptPass>"
    )

    set_target_properties(CryptPass PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(UNIX AND NOT APPLE)

endif()
