#ifndef OURSETTINGS_H
#define OURSETTINGS_H

#include <QSettings>
#include <QDebug>
#include <QColor>

#include "piconstants.h"

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

// Let QSettings do most of the work, but integrate with piconstants defines for defaults

class MainWindow;

class ourSettings
{
public:
    ourSettings(MainWindow* parent);
    ~ourSettings();
    void setSetting(QString key, QVariant value);
    QSettings* setPtr;
    MainWindow* mParent;

    // Variables representing values for easy access in other routines
    // See settings in constructor for documentation
    unsigned int midiPort;
    int maxCache;
    int overlayDuration;
    float overlayTopPortion;
    float overlaySidePortion;
    int pageTurnDelay;
    int pageHighlightDelay;
    int pageHighlightHeight;
    unsigned int ALSAlowWater;
    unsigned int ALSAhighWater;
    unsigned int ALSAmaxOutputBuffer;
    unsigned int ALSAqueueChunkSize;
    unsigned int ALSApacingInterval;
    bool ALSAMidiQuashResetAll;
    bool debugMidiSendDetails;
    bool debugMidiFileParseDetails;
    bool debugMidiMeasureDetails;
    unsigned int debugQueueInfoInterval;
    unsigned int midiInitialVelocityScale;
    unsigned int midiInitialTempoScale;
    unsigned statusUpdateRate;
    QString calibrePath;
    QString calibreDatabase;
    QString calibreMusicTag;
    float logoPct;
    int pageBorderWidth;

};

#endif // OURSETTINGS_H
