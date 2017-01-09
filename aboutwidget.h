#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>
#include <QDebug>

class aboutWidget : public QLabel
{
    Q_OBJECT
public:
    aboutWidget(QWidget *parent = 0);
    void play();
};

#endif // ABOUTWIDGET_H
