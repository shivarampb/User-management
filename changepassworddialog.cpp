#include "changepassworddialog.h"
#include "ui_changepassworddialog.h"
#include "databasemanager.h"
#include "rbac.h"

#include <QMessageBox>

ChangePasswordDialog::ChangePasswordDialog(const QString &currentUsername,
                                           int currentPrivilege,
                                           QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangePasswordDialog)
    , m_currentUsername(currentUsername)
    , m_currentPrivilege(currentPrivilege)
{
    ui->setupUi(this);
    setWindowTitle(tr("Change Password"));

    ui->currentPasswordEdit->setEchoMode(QLineEdit::Password);
    ui->newPasswordEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    populateTargets();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ChangePasswordDialog::apply);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ChangePasswordDialog::reject);
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    delete ui;
}

void ChangePasswordDialog::populateTargets()
{
    ui->targetCombo->clear();

    // Self is always a valid target.
    ui->targetCombo->addItem(
        tr("%1  (yourself - %2)").arg(m_currentUsername, Rbac::roleName(m_currentPrivilege)),
        m_currentUsername);

    // Plus any users the actor is allowed to manage.
    const QList<int> levels = Rbac::manageableLevels(m_currentPrivilege);
    if (!levels.isEmpty()) {
        const auto users = DatabaseManager::instance().usersByPrivilege(levels);
        for (const auto &u : users) {
            if (u.username == m_currentUsername)
                continue; // already listed as "yourself"
            ui->targetCombo->addItem(
                tr("%1  (%2)").arg(u.username, Rbac::roleName(u.privilege)),
                u.username);
        }
    }
}

void ChangePasswordDialog::apply()
{
    const QString target      = ui->targetCombo->currentData().toString();
    const QString currentPass = ui->currentPasswordEdit->text();
    const QString newPass     = ui->newPasswordEdit->text();
    const QString confirmPass = ui->confirmPasswordEdit->text();

    if (currentPass.isEmpty() || newPass.isEmpty()) {
        QMessageBox::warning(this, tr("Change Password"),
                             tr("Please fill in all password fields."));
        return;
    }
    if (newPass != confirmPass) {
        QMessageBox::warning(this, tr("Change Password"),
                             tr("New password and confirmation do not match."));
        return;
    }

    DatabaseManager &db = DatabaseManager::instance();

    // 1) "Windows-style" authorisation: verify the CURRENT user's own
    //    active password, regardless of whose password is being changed.
    if (!db.verifyPassword(m_currentUsername, currentPass)) {
        QMessageBox::critical(this, tr("Authorisation Failed"),
                              tr("Your current password is incorrect. "
                                 "The change has not been authorised."));
        return;
    }

    // 2) Defence-in-depth: confirm the actor is allowed to change this target.
    const bool isSelf = (target == m_currentUsername);
    UserRecord rec;
    if (!db.getUser(target, rec)) {
        QMessageBox::critical(this, tr("Error"), tr("Target user not found."));
        return;
    }
    if (!Rbac::canChangePassword(m_currentPrivilege, rec.privilege, isSelf)) {
        QMessageBox::critical(this, tr("Permission Denied"),
                              tr("You are not authorised to change this user's password."));
        return;
    }

    // 3) Apply the change.
    if (db.changePassword(target, newPass)) {
        QMessageBox::information(this, tr("Change Password"),
                                 tr("Password updated successfully for '%1'.").arg(target));
        accept();
    } else {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to update password:\n%1").arg(db.lastError()));
    }
}
