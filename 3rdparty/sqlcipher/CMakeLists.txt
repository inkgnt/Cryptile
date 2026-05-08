
add_library(sqlcipher STATIC sqlite3.c)
add_library(sqlcipher_interface INTERFACE)

target_compile_definitions(sqlcipher_interface INTERFACE SQLITE_HAS_CODEC=1)

target_include_directories(sqlcipher_interface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(sqlcipher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(sqlcipher PRIVATE
    HAVE_STDINT_H

    SQLITE_HAS_CODEC=1
    SQLITE_TEMP_STORE=3
    SQLITE_THREADSAFE=1

    SQLITE_EXTRA_INIT=sqlcipher_extra_init
    SQLITE_EXTRA_SHUTDOWN=sqlcipher_extra_shutdown

    SQLITE_OMIT_DEPRECATED

    HAVE_OPENSSL
)

target_link_libraries(sqlcipher PRIVATE OpenSSL::Crypto)

set_target_properties(sqlcipher PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

set_source_files_properties(sqlite3.c PROPERTIES LANGUAGE C)