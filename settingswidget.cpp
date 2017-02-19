// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "settingswidget.h"
#include "mainwindow.h"

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QLabel(p)
{

    mParent = mp;
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
    formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    //    heading = new QLabel(this);
//    heading->setText("MusicalPi Settings");
//    int gridRow = 0;
//    int gridColumns = 2;
//    grid->addWidget(heading,gridRow++,0,1,gridColumns,Qt::AlignCenter);

//    midiSubHeading = new QLabel(this);
//    midiSubHeading->setText("Midi Player");
//    grid->addWidget(midiSubHeading,gridRow++,0,1,gridColumns,Qt::AlignLeft);


    midiPortValue = new QLineEdit(this);
    midiPortValue->setText(QString::number(mParent->ourSettingsPtr->midiPort));
    formLayout->addRow("Midi Port:",midiPortValue);


    ALSAlowWaterValue = new QLineEdit(this);
    ALSAlowWaterValue->setText(QString::number(mParent->ourSettingsPtr->ALSAlowWater));
    formLayout->addRow("ALSA Queue low water mark:",ALSAlowWaterValue);

    ALSAhighWaterValue = new QLineEdit(this);
    ALSAhighWaterValue->setText(QString::number(mParent->ourSettingsPtr->ALSAhighWater));
    formLayout->addRow("ALSA Queue high water mark:",ALSAhighWaterValue);

    ALSAmaxOutputBufferValue = new QLineEdit(this);
    ALSAmaxOutputBufferValue->setText(QString::number(mParent->ourSettingsPtr->ALSAmaxOutputBuffer));
    formLayout->addRow("ALSA Queue size max:",ALSAmaxOutputBufferValue);

    ALSAqueueChunkSizeValue = new QLineEdit(this);
    ALSAqueueChunkSizeValue->setText(QString::number(mParent->ourSettingsPtr->ALSAqueueChunkSize));
    formLayout->addRow("ALSA Queue Chunk Size:",ALSAqueueChunkSizeValue);

    ALSApacingIntervalValue = new QLineEdit(this);
    ALSApacingIntervalValue->setText(QString::number(mParent->ourSettingsPtr->ALSApacingInterval));
    formLayout->addRow("ALSA pacing Interval (ms):",ALSApacingIntervalValue);

    ALSAMidiQuashResetAllValue = new QCheckBox(this);
    ALSAMidiQuashResetAllValue->setChecked(mParent->ourSettingsPtr->ALSAMidiQuashResetAll);
    formLayout->addRow("ALSA Quash Reset-All:",ALSAMidiQuashResetAllValue);

//    calibreSubHeading = new QLabel(this);
//    calibreSubHeading->setText("Music Library");
//    grid->addWidget(calibreSubHeading,gridRow++,0,1,gridColumns,Qt::AlignLeft);

    calibrePathValue = new QLineEdit(this);
    calibrePathValue->setText(mParent->ourSettingsPtr->calibrePath);
    formLayout->addRow("Calibre Path:",calibrePathValue);

    calibreDatabaseValue = new QLineEdit(this);
    calibreDatabaseValue->setText(mParent->ourSettingsPtr->calibreDatabase);
    formLayout->addRow("Calibre Database:",calibreDatabaseValue);

    calibreMusicTagValue = new QLineEdit(this);
    calibreMusicTagValue->setText(mParent->ourSettingsPtr->calibreMusicTag);
    formLayout->addRow("Calibre Music Tag Value:",calibreMusicTagValue);

    setLayout(formLayout);
}


