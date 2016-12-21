#-------------------------------------------------
#
# Project created by QtCreator 2016-10-29T21:05:21
#
#-------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestMusic
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
    aboutwidget.cpp

HEADERS  += mainwindow.h \
    pdfdocument.h \
    tipoverlay.h \
    docpagelabel.h \
    debugmessages.h \
    piconstants.h \
    renderthread.h \
    musiclibrary.h \
    aboutwidget.h

DISTFILES += \
    MusicalPi.gif \
    MusicalPi.psd

FORMS +=

unix|win32: LIBS += -lQt5DBus
unix|win32: LIBS += -lpoppler-qt5

#INCLUDEPATH += /usr/include/poppler/qt5
#DEPENDPATH += /usr/include/poppler/qt5
