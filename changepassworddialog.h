#ifndef CHANGEPASSWORDDIALOG_H
#define CHANGEPASSWORDDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui { class ChangePasswordDialog; }

// "Windows-style" verification: regardless of whose password is being
// changed, the dialog requires the CURRENTLY LOGGED-IN user to re-enter
// their own active password to authorise the change.
class ChangePasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ChangePasswordDialog(const QString &currentUsername,
                                  int currentPrivilege,
                                  QWidget *parent = nullptr);
    ~ChangePasswordDialog();

private slots:
    void apply();

private:
    void populateTargets();

    Ui::ChangePasswordDialog *ui;
    QString m_currentUsername;
    int     m_currentPrivilege;
};

#endif // CHANGEPASSWORDDIALOG_H
