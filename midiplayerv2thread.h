#ifndef MIDIPLAYERV2THREAD_H
#define MIDIPLAYERV2THREAD_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QDebug>
#include <QCoreApplication>

#include <cassert>
#include "piconstants.h"
#include "midiplayerV2.h"

#include <alsa/asoundlib.h>
#include <sys/types.h>
#include <sys/syscall.h>


class midiPlayerV2;

class midiplayerV2Thread : public QThread
{
    Q_OBJECT

public:
    midiplayerV2Thread(midiPlayerV2 *parent=0);
    ~midiplayerV2Thread();
    void play(int startAtMeasure, int volumeScale, int tempoScale);
    void stop();
    void getQueueInfo();
    bool openSequencerInitialize();

    unsigned int startAtTick;     // What tick are we starting from (this is an offset so the queue starts at zero)
    unsigned int currentTick;
    unsigned int currentEvents;
    unsigned int currentMeasure;
    unsigned int currentTempo;  // as uSec per tick
    unsigned int outputFree;
    unsigned int outputRoom;
    unsigned int outputSize;
    unsigned int currentIsRunning;  // This is defined as "status bits" plural - haven't found definition

    snd_seq_t* handle;  // shared between this and thread
    int queue;
    snd_seq_addr_t sourceAddress;
    snd_seq_addr_t destAddress;

protected:
    void run() Q_DECL_OVERRIDE;

signals:

private slots:
    void gooseThread();
    void queueInfoDebugOutput();

private:
    QMutex mutex;
    QWaitCondition condition;
    int m_startAtMeasure;
    int m_volumeScale;
    int m_tempoScale;
    enum {none, abort, startPlay, playing, playWaiting, stopPlay} requestType;
    QString errorEncountered;  // Does this need to be somehow linked to parent?
    midiPlayerV2 * mParent;  // can't define as explicit pdfdocument due to header load sequences
    QTimer *workTimer;
    QTimer *queueInfoDebug;
    unsigned int outBufferSize;
    int lastTickProcessed;
    void drainQueue();
};


#endif // MIDIPLAYERV2THREAD_H
