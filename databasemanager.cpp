#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QVariant>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager s_instance;
    return s_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

bool DatabaseManager::openDatabase(const QString &host,
                                   const QString &dbName,
                                   const QString &user,
                                   const QString &password,
                                   int port)
{
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection))
        m_db = QSqlDatabase::database(QSqlDatabase::defaultConnection);
    else
        m_db = QSqlDatabase::addDatabase("QMYSQL"); // MariaDB uses the MySQL driver

    m_db.setHostName(host);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);
    m_db.setPort(port);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    m_lastError.clear();
    return true;
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

// --------------------------------------------------------------------------
//  Authentication
// --------------------------------------------------------------------------
bool DatabaseManager::authenticate(const QString &username, const QString &password, int &privilegeOut)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT Privilege FROM userdetails WHERE Username = :u AND Password = :p");
    q.bindValue(":u", username);
    q.bindValue(":p", password);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    if (q.next()) {
        privilegeOut = q.value(0).toInt();
        return true;
    }
    m_lastError = QStringLiteral("Invalid username or password.");
    return false;
}

bool DatabaseManager::verifyPassword(const QString &username, const QString &password)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM userdetails WHERE Username = :u AND Password = :p");
    q.bindValue(":u", username);
    q.bindValue(":p", password);
    if (!q.exec() || !q.next()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return q.value(0).toInt() > 0;
}

// --------------------------------------------------------------------------
//  Queries
// --------------------------------------------------------------------------
bool DatabaseManager::userExists(const QString &username)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT COUNT(*) FROM userdetails WHERE Username = :u");
    q.bindValue(":u", username);
    if (!q.exec() || !q.next())
        return false;
    return q.value(0).toInt() > 0;
}

bool DatabaseManager::getUser(const QString &username, UserRecord &out)
{
    QSqlQuery q(m_db);
    q.prepare("SELECT Username, Privilege FROM userdetails WHERE Username = :u");
    q.bindValue(":u", username);
    if (!q.exec() || !q.next()) {
        m_lastError = q.lastError().text();
        return false;
    }
    out.username  = q.value(0).toString();
    out.privilege = q.value(1).toInt();
    return true;
}

QVector<UserRecord> DatabaseManager::usersByPrivilege(const QList<int> &levels)
{
    QVector<UserRecord> result;
    if (levels.isEmpty())
        return result;

    QStringList placeholders;
    for (int i = 0; i < levels.size(); ++i)
        placeholders << "?";

    const QString sql =
        "SELECT Username, Privilege FROM userdetails "
        "WHERE Privilege IN (" + placeholders.join(", ") + ") "
        "ORDER BY Privilege ASC, Username ASC";

    QSqlQuery q(m_db);
    q.prepare(sql);
    for (int level : levels)
        q.addBindValue(level);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return result;
    }
    while (q.next()) {
        UserRecord r;
        r.username  = q.value(0).toString();
        r.privilege = q.value(1).toInt();
        result.push_back(r);
    }
    return result;
}

// --------------------------------------------------------------------------
//  Mutations
// --------------------------------------------------------------------------
bool DatabaseManager::addUser(const QString &username, const QString &password, int privilege)
{
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO userdetails (Username, Password, Privilege) VALUES (:u, :p, :pr)");
    q.bindValue(":u", username);
    q.bindValue(":p", password);
    q.bindValue(":pr", privilege);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateUser(const QString &originalUsername,
                                 const QString &newUsername,
                                 int newPrivilege)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE userdetails SET Username = :nu, Privilege = :pr WHERE Username = :ou");
    q.bindValue(":nu", newUsername);
    q.bindValue(":pr", newPrivilege);
    q.bindValue(":ou", originalUsername);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteUser(const QString &username)
{
    QSqlQuery q(m_db);
    q.prepare("DELETE FROM userdetails WHERE Username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::changePassword(const QString &username, const QString &newPassword)
{
    QSqlQuery q(m_db);
    q.prepare("UPDATE userdetails SET Password = :p WHERE Username = :u");
    q.bindValue(":p", newPassword);
    q.bindValue(":u", username);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}
