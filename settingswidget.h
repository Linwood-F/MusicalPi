#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QMap>
#include <QString>

class MainWindow;

class settingsWidget : public QWidget
{
    Q_OBJECT
public:
    settingsWidget(QWidget*, MainWindow*);
    void loadData();

private:
    MainWindow* mParent;

    QWidget* containingWidget;
    QGridLayout* grid;

    QLabel* errorSubHeading;

    QPushButton* saveButton;

    bool validateAll();
    void paintEvent(QPaintEvent *);
    bool validateInt(int row, int bottom, int top);
    int gridRow;

    struct widgets_t {int row; QString type;};

    typedef  std::map<QString,widgets_t> rowMap_t;  // value key and row in table
    rowMap_t values;
};

#endif // SETTINGSWIDGET_H
