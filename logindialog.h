#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui { class LoginDialog; }

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString loggedInUser() const { return m_username; }
    int     privilege() const { return m_privilege; }

private slots:
    void attemptLogin();

private:
    Ui::LoginDialog *ui;
    QString m_username;
    int     m_privilege = -1;
};

#endif // LOGINDIALOG_H
