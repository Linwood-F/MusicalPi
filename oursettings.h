#ifndef OURSETTINGS_H
#define OURSETTINGS_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QVariant>

#include "piconstants.h"

class MainWindow;
class QSettings;

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
    bool debugSettingsAsLoaded;
};

#endif // OURSETTINGS_H
