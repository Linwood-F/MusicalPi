#ifndef MIDIPLAYER_H
#define MIDIPLAYER_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0


#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QTimer>

#include "piconstants.h"

#include "tse3/MidiFile.h"
#include "tse3/Metronome.h"
#include "tse3/util/MidiScheduler.h"
#include "tse3/Transport.h"
#include "tse3/plt/Factory.h"
#include "tse3/Track.h"
#include "tse3/Part.h"
#include "tse3/Song.h"
#include "tse3/TempoTrack.h"
#include "tse3/TimeSigTrack.h"
#include "tse3/Phrase.h"
#include "tse3/Error.h"

class midiPlayer : public QWidget
{
    Q_OBJECT
public:
    midiPlayer(QWidget *parent, QString midiFile);
    ~midiPlayer();

    int lastBar;


private:
    QVBoxLayout *outLayout;
    QHBoxLayout *h1Layout, *h2Layout, *h3Layout, *h4Layout;
    QLabel      *measureInLabel;
    QLineEdit   *measureIn;
    QLabel      *measureInRange;
    QPushButton *measureGo;
    QSlider     *tempoSlider;
    QSlider     *positionSlider;
    QLabel      *tempoLabel, *tempoValueLabel;
    QLabel      *positionLabel, *positionValueLabel;
    QSlider     *volumeSlider;
    QLabel      *volumeLabel, *volumeValueLabel;
    QLabel      *songLabel;
    QLabel      *errorLabel;
    QString     _midiFile;     // Passed in file

    void doPlayingLayout();
    void openAndLoadFile();

    bool canPlay;   // Set to indicate if the song is loaded and playable
    TSE3::Clock barsClock[MUSICALPI_MAX_MEASURE];
    TSE3::MidiFileImport* mfi;
    TSE3::Song *song;
    TSE3::TimeSigTrack *tst;
    TSE3::Metronome *metronome;
    TSE3::MidiSchedulerFactory *msf;
    TSE3::MidiScheduler *sch;
    TSE3::Transport *transport;
    QTimer* timer;

private slots:
    void updateSliders();
};

#endif // MIDIPLAYER_H
