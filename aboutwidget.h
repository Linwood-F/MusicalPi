#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>

class aboutWidget : public QLabel
{
    Q_OBJECT
public:
    aboutWidget(QWidget *parent = 0);
};

#endif // ABOUTWIDGET_H
