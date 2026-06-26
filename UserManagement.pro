#-------------------------------------------------
# User Management System (RBAC) - Qt 5.14.2 / MariaDB
#-------------------------------------------------

QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG   += c++11

TARGET    = UserManagement
TEMPLATE  = app

DEFINES  += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    databasemanager.cpp \
    rbac.cpp \
    logindialog.cpp \
    mainwindow.cpp \
    userdialog.cpp \
    changepassworddialog.cpp

HEADERS += \
    databasemanager.h \
    rbac.h \
    logindialog.h \
    mainwindow.h \
    userdialog.h \
    changepassworddialog.h

FORMS += \
    logindialog.ui \
    mainwindow.ui \
    userdialog.ui \
    changepassworddialog.ui
