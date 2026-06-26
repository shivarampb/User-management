#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui { class UserDialog; }

// Handles both "Add User" and "Update User" features.
// The password field is only shown in Add mode; existing passwords are
// changed through the dedicated, verified Change Password feature.
class UserDialog : public QDialog
{
    Q_OBJECT
public:
    enum Mode { AddMode, EditMode };

    explicit UserDialog(Mode mode, int actorPrivilege, QWidget *parent = nullptr);
    ~UserDialog();

    // Pre-load existing data for EditMode.
    void setUser(const QString &username, int privilege);

private slots:
    void apply();

private:
    void populateRoles();

    Ui::UserDialog *ui;
    Mode    m_mode;
    int     m_actorPrivilege;
    QString m_originalUsername;
};

#endif // USERDIALOG_H
