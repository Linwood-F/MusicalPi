#ifndef OURSETTINGS_H
#define OURSETTINGS_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

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
