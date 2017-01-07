#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QFont>
#include <QDebug>

// For TSE3

#include "tse3/Error.h"
#include "tse3/Transport.h"
#include "tse3/plt/Factory.h"
#include "tse3/util/MidiScheduler.h"
#include "tse3/TSE3MDL.h"
#include "tse3/TSE2MDL.h"
#include "tse3/MidiFile.h"
#include "tse3/Transport.h"
#include "tse3/Song.h"
#include "tse3/Track.h"
#include "tse3/TSE3.h"
#include "tse3/Error.h"
#include "tse3/Metronome.h"
#include <fstream>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef TSE3_WITH_OSS
#include "tse3/plt/OSS.h"
#endif
#ifdef TSE3_WITH_ALSA
#include "tse3/plt/Alsa.h"
#endif
#ifdef TSE3_WITH_ARTS
#include "tse3/plt/Arts.h"
#endif
#include <unistd.h> // for usleep


class aboutWidget : public QLabel
{
    Q_OBJECT
public:
    aboutWidget(QWidget *parent = 0);
    void play();
};

#endif // ABOUTWIDGET_H
