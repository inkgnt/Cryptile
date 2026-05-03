
add_library(sqlcipher STATIC sqlite3.c)

set_source_files_properties(sqlite3.c PROPERTIES LANGUAGE C)

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