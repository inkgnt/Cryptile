#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QList>
#include <QByteArray>

struct PasswordRecord {
    int id;
    QString url;
    QByteArray login;
    QByteArray password;
};

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager& instance();

    bool databaseFileExists(const QString &dbPath) const;

    bool createTables();

    bool openDatabase(const QString &dbPath);
    void closeDatabase();

    bool addRecord(const QString &url, const QByteArray &login, const QByteArray &encryptedPassword);
    bool deleteRecord(int id);

    QList<PasswordRecord> getAllRecords() const;

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    QSqlDatabase m_database;
};
