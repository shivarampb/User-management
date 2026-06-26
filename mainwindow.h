#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &username, int privilege, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddUser();
    void onUpdateUser();
    void onDeleteUser();
    void onChangePassword();
    void onLogout();
    void refreshUserList();

private:
    void applyPermissions();                               // hide actions/buttons per role
    QString selectedUsername(int *privilegeOut = nullptr) const;

    Ui::MainWindow *ui;
    QString m_currentUser;
    int     m_currentPrivilege;
};

#endif // MAINWINDOW_H
