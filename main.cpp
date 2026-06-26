#include "logindialog.h"
#include "mainwindow.h"
#include "databasemanager.h"

#include <QApplication>
#include <QMessageBox>

static const char *kAppStyle = R"QSS(
/* ── Global ─────────────────────────────────────────── */
QWidget {
    font-family: "Segoe UI", "Helvetica Neue", Arial, sans-serif;
    font-size: 13px;
    color: #202124;
}
QDialog  { background-color: #ffffff; }
QMainWindow > QWidget { background-color: #f0f2f5; }

/* ── Labels ──────────────────────────────────────────── */
QLabel { background: transparent; }

/* ── Inputs ──────────────────────────────────────────── */
QLineEdit {
    background-color: #ffffff;
    border: 1.5px solid #dadce0;
    border-radius: 6px;
    padding: 8px 12px;
    min-height: 20px;
    selection-background-color: #1a73e8;
    selection-color: #ffffff;
}
QLineEdit:focus  { border-color: #1a73e8; }
QLineEdit:hover  { border-color: #adb5bd; }

/* ── ComboBox ────────────────────────────────────────── */
QComboBox {
    background-color: #ffffff;
    border: 1.5px solid #dadce0;
    border-radius: 6px;
    padding: 7px 12px;
    min-height: 20px;
    min-width: 140px;
}
QComboBox:focus { border-color: #1a73e8; }
QComboBox::drop-down { border: none; width: 24px; }
QComboBox QAbstractItemView {
    background-color: #ffffff;
    border: 1px solid #dadce0;
    selection-background-color: #e8f0fe;
    selection-color: #1a73e8;
    outline: none;
    padding: 4px 0;
}

/* ── Buttons (default / secondary) ──────────────────── */
QPushButton {
    background-color: #ffffff;
    color: #202124;
    border: 1.5px solid #dadce0;
    border-radius: 6px;
    padding: 8px 20px;
    font-weight: 500;
    min-height: 20px;
}
QPushButton:hover   { background-color: #f8f9fa; border-color: #adb5bd; }
QPushButton:pressed { background-color: #e8eaed; }
QPushButton:disabled { color: #adb5bd; border-color: #e8eaed; }

/* ── Primary button ──────────────────────────────────── */
QPushButton#loginButton {
    background-color: #1a73e8;
    color: #ffffff;
    border: none;
    font-size: 14px;
    padding: 10px 20px;
}
QPushButton#loginButton:hover   { background-color: #1557b0; }
QPushButton#loginButton:pressed { background-color: #0d47a1; }

QPushButton#addButton {
    background-color: #1a73e8;
    color: #ffffff;
    border: none;
}
QPushButton#addButton:hover { background-color: #1557b0; }

/* ── Danger button ───────────────────────────────────── */
QPushButton#deleteButton {
    color: #d93025;
    border-color: #f5c6c4;
}
QPushButton#deleteButton:hover { background-color: #fce8e6; border-color: #d93025; }

/* ── Header logout button ────────────────────────────── */
QPushButton#logoutButton {
    background-color: rgba(255,255,255,0.15);
    color: #ffffff;
    border: 1px solid rgba(255,255,255,0.4);
    border-radius: 6px;
    padding: 6px 16px;
    font-size: 12px;
}
QPushButton#logoutButton:hover { background-color: rgba(255,255,255,0.25); }

/* ── Table ───────────────────────────────────────────── */
QTableWidget {
    background-color: #ffffff;
    border: 1px solid #e0e0e0;
    border-radius: 8px;
    gridline-color: #f0f2f5;
    selection-background-color: #e8f0fe;
    selection-color: #1a73e8;
    alternate-background-color: #f8f9fa;
    outline: none;
}
QTableWidget::item          { padding: 10px 14px; border: none; }
QTableWidget::item:selected { background-color: #e8f0fe; color: #1a73e8; }
QHeaderView::section {
    background-color: #f8f9fa;
    color: #5f6368;
    border: none;
    border-bottom: 2px solid #e0e0e0;
    padding: 10px 14px;
    font-weight: 600;
    font-size: 11px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

/* ── Menu bar ────────────────────────────────────────── */
QMenuBar {
    background-color: #1a73e8;
    color: #ffffff;
    padding: 2px 0;
    border: none;
}
QMenuBar::item                  { background: transparent; padding: 6px 14px; color: #ffffff; }
QMenuBar::item:selected         { background-color: rgba(255,255,255,0.18); border-radius: 4px; }
QMenu                           { background-color: #ffffff; border: 1px solid #e0e0e0; border-radius: 6px; padding: 6px 0; }
QMenu::item                     { padding: 8px 20px; color: #202124; }
QMenu::item:selected            { background-color: #e8f0fe; color: #1a73e8; }
QMenu::separator                { height: 1px; background-color: #e0e0e0; margin: 4px 12px; }

/* ── Status bar ──────────────────────────────────────── */
QStatusBar { background-color: #ffffff; border-top: 1px solid #e0e0e0; color: #5f6368; font-size: 12px; }

/* ── Dialog button box ───────────────────────────────── */
QDialogButtonBox QPushButton { min-width: 82px; }

/* ── Scrollbars ──────────────────────────────────────── */
QScrollBar:vertical   { background: transparent; width: 8px; margin: 0; }
QScrollBar:horizontal { background: transparent; height: 8px; }
QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
    background-color: #dadce0; border-radius: 4px; min-height: 30px; min-width: 30px;
}
QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover { background-color: #adb5bd; }
QScrollBar::add-line, QScrollBar::sub-line { width: 0; height: 0; }
)QSS";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("User Management System"));
    app.setStyleSheet(QString::fromLatin1(kAppStyle));

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
