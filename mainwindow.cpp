#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "databasemanager.h"
#include "rbac.h"
#include "userdialog.h"
#include "changepassworddialog.h"

#include <QMessageBox>
#include <QTableWidgetItem>
#include <QHeaderView>

MainWindow::MainWindow(const QString &username, int privilege, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_currentUser(username)
    , m_currentPrivilege(privilege)
{
    ui->setupUi(this);
    setWindowTitle(tr("User Management System"));

    ui->currentUserLabel->setText(
        tr("Logged in as: %1    |    Role: %2")
            .arg(m_currentUser, Rbac::roleName(m_currentPrivilege)));

    // --- Table setup ---
    ui->userTable->setColumnCount(2);
    ui->userTable->setHorizontalHeaderLabels(QStringList() << tr("Username") << tr("Role"));
    ui->userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->userTable->verticalHeader()->setVisible(false);

    // --- Menu actions ---
    connect(ui->actionAddUser,        &QAction::triggered, this, &MainWindow::onAddUser);
    connect(ui->actionUpdateUser,     &QAction::triggered, this, &MainWindow::onUpdateUser);
    connect(ui->actionDeleteUser,     &QAction::triggered, this, &MainWindow::onDeleteUser);
    connect(ui->actionChangePassword, &QAction::triggered, this, &MainWindow::onChangePassword);
    connect(ui->actionRefresh,        &QAction::triggered, this, &MainWindow::refreshUserList);
    connect(ui->actionLogout,         &QAction::triggered, this, &MainWindow::onLogout);

    // --- Buttons (mirror the menu) ---
    connect(ui->addButton,            &QPushButton::clicked, this, &MainWindow::onAddUser);
    connect(ui->updateButton,         &QPushButton::clicked, this, &MainWindow::onUpdateUser);
    connect(ui->deleteButton,         &QPushButton::clicked, this, &MainWindow::onDeleteUser);
    connect(ui->changePasswordButton, &QPushButton::clicked, this, &MainWindow::onChangePassword);
    connect(ui->refreshButton,        &QPushButton::clicked, this, &MainWindow::refreshUserList);
    connect(ui->logoutButton,         &QPushButton::clicked, this, &MainWindow::onLogout);

    applyPermissions();
    refreshUserList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::applyPermissions()
{
    const bool canAdd    = Rbac::canUseAddUser(m_currentPrivilege);
    const bool canUpdate = Rbac::canUseUpdateUser(m_currentPrivilege);
    const bool canDelete = Rbac::canUseDeleteUser(m_currentPrivilege);

    // Menu items must be completely hidden when not permitted (per spec).
    ui->actionAddUser->setVisible(canAdd);
    ui->actionUpdateUser->setVisible(canUpdate);
    ui->actionDeleteUser->setVisible(canDelete);
    ui->actionChangePassword->setVisible(true); // available to everyone

    // Mirror visibility on the buttons.
    ui->addButton->setVisible(canAdd);
    ui->updateButton->setVisible(canUpdate);
    ui->deleteButton->setVisible(canDelete);
    ui->changePasswordButton->setVisible(true);

    // A plain User has no manageable list at all.
    const bool hasList = canUpdate || canDelete;
    ui->userTable->setVisible(hasList);
    ui->tableHintLabel->setVisible(hasList);
    ui->refreshButton->setVisible(hasList);
}

void MainWindow::refreshUserList()
{
    ui->userTable->setRowCount(0);

    const QList<int> levels = Rbac::manageableLevels(m_currentPrivilege);
    if (levels.isEmpty())
        return;

    const auto users = DatabaseManager::instance().usersByPrivilege(levels);
    ui->userTable->setRowCount(users.size());
    for (int row = 0; row < users.size(); ++row) {
        auto *nameItem = new QTableWidgetItem(users[row].username);
        auto *roleItem = new QTableWidgetItem(Rbac::roleName(users[row].privilege));
        nameItem->setData(Qt::UserRole, users[row].privilege); // keep privilege handy
        ui->userTable->setItem(row, 0, nameItem);
        ui->userTable->setItem(row, 1, roleItem);
    }
}

QString MainWindow::selectedUsername(int *privilegeOut) const
{
    const int row = ui->userTable->currentRow();
    if (row < 0)
        return QString();
    QTableWidgetItem *nameItem = ui->userTable->item(row, 0);
    if (!nameItem)
        return QString();
    if (privilegeOut)
        *privilegeOut = nameItem->data(Qt::UserRole).toInt();
    return nameItem->text();
}

void MainWindow::onAddUser()
{
    UserDialog dlg(UserDialog::AddMode, m_currentPrivilege, this);
    if (dlg.exec() == QDialog::Accepted)
        refreshUserList();
}

void MainWindow::onUpdateUser()
{
    int targetPriv = -1;
    const QString user = selectedUsername(&targetPriv);
    if (user.isEmpty()) {
        QMessageBox::information(this, tr("Update User"),
                                 tr("Please select a user from the list first."));
        return;
    }
    if (!Rbac::canManage(m_currentPrivilege, targetPriv)) {
        QMessageBox::warning(this, tr("Update User"),
                             tr("You are not authorised to update this user."));
        return;
    }
    UserDialog dlg(UserDialog::EditMode, m_currentPrivilege, this);
    dlg.setUser(user, targetPriv);
    if (dlg.exec() == QDialog::Accepted)
        refreshUserList();
}

void MainWindow::onDeleteUser()
{
    int targetPriv = -1;
    const QString user = selectedUsername(&targetPriv);
    if (user.isEmpty()) {
        QMessageBox::information(this, tr("Delete User"),
                                 tr("Please select a user from the list first."));
        return;
    }
    if (!Rbac::canManage(m_currentPrivilege, targetPriv)) {
        QMessageBox::warning(this, tr("Delete User"),
                             tr("You are not authorised to delete this user."));
        return;
    }
    const auto answer = QMessageBox::question(
        this, tr("Delete User"),
        tr("Permanently delete '%1' (%2)?").arg(user, Rbac::roleName(targetPriv)),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    if (DatabaseManager::instance().deleteUser(user)) {
        QMessageBox::information(this, tr("Delete User"),
                                 tr("User '%1' deleted.").arg(user));
        refreshUserList();
    } else {
        QMessageBox::critical(this, tr("Delete User"),
                              tr("Failed to delete user:\n%1")
                                  .arg(DatabaseManager::instance().lastError()));
    }
}

void MainWindow::onChangePassword()
{
    ChangePasswordDialog dlg(m_currentUser, m_currentPrivilege, this);
    dlg.exec();
}

void MainWindow::onLogout()
{
    // main() inspects this property to decide whether to show the login again.
    setProperty("logout", true);
    close();
}
