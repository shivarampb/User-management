#include "logindialog.h"
#include "mainwindow.h"
#include "databasemanager.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("User Management System"));

    // -----------------------------------------------------------------
    //  Connect to MariaDB.  EDIT these values for your environment.
    //  MariaDB uses Qt's "QMYSQL" driver.
    // -----------------------------------------------------------------
    DatabaseManager &db = DatabaseManager::instance();
    if (!db.openDatabase(/* host */ QStringLiteral("127.0.0.1"),
                         /* db   */ QStringLiteral("user_mgmt"),
                         /* user */ QStringLiteral("root"),
                         /* pass */ QStringLiteral("your_db_password"),
                         /* port */ 3306)) {
        QMessageBox::critical(nullptr, QObject::tr("Database Error"),
                              QObject::tr("Could not connect to MariaDB:\n%1")
                                  .arg(db.lastError()));
        return 1;
    }

    // Login -> Main window -> (optional) back to Login on logout.
    while (true) {
        LoginDialog login;
        if (login.exec() != QDialog::Accepted)
            break; // login cancelled -> exit application

        MainWindow w(login.loggedInUser(), login.privilege());
        w.show();
        app.exec();

        if (!w.property("logout").toBool())
            break; // window closed (not a logout) -> exit
        // otherwise loop back to the login screen
    }

    return 0;
}
