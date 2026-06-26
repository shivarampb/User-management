#include "logindialog.h"
#include "ui_logindialog.h"
#include "databasemanager.h"

#include <QMessageBox>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("User Management - Login"));
    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    connect(ui->loginButton,  &QPushButton::clicked,    this, &LoginDialog::attemptLogin);
    connect(ui->cancelButton, &QPushButton::clicked,    this, &LoginDialog::reject);
    connect(ui->passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::attemptLogin);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::attemptLogin()
{
    const QString user = ui->usernameEdit->text().trimmed();
    const QString pass = ui->passwordEdit->text();

    if (user.isEmpty() || pass.isEmpty()) {
        QMessageBox::warning(this, tr("Login"),
                             tr("Please enter both username and password."));
        return;
    }

    int priv = -1;
    if (DatabaseManager::instance().authenticate(user, pass, priv)) {
        m_username  = user;
        m_privilege = priv;
        accept();
    } else {
        QMessageBox::critical(this, tr("Login Failed"),
                              tr("Invalid username or password."));
        ui->passwordEdit->clear();
        ui->passwordEdit->setFocus();
    }
}
