#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

//TODO https://www.zetetic.net/sqlcipher/sqlcipher-api/

DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager::~DatabaseManager() {}

bool DatabaseManager::openDatabase(const QString &dbPath)
{
    closeDatabase();

    qDebug() << "Available SQL drivers:" << QSqlDatabase::drivers();

    m_database = QSqlDatabase::addDatabase("QSQLITECIPHER");
    qDebug() << "Attempting to use driver:" << m_database.driverName();

    m_database.setDatabaseName(dbPath);

    if (!m_database.open()) {
        qDebug() << "Failed to open database!";
        qDebug() << "Database error:" << m_database.lastError().text();
        return false;
    }

    QSqlQuery query(m_database);
    if (!query.exec("PRAGMA key = 'mysecretpassword';")) {
        qDebug() << "Failed to set key:" << query.lastError().text();
        return false;
    }

    //crash reason lol some virtual alloc problems
    /*
    if (!query.exec("PRAGMA cipher_memory_security = ON")) {
        qDebug() << "Failed to set cipher_memory_security:" << query.lastError().text();
        return false;
    }
    */

    qDebug() << "Database opened successfully:" << dbPath;
    return true;
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen())
        m_database.close();

    QString cname = m_database.connectionName();
    m_database = QSqlDatabase();

    if (QSqlDatabase::contains(cname))
        QSqlDatabase::removeDatabase(cname);

    qDebug() << "Database connection closed:" << cname;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS passwords ("
                               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                               "url TEXT NOT NULL, "
                               "login BLOB NOT NULL, "
                               "password BLOB NOT NULL"
                               ")";

    query.exec("PRAGMA key = 'mysecretpassword';");//TODO
    if (!query.exec(createTableQuery))
    {
        qDebug() << "Failed to create tables:" << query.lastError().text();
        return false;
    }

    qDebug() << "Tables created successfully.";
    return true;
}

bool DatabaseManager::addRecord(const QString &url, const QByteArray &login, const QByteArray &encryptedPassword)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO passwords (url, login, password) VALUES (:url, :login, :password)");
    query.bindValue(":url", url);
    query.bindValue(":login", login);
    query.bindValue(":password", encryptedPassword);

    if (!query.exec()) {
        qDebug() << "Failed to add record:" << query.lastError().text();
        return false;
    }

    qDebug() << "Record added successfully:" << url;
    return true;
}

bool DatabaseManager::deleteRecord(int id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM passwords WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        qDebug() << "Failed to delete record:" << query.lastError().text();
        return false;
    }

    qDebug() << "Record deleted successfully:" << id;
    return true;
}

QList<PasswordRecord> DatabaseManager::getAllRecords() const
{
    QList<PasswordRecord> records;
    QSqlQuery query(m_database);
    if (!query.exec("SELECT id, url, login, password FROM passwords")) {
        qDebug() << "Failed to retrieve records:" << query.lastError().text();
        return {};
    }

    while (query.next())
    {
        PasswordRecord rec;
        rec.id = query.value(0).toInt();
        rec.url = query.value(1).toString();
        rec.login = query.value(2).toByteArray();
        rec.password = query.value(3).toByteArray();
        records.append(rec);
    }

    qDebug() << "Total records retrieved:" << records.size();
    return records;
}
