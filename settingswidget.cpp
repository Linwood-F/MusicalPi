// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0
#include <QFileInfo>
#include <QFont>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QFrame>

#include "settingswidget.h"
#include "mainwindow.h"
#include "focuswatcher.h"
#include "oursettings.h"
#include "settingsitem.h"

// These macros are ugly but save a lot of typing when setting up items.
// Note that a QForm wasn't used because I wanted the error messages immediately to the right
// And not silent validation like QValidators are.  These could likely be encapsulated
// into a widget that itself created the entry/message widget and be cleaner but I was
// tired of typing after I did this.

//        innerLayout->addWidget(si);
//        values[name]=si;

#define Heading(prompt) \
    { \
        QLabel* l = new QLabel(containingWidget); \
        l->setText(prompt); \
        l->setAlignment(Qt::AlignCenter);  \
        l->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_HEADING_FONT_SIZE) + "px; font-style: Bold; color: Green;"); \
        innerLayout->addWidget(l); \
    }

#define subHeading(prompt) \
    { \
        QLabel* l = new QLabel(containingWidget); \
        l->setText(prompt); \
        l->setAlignment(Qt::AlignLeft);  \
        l->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE) + "px; font-style: Italic; color: Blue;"); \
        innerLayout->addWidget(l); \
    }

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QWidget(p)
{
    mParent = mp;
    containingWidget = NULL;
    this->setLayout(new QHBoxLayout()); // Always need some layout on ourselves
    this->layout()->setContentsMargins(0,0,0,0);

    // When we create the widget we leave it empty, and only load data on a separate method call
    // so that changes get reflected (otherwise only on first (only) instantiation are they loaded)
}

void settingsWidget::loadData()
{
    qDebug() << "Entered";

    // Clear anything from last time

    DELETE_LOG(containingWidget);  // In case we are recalling - this deletes all children as well
    containingWidget = new QWidget(this);
    containingWidget->setObjectName("containingWidget");
    this->layout()->addWidget(containingWidget); // Containing widget is inside our (settingsWidget) self's layout

    innerLayout = new QVBoxLayout(containingWidget);  // the containing widget lays out vertically
    innerLayout->setContentsMargins(0,0,0,0);         // and has no spaces between them.
    innerLayout->setSpacing(2);

    Heading("MusicalPi Settings");

    // You can rearrange these as needed for cosmetics, but if you create any new
    // ones you need to update the defaults defined in ourSettings.cpp

    subHeading("Calibre Music Library Integration");

    new settingsItem(this, containingWidget, "calibrePath","Path to calibre library folder:",MUSICALPI_SETTINGS_PATH_WIDTH);

    new settingsItem(this, containingWidget, "calibreDatabase","Database file name (only):", MUSICALPI_SETTINGS_STRING_LEN);
    new settingsItem(this, containingWidget, "calibreMusicTag","Calibre tag for music items:",MUSICALPI_SETTINGS_STRING_LEN);

    subHeading("Embedded Midi Player (only shows if files of type '.mid' accompany PDF's");

    new settingsItem(this, containingWidget, "midiPort","Port:",0,255);
    new settingsItem(this, containingWidget, "ALSAMidiQuashResetAll","Prevent sending Reset-All:");
    new settingsItem(this, containingWidget, "midiInitialVelocityScale","Midi Player default volume %:",10,200);
    new settingsItem(this, containingWidget, "midiInitialTempoScale","Midi Player default tempo %:",10,200);

    subHeading("Display and Page Turn Controls");

    new settingsItem(this, containingWidget, "logoPct","Screen % width of logo:",10,50);
    new settingsItem(this, containingWidget, "overlayTopPortion","Screen % height of top area in play:",5,50);
    new settingsItem(this, containingWidget, "overlaySidePortion","Screen % height of top area in play:",5,50);
    new settingsItem(this, containingWidget, "pageBorderWidth","Width of page border (between):",0,100);
    new settingsItem(this, containingWidget, "maxCache","Cache: Max number of pages kept:",5,100);
    new settingsItem(this, containingWidget, "overlayDuration","Duration of help overlay during play (ms):",0,5000);
    new settingsItem(this, containingWidget, "pageTurnDelay","Page turn, time to overwrite current page (ms):",0,5000);
    new settingsItem(this, containingWidget, "pageHighlightDelay","Page turn, time new page highlights:",0,5000);
    new settingsItem(this, containingWidget, "pageHighlightHeight","Page turn, highlight border width:",0,5000);

    subHeading("Debug output controls");

    new settingsItem(this, containingWidget, "debugMidiSendDetails","Debug output for each midi note sent:");
    new settingsItem(this, containingWidget, "debugMidiFileParseDetails","Debug output as file is parsed:");
    new settingsItem(this, containingWidget, "debugMidiMeasureDetails","Debug output for each midi measure parsed:");
    new settingsItem(this, containingWidget, "debugQueueInfoInterval","Debug output rate (ms) for queue info:",10,20000);

    errorSubHeading = new QLabel(containingWidget);
    errorSubHeading->setStyleSheet("font-size:" + QString::number(MUSICALPI_SETTINGS_HEADING_FONT_SIZE) + "px; font-style: Italic; color:darkRed;");
    innerLayout->addWidget(errorSubHeading);

    saveButton = new QPushButton(containingWidget);
    saveButton->setText("Save");
    innerLayout->addWidget(saveButton);
    connect(saveButton,&QPushButton::clicked, this, &settingsWidget::validateAll);

    // Find maximum prompt width in pixels
    int maxWidth = 0;
    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
        maxWidth = std::max(maxWidth,it->second->getPromptWidth());
    // Then go through and set them all to the same so the columns line up
    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
        it->second->setPromptWidth(maxWidth);
}


bool settingsWidget::validateAll()
{
    QFileInfo checkFile(values["calibrePath"]->toString() + "/" + values["calibreDatabase"]->toString());
    if(!checkFile.exists() || !checkFile.isFile())
    {
        errorSubHeading->setText("The calibre database path + file name does not find a file - check OS setup");
        values["calibrePath"]->setFocus();
        return false;
    }
    //   MORE TESTS NEEDED HERE ???

    errorSubHeading->setText("");  // Clear any errors shown

    // Step through each and update if needed/changed; since we force all to have a valid string, we can just do text compares

    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
        if(mParent->ourSettingsPtr->getSetting(it->first).toString() != it->second->toString())
            mParent->ourSettingsPtr->setSetting(it->first,it->second->toString());

    return true;
}

void settingsWidget::paintEvent(QPaintEvent *)  // This is here so we can use stylesheet styling if needed
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
