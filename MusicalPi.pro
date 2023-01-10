#--------------------------------------------------------------------
#
# Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3
#
#--------------------------------------------------------------------

QT       += core gui dbus
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

defineTest(copyToDestDir) {
    files = $$1

    for(FILE, files) {
        DDIR = $$DESTDIR
                    FILE = $$absolute_path($$FILE)

        # Replace slashes in paths with backslashes for Windows
        win32:FILE ~= s,/,\\,g
        win32:DDIR ~= s,/,\\,g

        QMAKE_POST_LINK += $$QMAKE_COPY $$quote($$FILE) $$quote($$DDIR) $$escape_expand(\\n\\t)
    }

    export(QMAKE_POST_LINK)
}

TEMPLATE = app

TARGET = MusicalPi

DESTDIR = ../MusicalPi_Kit

copyToDestDir(LICENSE MusicalPi.gif)

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
    midiplayerV2.cpp \
    midiplayerv2thread.cpp \
    oursettings.cpp \
    focuswatcher.cpp \
    settingsitem.cpp \
    playlists.cpp

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
    midiplayerV2.h \
    midiplayerv2thread.h \
    oursettings.h \
    focuswatcher.h \
    settingsitem.h \
    playlists.h

DISTFILES += \
    MusicalPi.gif \
    MusicalPi.psd \
    LICENSE \
    README \
    rhythmbox.png \
    brcmfmac43455-sdio.txt \
    onReboot \
    brcmfmac43455-sdio.clm_blob

unix|win32: LIBS += -lpoppler-qt6
unix|win32: LIBS += -L../midifile/lib -lmidifile
unix|win32: LIBS += -lasound

INCLUDEPATH += ../midifile/include
INCLUDEPATH += ../poppler/src/


