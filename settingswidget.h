#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>

class settingsWidget : public QLabel
{
    Q_OBJECT
public:
    settingsWidget(QWidget *parent = 0);
};

#endif // SETTINGSWIDGET_H
