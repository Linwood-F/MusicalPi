// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include "oursettings.h"
#include <QDebug>
#include <QSettings>

ourSettings::ourSettings(MainWindow* parent)
{
    qDebug()<< "Entered";
    mParent = parent;
    setPtr = new QSettings("Linwood Ferguson","MusicalPi",(QObject*)parent);
    qDebug() << "Settings file=" << setPtr->fileName() << ", Format=" << setPtr->format() << ", status=" << setPtr->status();
    getSaveValues();
}

void ourSettings::getSaveValues()
{
    // This initializes the file if it's not extant, or initializes individual settings
    // (e.g. during an upgrade) if they are not in the existing file.  It really need be called
    // only once but it's fast and easy to call each run (defaults only apply if not defined in the config)
    // *** Short version is system defaults are here -- delete config and run to reset defaults ****

    setPtr->setValue("midiPort",setPtr->value("midiPort",20));
    setPtr->setValue("calibrePath",setPtr->value("calibrePath","/mnt/lef/t/Calibre Library")); // Path must exist in OS ahead of time
    setPtr->setValue("calibreDatabase",setPtr->value("calibreDatabase","metadata.db").toString());             // Database file name
    setPtr->setValue("calibreMusicTag",setPtr->value("calibreMusicTag","music").toString());                  // Tag must be setup in Calibre
    setPtr->setValue("calibreListPrefix",setPtr->value("calibreListPrefix","musicList_").toString());     // Prefix for any lists we maintain
    setPtr->setValue("logoPct",setPtr->value("logoPct",20).toInt());                                   // How wide should logo be on non-play window
    setPtr->setValue("pageBorderWidth",setPtr->value("pageBorderWidth",10).toInt());                          // Border around pages
    setPtr->setValue("forceOnboardKeyboard",setPtr->value("forceOnboardKeyboard",true).toBool());  // should we do a dbus command to bring up onboard?

    // This one can be smaller than calculated below, but is set to allow twice the number of pages
    // ahead of, and the same number behind, the max "up" display, plus some slop, so page forward/back works.
    // The 2:1 ratio of ahead to behind is hard coded into the cache range calculation routine
    // Large impact if larger; if "up" changes and this gets too large it can be smaller and will
    // only really impact performance when paging through with very high "up" size.

    setPtr->setValue("maxCache",setPtr->value("maxCache",(4 * (MUSICALPI_MAXROWS * MUSICALPI_MAXCOLUMNS) + 3)).toInt());

    // Duration and sizing of "where to touch" overlay hint went switched to play mode

    setPtr->setValue("overlayDuration",setPtr->value("overlayDuration",3000).toInt());
    setPtr->setValue("overlayTopPortion",setPtr->value("overlayTopPortion",20).toInt());
    setPtr->setValue("overlaySidePortion",setPtr->value("overlaySidePortion",40).toInt());

    // During page turns, how long does it delay for the turn, highlights, etc. and cosmetics

    setPtr->setValue("pageTurnDelay",setPtr->value("pageTurnDelay",3200).toInt());
    setPtr->setValue("pageHighlightDelay",setPtr->value("pageHighlightDelay",1500).toInt());
    setPtr->setValue("pageHighlightHeight",setPtr->value("pageHighlightHeight",10).toInt());
    setPtr->setValue("pageTurnTipOverlay",setPtr->value("pageTurnTipOverlay",true).toBool());

    // Because we might not always deterimned pagesize, this allows you to override it.
    // Normally set to zero, but if needed set this to the full resolution of the window, though it could be smaller

    setPtr->setValue("fullPageWidth",setPtr->value("fullPageWidth",0).toInt());
    setPtr->setValue("fullPageHeight",setPtr->value("fullPageHeight",0).toInt());

    // ALSA Interface and player tuning
    setPtr->setValue("ALSAMidiQuashResetAll",setPtr->value("ALSAMidiQuashResetAll",true).toBool()); // Should we above sending ResetAll's (some devices may not handle, or may miss the first notes)
    setPtr->setValue("midiInitialVelocityScale",setPtr->value("midiInitialVelocityScale",20).toUInt()); // How loud do we play by default (% of full)
    setPtr->setValue("midiInitialTempoScale",setPtr->value("midiInitialTempoScale",100).toUInt());   // How fast do we play by default (% of full)

    // Debugging control

    setPtr->setValue("debugMidiSendDetails",setPtr->value("debugMidiSendDetails",false).toBool());      // Should midi outputs be itemized
    setPtr->setValue("debugMidiFileParseDetails",setPtr->value("debugMidiFileParseDetails",false).toBool()); // Should parse of file be itemized
    setPtr->setValue("debugMidiMeasureDetails",setPtr->value("debugMidiMeasureDetails",false).toBool());   // During file parse should measure contains be itemized
    setPtr->setValue("debugQueueInfoInterval",setPtr->value("debugQueueInfoInterval",500).toUInt());      // How often should queue status debugging be written (msec)

    // Note this one is special as it caches the value for later use
    setPtr->setValue("debugSettingsAsLoaded",debugSettingsAsLoaded = setPtr->value("debugSettingsAsLoaded",false).toBool());      // Show each settings file entry as loaded or written
}
ourSettings::~ourSettings()
{
    if(setPtr!=NULL) setPtr->sync();
    DELETE_LOG(setPtr);
}

QVariant ourSettings::getSetting(QString key)
{
    QVariant v;
    v = setPtr->value(key);
    if(debugSettingsAsLoaded) qDebug() << "Returning for " << key << " value = " << v.toString();
    return v;
}

QVariant ourSettings::setSetting(QString key, QVariant value)
{
    setPtr->setValue(key,value);
    if(debugSettingsAsLoaded) qDebug() << "Saving value for key " << key << ", value=" << value;
    setPtr->sync();
    return value;  // Just in case it's syntactically easier to use the return as output
}
