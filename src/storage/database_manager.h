#pragma once

#include "sqlite3.h"
#include "data_record.h"

#include <QObject>
#include <QString>
#include <QList>
#include <QByteArray>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    bool databaseFileExists(const QString &dbPath) const;

    bool openDatabase(const QString &dbPath, const QByteArray &password);

    void closeDatabase();
    bool createTables();

    bool addRecord(const QString &url,
                   const QByteArray &login,
                   const QByteArray &encryptedPassword);

    bool deleteRecord(int id);
    QList<DataRecord> getAllRecords() const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    sqlite3* m_db = nullptr;
    QString  m_currentDbPath;
};