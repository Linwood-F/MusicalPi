// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QFileInfo>
#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>
#include <QPushButton>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>

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

#define Heading(prompt) \
    { \
        QLabel* l = new QLabel(containingWidget); \
        l->setText(prompt); \
        l->setAlignment(Qt::AlignCenter);  \
        l->setProperty("Heading",true);  \
        innerLayout->addWidget(l); \
    }

#define subHeading(prompt) \
    { \
        QLabel* l = new QLabel(containingWidget); \
        l->setText(prompt); \
        l->setAlignment(Qt::AlignLeft);  \
        l->setProperty("SubHeading",true);  \
        innerLayout->addWidget(l); \
    }

settingsWidget::settingsWidget(QWidget* p, MainWindow* mp) : QWidget(p)
{
    // Basic UI to provide per-field range checking and per-screen validation prior to updates

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

    new settingsItem(this, containingWidget, "calibrePath","Path to calibre library folder:","Path");

    new settingsItem(this, containingWidget, "calibreDatabase","Database file name (only):", "SettingStr");
    new settingsItem(this, containingWidget, "calibreMusicTag","Calibre tag for music items:","SettingStr");
    new settingsItem(this, containingWidget, "calibreListPrefix","Calibre tag for play lists items:","SettingStr");

    subHeading("Embedded Midi Player (only type '.mid' accompany PDF's)");

    new settingsItem(this, containingWidget, "midiPort","Port:",0,255);
    new settingsItem(this, containingWidget, "ALSAMidiQuashResetAll","Prevent sending Reset-All:");
    new settingsItem(this, containingWidget, "midiInitialVelocityScale","Midi Player default volume %:",10,200);
    new settingsItem(this, containingWidget, "midiInitialTempoScale","Midi Player default tempo %:",10,200);

    subHeading("Display and Page Turn Controls");

    new settingsItem(this, containingWidget, "pageTurnTipOverlay","Show instructional overlay each play:");
    new settingsItem(this, containingWidget, "forceOnboardKeyboard","Force dbus call to show OnBoard keyboard:");
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
    new settingsItem(this, containingWidget, "debugSettingsAsLoaded","Debug: output settings as read/written:");

    errorSubHeading = new QLabel(containingWidget);
    errorSubHeading->setProperty("ErrorMessage",true);
    innerLayout->addWidget(errorSubHeading);

    saveButton = new QPushButton(containingWidget);
    saveButton->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    saveButton->setText("Save");
    innerLayout->addWidget(saveButton);

    connect(saveButton,&QPushButton::clicked, this, &settingsWidget::validateAll);

    this->show(); // necessary for the calculations below to work

    // Find maximum prompt width in pixels
    int maxWidth = 0;
    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
        maxWidth = std::max(maxWidth,it->second->getPromptWidth());
    qDebug() << "Using prompt width of " << maxWidth;
    // Then go through and set them all to the same so the columns line up
    for(rowMap_t::iterator it = values.begin(); it != values.end(); it++)
        it->second->setPromptWidth(maxWidth * 1.05);  // The extra % is to give a bit of room for odd fonts, etc.
    // Mate flashes onboard if you don't do this, it needs to be explicitly not implicitly called
    if(mParent->ourSettingsPtr->getSetting("forceOnboardKeyboard").toBool())
    {
        QDBusMessage show = QDBusMessage::createMethodCall("org.onboard.Onboard","/org/onboard/Onboard/Keyboard","org.onboard.Onboard.Keyboard","Show");
        QDBusConnection::sessionBus().send(show);
    }
}

bool settingsWidget::validateAll()
{
    // This routine should validate the overall settings (i.e. those which are inter-connected)
    // before saving the data. As written there is not a lot added (yet).

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
