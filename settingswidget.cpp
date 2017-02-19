// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "settingswidget.h"
#include "mainwindow.h"

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QLabel(p)
{

    mParent = mp;
    this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);

    outerLayout = new QVBoxLayout(this);

    formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->setFormAlignment(Qt::AlignLeft);
    formLayout->setLabelAlignment(Qt::AlignRight);

    heading = new QLabel(this);
    heading->setText("MusicalPi Settings");
    heading->setAlignment(Qt::AlignCenter);
    heading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_HEADING_FONT_SIZE) + "px; font-style: bold;");
    formLayout->addRow(heading);

    midiSubHeading = new QLabel(this);
    midiSubHeading->setText("Midi Player");
    midiSubHeading->setStyleSheet("font-size:" +QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic;");
    formLayout->addRow(midiSubHeading);

    midiPortValue = new QLineEdit(this);
    midiPortValue->setText(QString::number(mParent->ourSettingsPtr->midiPort));
    midiPortValue->setValidator(new QIntValidator(1,512));
    formLayout->addRow("Midi Port:",midiPortValue);

    ALSAlowWaterValue = new QLineEdit(this);
    ALSAlowWaterValue->setText(QString::number(mParent->ourSettingsPtr->ALSAlowWater));
    ALSAlowWaterValue->setValidator(new QIntValidator(10,9999));  // cross-field validate???
    formLayout->addRow("ALSA Queue low water mark:",ALSAlowWaterValue);

    ALSAhighWaterValue = new QLineEdit(this);
    ALSAhighWaterValue->setText(QString::number(mParent->ourSettingsPtr->ALSAhighWater));
    ALSAhighWaterValue->setValidator(new QIntValidator(10,9999));  // cross-field validate???
    formLayout->addRow("ALSA Queue high water mark:",ALSAhighWaterValue);

    ALSAmaxOutputBufferValue = new QLineEdit(this);
    ALSAmaxOutputBufferValue->setText(QString::number(mParent->ourSettingsPtr->ALSAmaxOutputBuffer));
    ALSAmaxOutputBufferValue->setValidator(new QIntValidator(512,16384));
    formLayout->addRow("ALSA Queue size max:",ALSAmaxOutputBufferValue);

    ALSAqueueChunkSizeValue = new QLineEdit(this);
    ALSAqueueChunkSizeValue->setText(QString::number(mParent->ourSettingsPtr->ALSAqueueChunkSize));
    ALSAqueueChunkSizeValue->setValidator(new QIntValidator(1,50));
    formLayout->addRow("ALSA Queue Chunk Size:",ALSAqueueChunkSizeValue);

    ALSApacingIntervalValue = new QLineEdit(this);
    ALSApacingIntervalValue->setText(QString::number(mParent->ourSettingsPtr->ALSApacingInterval));
    ALSApacingIntervalValue->setValidator(new QIntValidator(1,100));
    formLayout->addRow("ALSA pacing Interval (ms):",ALSApacingIntervalValue);

    ALSAMidiQuashResetAllValue = new QCheckBox(this);
    ALSAMidiQuashResetAllValue->setChecked(mParent->ourSettingsPtr->ALSAMidiQuashResetAll);
    formLayout->addRow("ALSA Quash Reset-All:",ALSAMidiQuashResetAllValue);

    calibreSubHeading = new QLabel(this);
    calibreSubHeading->setText("Music Library");
    calibreSubHeading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic;");
    formLayout->addRow(calibreSubHeading);

    calibrePathValue = new QLineEdit(this);
    calibrePathValue->setText(mParent->ourSettingsPtr->calibrePath);
    calibrePathValue->setMinimumWidth(800);
    formLayout->addRow("Calibre Path:",calibrePathValue);

    calibreDatabaseValue = new QLineEdit(this);
    calibreDatabaseValue->setText(mParent->ourSettingsPtr->calibreDatabase);
    calibreDatabaseValue->setMinimumWidth(350);
    formLayout->addRow("Calibre Database:",calibreDatabaseValue);

    calibreMusicTagValue = new QLineEdit(this);
    calibreMusicTagValue->setText(mParent->ourSettingsPtr->calibreMusicTag);
    formLayout->addRow("Calibre Music Tag Value:",calibreMusicTagValue);

    errorMessage = new QLabel(this);
    errorMessage->setText("");
    errorMessage->setMinimumWidth(800);
    formLayout->addRow(errorMessage);

    QList<QLineEdit*> list = this->findChildren<QLineEdit*>();
    foreach(QLineEdit* w, list)
    {
        w->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    }

    outerLayout->addLayout(formLayout);

    saveButton = new QPushButton(this);
    saveButton->setText("Save");
    outerLayout->addWidget(saveButton);
    connect(saveButton,&QPushButton::clicked, this, &settingsWidget::validateAll);
    setLayout(formLayout);

}

void settingsWidget::validateAll()
{
    errorMessage->setText("Something Bad");
}
