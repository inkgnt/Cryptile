#pragma once

#include "database_manager.h"

#include <QDebug>
#include <QFile>

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::databaseFileExists(const QString& dbPath) const
{
    return QFile::exists(dbPath);
}

bool DatabaseManager::openDatabase(const QString& dbPath, const QByteArray& key)
{
    if (key.isEmpty()) {
        qWarning() << "Key failure, refusing to open database.";
        closeDatabase();
        return false;
    }

    closeDatabase();

    int rc = sqlite3_open_v2(dbPath.toUtf8().constData(), &m_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);

    if (rc != SQLITE_OK) {
        qWarning() << "Cannot open database:" << sqlite3_errmsg(m_db);
        closeDatabase();
        return false;
    }

    rc = sqlite3_key(m_db, key.constData(), key.size());
    if (rc != SQLITE_OK) {
        qWarning() << "sqlite3_key failed:" << sqlite3_errmsg(m_db);
        closeDatabase();
        return false;
    }


    qDebug() << "Database opened successfully with SQLCipher:" << dbPath;
    return createTables();
}

void DatabaseManager::closeDatabase()
{
    if (m_db) {
        sqlite3_close_v2(m_db);
        m_db = nullptr;
    }
    qDebug() << "Database connection closed";
}


//todo delete
bool DatabaseManager::createTables()
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS passwords (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            url TEXT NOT NULL,
            login BLOB NOT NULL,
            password BLOB NOT NULL
        );
    )";

    char* errMsg = nullptr;
    int rc = sqlite3_exec(m_db, sql, nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK) {
        qWarning() << "Failed to create tables:" << errMsg;
        sqlite3_free(errMsg);
        return false;
    }

    qDebug() << "Tables created successfully.";
    return true;
}

bool DatabaseManager::addRecord(const QString &url, const QByteArray &login, const QByteArray &encryptedPassword)
{
    const char* sql = "INSERT INTO passwords (url, login, password) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Prepare failed:" << sqlite3_errmsg(m_db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, url.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 2, login.constData(), login.size(), SQLITE_TRANSIENT);
    sqlite3_bind_blob(stmt, 3, encryptedPassword.constData(), encryptedPassword.size(), SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to add record:" << sqlite3_errmsg(m_db);
        return false;
    }

    qDebug() << "Record added successfully:" << url;
    return true;
}

bool DatabaseManager::deleteRecord(int id)
{
    const char* sql = "DELETE FROM passwords WHERE id = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        qWarning() << "Prepare failed:" << sqlite3_errmsg(m_db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to delete record:" << sqlite3_errmsg(m_db);
        return false;
    }

    qDebug() << "Record deleted successfully:" << id;
    return true;
}

QList<DataRecord> DatabaseManager::getAllRecords() const
{
    QList<DataRecord> records;
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