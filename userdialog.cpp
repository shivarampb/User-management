#include "userdialog.h"
#include "ui_userdialog.h"
#include "databasemanager.h"
#include "rbac.h"

#include <QMessageBox>

UserDialog::UserDialog(Mode mode, int actorPrivilege, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserDialog)
    , m_mode(mode)
    , m_actorPrivilege(actorPrivilege)
{
    ui->setupUi(this);
    setWindowTitle(mode == AddMode ? tr("Add User") : tr("Update User"));

    ui->passwordEdit->setEchoMode(QLineEdit::Password);

    // Passwords are only set here when creating a new user. Changing an
    // existing password must go through the verified Change Password flow.
    if (mode == EditMode) {
        ui->passwordLabel->hide();
        ui->passwordEdit->hide();
    }

    populateRoles();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &UserDialog::apply);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &UserDialog::reject);
}

UserDialog::~UserDialog()
{
    delete ui;
}

void UserDialog::populateRoles()
{
    ui->roleCombo->clear();
    const QList<int> levels = Rbac::assignableLevels(m_actorPrivilege);
    for (int lvl : levels)
        ui->roleCombo->addItem(Rbac::roleName(lvl), lvl);
}

void UserDialog::setUser(const QString &username, int privilege)
{
    m_originalUsername = username;
    ui->usernameEdit->setText(username);

    const int idx = ui->roleCombo->findData(privilege);
    if (idx >= 0)
        ui->roleCombo->setCurrentIndex(idx);
}

void UserDialog::apply()
{
    const QString username = ui->usernameEdit->text().trimmed();

    if (username.isEmpty()) {
        QMessageBox::warning(this, windowTitle(), tr("Username cannot be empty."));
        return;
    }
    if (ui->roleCombo->currentIndex() < 0) {
        QMessageBox::warning(this, windowTitle(),
                             tr("You are not permitted to assign any role."));
        return;
    }
    const int privilege = ui->roleCombo->currentData().toInt();

    // Defence-in-depth: re-check authority before touching the database.
    if (!Rbac::canAdd(m_actorPrivilege, privilege)) {
        QMessageBox::critical(this, windowTitle(),
                              tr("You are not authorised to assign this role."));
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();

    if (m_mode == AddMode) {
        const QString password = ui->passwordEdit->text();
        if (password.isEmpty()) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("Please set a password for the new user."));
            return;
        }
        if (db.userExists(username)) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("A user named '%1' already exists.").arg(username));
            return;
        }
        if (db.addUser(username, password, privilege)) {
            QMessageBox::information(this, windowTitle(),
                                     tr("User '%1' added.").arg(username));
            accept();
        } else {
            QMessageBox::critical(this, windowTitle(),
                                  tr("Failed to add user:\n%1").arg(db.lastError()));
        }
    } else { // EditMode
        if (username != m_originalUsername && db.userExists(username)) {
            QMessageBox::warning(this, windowTitle(),
                                 tr("A user named '%1' already exists.").arg(username));
            return;
        }
        if (db.updateUser(m_originalUsername, username, privilege)) {
            QMessageBox::information(this, windowTitle(),
                                     tr("User '%1' updated.").arg(username));
            accept();
        } else {
            QMessageBox::critical(this, windowTitle(),
                                  tr("Failed to update user:\n%1").arg(db.lastError()));
        }
    }
}
