cmake_minimum_required(VERSION 3.16)
project(CryptPass VERSION 0.1 LANGUAGES CXX)

# ==========================================================
# Compiler setup
# ==========================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS
    ${CMAKE_SOURCE_DIR}/ui
)

# ==========================================================
# Find packages
# ==========================================================
find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)

find_package(OpenSSL REQUIRED)

# ==========================================================
# Sources
# ==========================================================
set(SOURCES_LIST
    src/main.cpp
    src/crypto.cpp
    src/keymanager.cpp
    src/mainapp.cpp
    src/registerwidget.cpp
    src/loginwidget.cpp
    src/mainwindowwidget.cpp
    src/databasemanager.cpp
    src/dialog.cpp
    src/passwordform.cpp
    src/passwordcardwidget.cpp
)


set(HEADERS_LIST
    src/crypto.h
    src/keymanager.h
    src/mainapp.h
    src/registerwidget.h
    src/loginwidget.h
    src/mainwindowwidget.h
    src/databasemanager.h
    src/dialog.h
    src/passwordform.h
    src/passwordcardwidget.h
)

set(UI_LIST
    ui/mainapp.ui
    ui/registerwidget.ui
    ui/loginwidget.ui
    ui/mainwindowwidget.ui
    ui/dialog.ui
    ui/passwordform.ui
    ui/passwordcardwidget.ui
)

set(RESOURCES_LIST
    resources/resource.qrc
)

# ==========================================================
# Target
# ==========================================================
qt_add_executable(CryptPass
    MANUAL_FINALIZATION
    ${SOURCES_LIST}
    ${HEADERS_LIST}
    ${UI_LIST}
    ${RESOURCES_LIST}
)

target_link_libraries(CryptPass PRIVATE
    Qt6::Widgets
    Qt6::Sql

    OpenSSL::Crypto
)

qt_finalize_executable(CryptPass)

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

    # ==========================================================
    # SQLCipher plugin
    # ==========================================================
    include(ExternalProject)

    set(SQLCIPHER_INSTALL_DIR "${CMAKE_BINARY_DIR}/sqlcipher-plugin/install")

    ExternalProject_Add(sqlcipher_plugin
        SOURCE_DIR "${CMAKE_SOURCE_DIR}/sqlcipher-plugin"
        BINARY_DIR "${CMAKE_BINARY_DIR}/sqlcipher-plugin"
        INSTALL_DIR "${SQLCIPHER_INSTALL_DIR}"
        CMAKE_ARGS
            -DCMAKE_BUILD_TYPE=$<CONFIG>
            -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
            -DQt6_DIR=${Qt6_DIR}
            -DCMAKE_PREFIX_PATH=${Qt6_DIR}/..
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DVCPKG_TARGET_TRIPLET=${VCPKG_TARGET_TRIPLET}
        BUILD_BYPRODUCTS
            "${SQLCIPHER_INSTALL_DIR}/plugins/sqldrivers/qsqlite.dll"
            "${SQLCIPHER_INSTALL_DIR}/plugins/sqldrivers/qsqlited.dll"
    )

    add_dependencies(CryptPass sqlcipher_plugin)

    add_custom_command(TARGET CryptPass POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SQLCIPHER_INSTALL_DIR}/plugins/sqldrivers/$<IF:$<CONFIG:Debug>,qsqlited,qsqlite>.dll"
            "$<TARGET_FILE_DIR:CryptPass>/sqldrivers/$<IF:$<CONFIG:Debug>,qsqlited,qsqlite>.dll"

        COMMENT "Deploying SQLCipher plugin dll"
    )

    set_target_properties(CryptPass PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(UNIX AND NOT APPLE)

endif()
