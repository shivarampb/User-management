#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>
#include <QVector>
#include <QList>

struct UserRecord {
    QString username;
    int     privilege = -1;
};

// ---------------------------------------------------------------------------
//  Thin wrapper around the MariaDB connection (QMYSQL driver).
//  All queries are parameterised to prevent SQL injection.
//  Table:  userdetails ( Username, Password, Privilege )
// ---------------------------------------------------------------------------
class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager &instance();

    bool openDatabase(const QString &host,
                      const QString &dbName,
                      const QString &user,
                      const QString &password,
                      int port = 3306);
    bool    isOpen() const;
    QString lastError() const { return m_lastError; }

    // --- Authentication ---
    bool authenticate(const QString &username, const QString &password, int &privilegeOut);
    bool verifyPassword(const QString &username, const QString &password);

    // --- Queries ---
    bool userExists(const QString &username);
    bool getUser(const QString &username, UserRecord &out);
    QVector<UserRecord> usersByPrivilege(const QList<int> &levels);

    // --- Mutations ---
    bool addUser(const QString &username, const QString &password, int privilege);
    bool updateUser(const QString &originalUsername,
                    const QString &newUsername,
                    int newPrivilege);
    bool deleteUser(const QString &username);
    bool changePassword(const QString &username, const QString &newPassword);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    Q_DISABLE_COPY(DatabaseManager)

    QSqlDatabase m_db;
    QString      m_lastError;
};

#endif // DATABASEMANAGER_H
