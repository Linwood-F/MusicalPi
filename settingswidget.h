#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

// Copyright 2018 by LE Ferguson, LLC, licensed under Apache 2.0

#include "settingsitem.h"

class MainWindow;
class QLineEdit;
class QVBoxLayout;
class QPushButton;

class settingsWidget : public QWidget
{
    Q_OBJECT
public:
    settingsWidget(QWidget*, MainWindow*);
    void loadData();
    MainWindow* mParent;
    typedef  std::map<QString,settingsItem*> rowMap_t;  // value key and row in table
    rowMap_t values;
    QVBoxLayout* innerLayout;

private:


    QWidget* containingWidget; // Just so we can delete enmass easily
    QLabel* errorSubHeading;

    QPushButton* saveButton;

    bool validateAll();
    void paintEvent(QPaintEvent *);
    bool validateInt(int row, int bottom, int top);

};

#endif // SETTINGSWIDGET_H
