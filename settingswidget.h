#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QIntValidator>
#include <QVBoxLayout>
#include <QHBoxLayout>

#define declareItem(type,name) \
    type* name##Value;  \
    QLabel* name##Label; \
    QLabel* name##Msg \

class MainWindow;

class settingsWidget : public QWidget
{
    Q_OBJECT
public:
    settingsWidget(QWidget*, MainWindow*);

private:
    MainWindow* mParent;

    QGridLayout* grid;

    QLabel* heading;
    QLabel* midiSubHeading;

    declareItem(QLineEdit, midiPort);
    declareItem(QLineEdit, ALSAlowWater);
    declareItem(QLineEdit, ALSAhighWater);
    declareItem(QLineEdit, ALSAmaxOutputBuffer);
    declareItem(QLineEdit, ALSAqueueChunkSize);
    declareItem(QLineEdit, ALSApacingInterval);
    declareItem(QCheckBox, ALSAMidiQuashResetAll);

    QLabel* calibreSubHeading;

    declareItem(QLineEdit, calibrePath);
    declareItem(QLineEdit, calibreDatabase);
    declareItem(QLineEdit, calibreMusicTag);

    QLabel* errorSubHeading;

    QPushButton* saveButton;

    bool validateAll();
    void paintEvent(QPaintEvent *);
    bool validateInt(QLineEdit* l, int bottom, int top, QLabel* m);
    bool validateBool(QCheckBox* l, QLabel* m);
    bool validateText(QLineEdit* l, QLabel* m);

};

#endif // SETTINGSWIDGET_H
