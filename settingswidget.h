#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>
#include <QGridLayout>
#include <QLineEdit>
#include <QCheckBox>
#include <QFormLayout>

class MainWindow;

class settingsWidget : public QLabel
{
    Q_OBJECT
public:
    settingsWidget(QWidget*, MainWindow*);

private:
    MainWindow* mParent;

    QFormLayout* formLayout;

//    QLabel* heading;
//    QGridLayout* grid;
//    QLabel* midiSubHeading;

    QLineEdit*  midiPortValue;

    QLineEdit* ALSAlowWaterValue;

    QLineEdit* ALSAhighWaterValue;

    QLineEdit* ALSAmaxOutputBufferValue;

    QLineEdit* ALSAqueueChunkSizeValue;

    QLineEdit* ALSApacingIntervalValue;

    QCheckBox* ALSAMidiQuashResetAllValue;

//    QLabel* calibreSubHeading;

    QLineEdit* calibrePathValue;

    QLineEdit* calibreDatabaseValue;

    QLineEdit* calibreMusicTagValue;
};

#endif // SETTINGSWIDGET_H
