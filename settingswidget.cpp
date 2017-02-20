// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include "settingswidget.h"
#include "mainwindow.h"
#include "focuswatcher.h"
#include <QFileInfo>

#define COMMA ,
#define createItem(type,name,prompt,lenPx,verb,modifier,validator,param) \
    { \
        name##Value = new type(this); \
        name##Value->verb(modifier(mParent->ourSettingsPtr->name));   \
        name##Value->setObjectName(#name); \
        name##Msg = new QLabel(this); \
        name##Label = new QLabel(this); \
        name##Label->setText(prompt); \
        grid->addWidget(name##Label,gridRow,0,1,1,Qt::AlignRight); \
        QHBoxLayout* hb = new QHBoxLayout;  \
        grid->addLayout(hb,gridRow,1,1,1,Qt::AlignLeft); \
        hb->addWidget(name##Value); \
        hb->addWidget(name##Msg); \
        gridRow++;  \
        if(lenPx > 0) { name##Value->setMinimumWidth(lenPx); name##Value->setMaximumWidth(lenPx); } \
        name##Msg->setMinimumWidth(MUSICALPI_SETTINGS_MSG_MIN_WIDTH); \
        name##Label->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE * 0.8) + "px;"); \
        name##Value->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE) + "px; font-style: bold;"); \
        name##Msg->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE) + "px; color: darkRed;"); \
        connect(new FocusWatcher(name##Value), &FocusWatcher::focusChanged, this, [this](){validator(name##Value,param name##Msg);}); \
    }

#define subHeading(name, prompt) \
    name##SubHeading = new QLabel(this); \
    name##SubHeading->setText(prompt); \
    name##SubHeading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic;"); \
    grid->addWidget(name##SubHeading,gridRow++,0,1,3,Qt::AlignLeft) \

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QWidget(p)
{
    mParent = mp;
    int gridRow = 0;
    grid = new QGridLayout(this);
    heading = new QLabel(this);
    heading->setText("MusicalPi Settings");
    heading->setAlignment(Qt::AlignCenter);
    heading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_HEADING_FONT_SIZE) + "px; font-style: bold;");
    grid->addWidget(heading,gridRow++,0,1,3,Qt::AlignCenter);

    subHeading(midi,"Midi Player");

    createItem(QLineEdit, midiPort,    "Port:",MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,0 COMMA 255 COMMA);
    createItem(QLineEdit, ALSAlowWater,"Queue Low Water Mark:",MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,10 COMMA 9999 COMMA);
    createItem(QLineEdit, ALSAhighWater,"QUeue High Water Mark:",MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,100  COMMA 9999 COMMA);
    createItem(QLineEdit, ALSAmaxOutputBuffer,"Queue Size (in events):",MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,512  COMMA 99999 COMMA);
    createItem(QLineEdit, ALSAqueueChunkSize,"Event processing unit of work (events):",MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,1  COMMA 500 COMMA);
    createItem(QLineEdit, ALSApacingInterval,"Event processing idle delay (ms):", MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH,setText,QString::number,validateInt,1  COMMA 200 COMMA);
    createItem(QCheckBox, ALSAMidiQuashResetAll,"Prevent sending Reset-All:",0,setChecked,,validateBool,);

    subHeading(calibre,"Music Library");

    createItem(QLineEdit, calibrePath,"Path to calibre library folder:",MUSICALPI_SETTINGS_PATH_WIDTH,setText,,validateText,);
    createItem(QLineEdit, calibreDatabase,"Database file name (only):", MUSICALPI_SETTINGS_STRING_LEN,setText,,validateText,);
    createItem(QLineEdit, calibreMusicTag,"Calibre tag for music items:",MUSICALPI_SETTINGS_STRING_LEN,setText,,validateText,);

    subHeading(error,"");
    errorSubHeading->setStyleSheet("color:darkRed;");

    saveButton = new QPushButton(this);
    saveButton->setText("Save");
    grid->addWidget(saveButton,gridRow++,2,1,1,Qt::AlignRight);
    connect(saveButton,&QPushButton::clicked, this, &settingsWidget::validateAll);
}
bool settingsWidget::validateAll()
{
    if(ALSAlowWaterValue->text().toInt() < ALSAqueueChunkSizeValue->text().toInt())
    {
        errorSubHeading->setText("The low water size must exceed the chunk size");
        ALSAlowWaterValue->setFocus();
        return false;
    }
    if(ALSAhighWaterValue->text().toInt() - ALSAlowWaterValue->text().toInt() < 2 * ALSAqueueChunkSizeValue->text().toInt())
    {
        errorSubHeading->setText("The difference in high and low water mark must be more than twice the chunk size");
        ALSAlowWaterValue->setFocus();
        return false;
    }
    if(ALSAhighWaterValue->text().toInt() >= ALSAmaxOutputBufferValue->text().toInt())
    {
        errorSubHeading->setText("The high water mark should be much less than the queue size");
        ALSAhighWaterValue->setFocus();
        return false;
    }
    if(ALSAqueueChunkSizeValue->text().toInt() > ALSAmaxOutputBufferValue->text().toInt() / 4)
    {
        errorSubHeading->setText("The chunk size should be much less than the queue size");
        ALSAqueueChunkSizeValue->setFocus();
        return false;
    }
    QFileInfo checkFile(calibrePathValue->text() + "/" + calibreDatabaseValue->text());
    if(!checkFile.exists() || !checkFile.isFile())
    {
        errorSubHeading->setText("The calibre database path + file name does not find a file - check OS setup");
        calibrePathValue->setFocus();
        return false;
    }
    errorSubHeading->setText("");
    return true;
}
bool settingsWidget::validateInt(QLineEdit* l, int bottom, int top, QLabel* m)
{
    // Validate int field that it is an int, and is in range [bottom,top], giving message in label if not (clearing if so)
    bool ok;
    int newVal = l->text().toInt(&ok);
    if(!ok)
    {
        m->setText("Invalid number format");
        l->setFocus();
        return false;
    }
    if(newVal < bottom || newVal > top)
    {
        m->setText("Value must be between " + QString::number(bottom) + " and " + QString::number(top));
        l->setFocus();
        return false;
    }
    m->setText("");
    return true;
}
bool settingsWidget::validateBool(QCheckBox* l, QLabel* m) // Just present so macro works
{
    return true;
}
bool settingsWidget::validateText(QLineEdit* l, QLabel* m) // just present so macro works
{
    return true;
}

void settingsWidget::paintEvent(QPaintEvent *)  // This is here so we can use styles
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
