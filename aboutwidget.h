#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QLabel>

class aboutWidget : public QLabel
{
    Q_OBJECT
public:
    aboutWidget(QWidget *parent = 0);
    ~aboutWidget();
};

#endif // ABOUTWIDGET_H
