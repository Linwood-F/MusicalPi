#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include <QLabel>
#include <QPlainTextEdit>
#include <QWidget>
#include <QApplication>

class aboutWidget : public QTextEdit
{
    Q_OBJECT
public:
    aboutWidget(QWidget *parent = 0);
    ~aboutWidget();
};

#endif // ABOUTWIDGET_H
