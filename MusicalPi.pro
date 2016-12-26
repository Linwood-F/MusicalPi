#-------------------------------------------------
#
# Project created by QtCreator 2016-10-29T21:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MusicalPi
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    pdfdocument.cpp \
    tipoverlay.cpp \
    docpagelabel.cpp \
    debugmessages.cpp \
    renderthread.cpp \
    musiclibrary.cpp \
    aboutwidget.cpp \
    settingswidget.cpp

HEADERS  += mainwindow.h \
    pdfdocument.h \
    tipoverlay.h \
    docpagelabel.h \
    debugmessages.h \
    piconstants.h \
    renderthread.h \
    musiclibrary.h \
    aboutwidget.h \
    settingswidget.h

DISTFILES += \
    MusicalPi.gif \
    MusicalPi.psd \
    LICENSE \
    README

unix|win32: LIBS += -lQt5DBus
unix|win32: LIBS += -lpoppler-qt5

