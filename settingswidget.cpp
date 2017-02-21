// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>

#include "settingswidget.h"
#include "mainwindow.h"
#include "focuswatcher.h"
#include "oursettings.h"

// These macros are ugly but save a lot of typing when setting up items.
// Note that a QForm wasn't used because I wanted the error messages immediately to the right
// And not silent validation like QValidators are.  These could likely be encapsulated
// into a widget that itself created the entry/message widget and be cleaner but I was
// tired of typing after I did this.

#define createItem(name,prompt,lenPx) \
        values[#name].row=gridRow; \
        v->setObjectName(#name); \
        QLabel* m = new QLabel(containingWidget); \
        QLabel* l = new QLabel(containingWidget); \
        l->setText(prompt); \
        grid->addWidget(l,gridRow,0,1,1,Qt::AlignRight); \
        QHBoxLayout* hb = new QHBoxLayout;  \
        grid->addLayout(hb,gridRow,1,1,1,Qt::AlignLeft); \
        hb->addWidget(v); \
        hb->addWidget(m); \
        gridRow++;  \
        if(lenPx > 0) { v->setMinimumWidth(lenPx); v->setMaximumWidth(lenPx); } \
        m->setMinimumWidth(MUSICALPI_SETTINGS_MSG_MIN_WIDTH); \
        l->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE * 0.8) + "px;"); \
        v->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE) + "px; font-style: bold;"); \
        m->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE) + "px; color: darkRed;") \

#define createUInt(name,prompt,from,to) \
    { \
        QLineEdit* v = new QLineEdit(containingWidget); \
        v->setText(QString::number(mParent->ourSettingsPtr->getSetting(#name).toUInt())); \
        createItem(name,prompt,MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH);  \
        values[#name].type="QLineEdit"; \
        qDebug() << "Connecting for " #name " at gridRow-1 = " << gridRow-1; \
        int thisRow = gridRow - 1; \
        connect(new FocusWatcher(v), &FocusWatcher::focusChanged, this, [this,thisRow](){validateInt(thisRow,from,to);});\
    }
#define createInt(name,prompt,from,to) \
    { \
        QLineEdit* v = new QLineEdit(containingWidget); \
        v->setText(QString::number(mParent->ourSettingsPtr->getSetting(#name).toInt())); \
        createItem(name,prompt,MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH); \
        values[#name].type="QLineEdit"; \
        qDebug() << "Connecting for " #name " at gridRow-1 = " << gridRow-1; \
        int thisRow = gridRow - 1; \
        connect(new FocusWatcher(v), &FocusWatcher::focusChanged, this, [this,thisRow](){validateInt(thisRow,from,to);}); \
    }
#define createBool(name,prompt) \
    { \
        QCheckBox* v = new QCheckBox(containingWidget); \
        v->setChecked(mParent->ourSettingsPtr->getSetting(#name).toBool()); \
        createItem(name,prompt,0);  \
        values[#name].type="QCheckBox"; \
    }
#define createString(name,prompt,lenPx) \
    { \
        QLineEdit *v = new QLineEdit(containingWidget); \
        v->setText(mParent->ourSettingsPtr->getSetting(#name).toString()); \
        createItem(name,prompt,lenPx);  \
        values[#name].type="QLineEdit"; \
    }
#define subHeading(name, prompt) \
    { \
        QLabel* name##SubHeading = new QLabel(containingWidget); \
        name##SubHeading->setText(prompt); \
        name##SubHeading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic;"); \
        grid->addWidget(name##SubHeading,gridRow++,0,1,3,Qt::AlignLeft); \
    }

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QWidget(p)
{
    mParent = mp;
    containingWidget = NULL;
    this->setLayout(new QHBoxLayout());

    // When we create the widget we leave it empty, and only load data on a separate method call
    // so that changes get reflected (otherwise only on first (only) instantiation are they loaded)
}

void settingsWidget::loadData()
{
    qDebug() << "Entered";
    DELETE_LOG(containingWidget);  // In case we are recalling - this deletes all children as well
    values.clear();

    containingWidget = new QWidget(this);
    containingWidget->setObjectName("containingWidget");
    this->layout()->addWidget(containingWidget);

    gridRow = 0;
    grid = new QGridLayout(containingWidget);
    QLabel* heading = new QLabel(containingWidget);
    heading->setText("MusicalPi Settings");
    heading->setAlignment(Qt::AlignCenter);
    heading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_HEADING_FONT_SIZE) + "px; font-style: bold;");
    grid->addWidget(heading,gridRow++,0,1,3,Qt::AlignCenter);

    // You can rearrange these as needed for cosmetics, but if you create any new
    // ones you need to update the defaults defined in ourSettings.cpp

    subHeading(calibre,"Calibre Music Library Integration");

    createString(calibrePath,"Path to calibre library folder:",MUSICALPI_SETTINGS_PATH_WIDTH);
    createString(calibreDatabase,"Database file name (only):", MUSICALPI_SETTINGS_STRING_LEN);
    createString(calibreMusicTag,"Calibre tag for music items:",MUSICALPI_SETTINGS_STRING_LEN);

    subHeading(midi,"Embedded Midi Player (only shows if files of type '.mid' accompany PDF's");

    createUInt(midiPort,"Port:",0,255);
    createBool(ALSAMidiQuashResetAll,"Prevent sending Reset-All:");
    createUInt(midiInitialVelocityScale,"Midi Player default volume %:",10,200);
    createUInt(midiInitialTempoScale,"Midi Player default tempo %:",10,200);

    subHeading(Playing,"Display and Page Turn Controls");

    createInt(logoPct,"Screen % width of logo:",10,50);
    createInt(overlayTopPortion,"Screen % height of top area in play:",5,50);
    createInt(overlaySidePortion,"Screen % height of top area in play:",5,50);
    createInt(pageBorderWidth,"Width of page border (between):",0,100);
    createInt(maxCache,"Cache: Max number of pages kept:",5,100);
    createInt(overlayDuration,"Duration of help overlay during play (ms):",0,5000);
    createInt(pageTurnDelay,"Page turn, time to overwrite current page (ms):",0,5000);
    createInt(pageHighlightDelay,"Page turn, time new page highlights:",0,5000);
    createInt(pageHighlightHeight,"Page turn, highlight border width:",0,5000);

    subHeading(debug,"Debug output controls");

    createBool(debugMidiSendDetails,"Debug output for each midi note sent:");
    createBool(debugMidiFileParseDetails,"Debug output as file is parsed:");
    createBool(debugMidiMeasureDetails,"Debug output for each midi measure parsed:");
    createUInt(debugQueueInfoInterval,"Debug output rate (ms) for queue info:",10,20000);

    errorSubHeading = new QLabel(containingWidget);
    errorSubHeading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic;");
    grid->addWidget(errorSubHeading,gridRow++,0,1,3,Qt::AlignLeft);
    errorSubHeading->setStyleSheet("color:darkRed;");

    saveButton = new QPushButton(containingWidget);
    saveButton->setText("Save");
    grid->addWidget(saveButton,gridRow++,2,1,1,Qt::AlignRight);
    connect(saveButton,&QPushButton::clicked, this, &settingsWidget::validateAll);
}

// In using values we get them by name under their position inside the structure created in the second (index 1) column
// Value is always in position 0 in the layout of that column

#define getWidget(name)  ((QLineEdit*)(grid->itemAtPosition(values[#name].row,1)->layout()->itemAt(0)->widget()))

bool settingsWidget::validateAll()
{

    QFileInfo checkFile(getWidget(calibrePath)->text() + "/" + getWidget(calibreDatabase)->text());
    if(!checkFile.exists() || !checkFile.isFile())
    {
        errorSubHeading->setText("The calibre database path + file name does not find a file - check OS setup");
        getWidget(calibrePath)->setFocus();
        return false;
    }
    errorSubHeading->setText("");

    // Step through each and update if needed/changed

    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
    {
        if(it->second.type == "QLineEdit")  // For updates all we really care about is the text representation
        {
            QLineEdit* v = (QLineEdit*)(grid->itemAtPosition(it->second.row,1)->layout()->itemAt(0)->widget());
            qDebug() << "Comparison '" << mParent->ourSettingsPtr->getSetting(it->first).toString() << "' vs '"<< v->text() <<  "'";
            if(mParent->ourSettingsPtr->getSetting(it->first).toString() != v->text())
                mParent->ourSettingsPtr->setSetting(it->first,v->text());
        }
        else if(it->second.type == "QCheckBox")
        {
            QCheckBox* v = (QCheckBox*)(grid->itemAtPosition(it->second.row,1)->layout()->itemAt(0)->widget());
            if(mParent->ourSettingsPtr->getSetting(it->first).toBool() != v->isChecked())
                mParent->ourSettingsPtr->setSetting(it->first,v->isChecked());
        }
    }

    return true;
}

bool settingsWidget::validateInt(int row, int bottom, int top)
{
    qDebug() << "row=" << row << ", bottom=" << bottom << ", top=" << top;
    QLabel* m = (QLabel*)(grid->itemAtPosition(row,1)->layout()->itemAt(1)->widget());
    QLineEdit* l = (QLineEdit*)(grid->itemAtPosition(row,1)->layout()->itemAt(0)->widget());

    // Validate int (or uint) field, that it is a number and is in range [bottom,top], giving message in label if not (clearing if so)
    bool ok;
    int newVal = l->text().toInt(&ok);
    if(!ok)
    {
        m->setText("Invalid integer number format");
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
    return true; // Note on success we aren't affecting focus
}

void settingsWidget::paintEvent(QPaintEvent *)  // This is here so we can use stylesheet styling if needed
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
