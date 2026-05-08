#include "data_manager.h"
#include <QDebug>

/*db_id(primary key, plain) | created_at(plain) | updated_at(plain) | last_access_at(plain) |
 * | object_id(plain, random, 32 byte) | type (plain) | ns_salt(plain) | s_salt(plain) |
 * | iv + aes256gcm_Knsd(NCEK) | iv + aes256gcm_Ksd(SCEK) | iv + aes256gcm_NCEK(nonsecure data) | iv + aes256gcm_SCEK(secure data)
 *
 * iv(nonce) - random, always stored in db before encrypted object
 * object_id (plain) - random, generates on writing DataRecord to db
 * NCEK - random, generates on writing DataRecord to db, encrypted using Knsd
 * SCEK - random, generates on writing DataRecord to db, encrypted using Ksd
 * nonsecure data - TLV array, encrypted using NCEK
 * secure data - TLV array, encrypted using SCEK
 *
 */
/*
bool DataManager::createTables() noexcept
{
    if (!m_db) {
        qWarning() << "Database isn't loaded!";
        return false;
    }

    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS DataRecords (
            db_id INTEGER PRIMARY KEY AUTOINCREMENT,
            created_at INTEGER NOT NULL DEFAULT (unixepoch()),
            updated_at INTEGER NOT NULL DEFAULT (unixepoch()),
            last_access_at INTEGER NOT NULL DEFAULT (unixepoch()),

            object_id BLOB NOT NULL,
            type INTEGER NOT NULL,

            ns_salt BLOB NOT NULL,
            s_salt BLOB NOT NULL,

            NCEK BLOB NOT NULL,
            SCEK BLOB NOT NULL,

            nonsecure_data BLOB NOT NULL,
            secure_data BLOB NOT NULL
        );

        CREATE TRIGGER IF NOT EXISTS trg_DataRecords_updated_at
            AFTER UPDATE ON DataRecords
        BEGIN
            UPDATE DataRecords SET updated_at = unixepoch() WHERE db_id = NEW.db_id;
        END;
    )";

    char* errMsg = nullptr;
    auto rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        qWarning() << "Failed to create tables:" << errMsg;
        sqlite3_free(errMsg);
        return false;
    }

    qDebug() << "Tables created successfully.";
    return true;
}

bool DataManager::createRecord(DbRecord& db_record) noexcept
{
    const char* sql = "INSERT INTO DataRecords (object_id, type, ns_salt, s_salt, NCEK, SCEK, nonsecure_data, secure_data) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Prepare failed:" << sqlite3_errmsg(m_db);
        return false;
    }

    sqlite3_bind_blob(stmt, 1, db_record.object_id.data(), db_record.object_id.size(), SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<uint8_t>(db_record.type));
    sqlite3_bind_blob(stmt, 3,  db_record.ns_salt.data(), db_record.ns_salt.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 4,  db_record.s_salt.data(), db_record.s_salt.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 5,  db_record.encrypted_ncek.data(), db_record.encrypted_ncek.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 6,  db_record.encrypted_scek.data(), db_record.encrypted_scek.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 7,  db_record.encrypted_nonsecure.data(), db_record.encrypted_nonsecure.size(), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 8,  db_record.encrypted_secure.data(), db_record.encrypted_secure.size(), SQLITE_STATIC);

    auto rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to add record:" << sqlite3_errmsg(m_db);
        return false;
    }

    db_record.db_id = sqlite3_last_insert_rowid(m_db);

    qDebug() << "Record added successfully! Record db_id: " << db_record.db_id;
    return true;
}

bool DataManager::updateRecord() noexcept
{
    //TODO
    return true;
}

bool DataManager::deleteRecord(uint64_t db_id) noexcept
{
    const char* sql = "DELETE FROM DataRecords WHERE db_id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Prepare failed:" << sqlite3_errmsg(m_db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, db_id);

    auto rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to delete a record:" << sqlite3_errmsg(m_db);
        return false;
    }

    qDebug() << "Record deleted successfully! Record db_id " << db_id;
    return true;
}

bool DataManager::getAllRecords() noexcept
{
    const char* sql = "SELECT id, url, login, password FROM passwords;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Prepare failed:" << sqlite3_errmsg(m_db);
        return records;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        DataRecord rec;
        rec.id       = sqlite3_column_int(stmt, 0);
        rec.url      = QString::fromUtf8((const char*)sqlite3_column_text(stmt, 1));
        rec.login    = QByteArray(reinterpret_cast<const char*>(sqlite3_column_blob(stmt, 2)),
                               sqlite3_column_bytes(stmt, 2));
        rec.password = QByteArray(reinterpret_cast<const char*>(sqlite3_column_blob(stmt, 3)),
                                  sqlite3_column_bytes(stmt, 3));

        records.append(rec);
    }

    sqlite3_finalize(stmt);
    qDebug() << "Total records retrieved:" << records.size();
    return records;
}

*/