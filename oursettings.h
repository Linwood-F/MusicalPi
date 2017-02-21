#ifndef OURSETTINGS_H
#define OURSETTINGS_H

#include <QSettings>
#include <QVariant>

#include "piconstants.h"

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

class MainWindow;

class ourSettings
{
public:
    ourSettings(MainWindow* parent);
    ~ourSettings();
    QVariant getSetting(QString key);
    QVariant setSetting(QString key, QVariant value);
    QSettings* setPtr;
    MainWindow* mParent;

private:
    void getSaveValues();
};

#endif // OURSETTINGS_H
