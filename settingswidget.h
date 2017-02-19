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
#include <QPushButton>
#include <QIntValidator>
#include <QVBoxLayout>

class MainWindow;

class settingsWidget : public QLabel
{
    Q_OBJECT
public:
    settingsWidget(QWidget*, MainWindow*);

private:
    MainWindow* mParent;

    QVBoxLayout* outerLayout;

    QFormLayout* formLayout;

    QLabel* heading;
    QLabel* midiSubHeading;

    QLineEdit* midiPortValue;
    QLineEdit* ALSAlowWaterValue;
    QLineEdit* ALSAhighWaterValue;
    QLineEdit* ALSAmaxOutputBufferValue;
    QLineEdit* ALSAqueueChunkSizeValue;
    QLineEdit* ALSApacingIntervalValue;
    QCheckBox* ALSAMidiQuashResetAllValue;

    QLabel* calibreSubHeading;

    QLineEdit* calibrePathValue;
    QLineEdit* calibreDatabaseValue;
    QLineEdit* calibreMusicTagValue;

    QLabel* errorMessage;

    QPushButton* saveButton;

    void validateAll();
};

#endif // SETTINGSWIDGET_H
