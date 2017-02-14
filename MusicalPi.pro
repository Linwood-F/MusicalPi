#--------------------------------------------------------------------
#
# Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0
#
#--------------------------------------------------------------------

QT       += core gui
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MusicalPi
TEMPLATE = app

DEFINES += QT_MESSAGELOGCONTEXT

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
    settingswidget.cpp \
    midiplayerV2.cpp

HEADERS  += mainwindow.h \
    pdfdocument.h \
    tipoverlay.h \
    docpagelabel.h \
    debugmessages.h \
    piconstants.h \
    renderthread.h \
    musiclibrary.h \
    aboutwidget.h \
    settingswidget.h \
    midiplayerV2.h

DISTFILES += \
    MusicalPi.gif \
    MusicalPi.psd \
    LICENSE \
    README

unix|win32: LIBS += -lQt5DBus
unix|win32: LIBS += -lpoppler-qt5
unix|win32: LIBS += -L../midifile/lib -lmidifile
unix|win32: LIBS += -lasound

INCLUDEPATH += ../midifile/include


