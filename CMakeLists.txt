cmake_minimum_required(VERSION 3.21)
project(Cryptile VERSION 0.1 LANGUAGES CXX C)

# ==========================================================
# Compiler setup
# ==========================================================
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# Find packages
# ==========================================================
find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(OpenSSL CONFIG REQUIRED)
find_package(libsodium CONFIG REQUIRED)
find_package(sqlcipher CONFIG REQUIRED)
find_package(libfido2 CONFIG REQUIRED)

# ==========================================================
# Sources
# ==========================================================
set(CORE_SOURCES
    src/crypto/crypto.cpp
    src/keymanager/keymanager.cpp
    src/storage/database_manager.cpp
    src/storage/data_manager.cpp
)

set(UI_SOURCES
    ui/root/root_view.cpp
    ui/vault/vault_view.cpp
    ui/setup/setup_view.cpp
    ui/unlock/unlock_view.cpp

    ui/common/data_entry_dialog.cpp
    ui/vault/data_entry_widget.cpp

    ui/common/dialog.cpp
)

# ==========================================================
# Target
# ==========================================================
add_executable(${PROJECT_NAME}
    src/main.cpp

    ${CORE_SOURCES}
    ${UI_SOURCES}

    resources/resource.qrc
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/ui
)

# ==========================================================
# Linking
# ==========================================================
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:QT_NO_DEBUG_OUTPUT>)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Widgets

    OpenSSL::Crypto
    libsodium::libsodium
    sqlcipher::sqlcipher
    libfido2::libfido2
)

# ==========================================================
# Deployment
# ==========================================================
if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::windeployqt IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    set(WINDEPLOYQT_EXECUTABLE "${QT_BIN_DIR}/windeployqt.exe")

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${WINDEPLOYQT_EXECUTABLE}" "$<TARGET_FILE:${PROJECT_NAME}>"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)


    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /GS
            /sdl
            /guard:cf
        )
        target_link_options(${PROJECT_NAME} PRIVATE
            /NXCOMPAT
            /GUARD:CF
            /CETCOMPAT
        )
    endif()


elseif(UNIX)

    target_compile_options(${PROJECT_NAME} PRIVATE
        -fstack-protector-strong
        $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=3>
    )

    if(NOT APPLE)
        target_link_options(${PROJECT_NAME} PRIVATE
            -Wl,-z,relro,-z,now
            -Wl,--no-execstack
        )
    endif()

endif()
