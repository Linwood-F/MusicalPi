#ifndef MIDIPLAYERV2THREAD_H
#define MIDIPLAYERV2THREAD_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>

#include <alsa/asoundlib.h>

class midiPlayerV2;
class MainWindow;

class midiplayerV2Thread : public QThread
{
    Q_OBJECT

public:
    midiplayerV2Thread(midiPlayerV2 *parent, MainWindow* mp);
    ~midiplayerV2Thread();
    void play(int startAtMeasure, int volumeScale, int tempoScale);
    void stop();
    void getQueueInfo();
    bool openSequencerInitialize();

    unsigned int startAtTick;     // What tick are we starting from (this is an offset so the queue starts at zero)

    // These come from the queue info call and are also visible to the update routine in the caller
    unsigned int currentQueueTick;         // Queue clock tick
    unsigned int currentQueueEventCount;   // Count of events in queue
    unsigned int currentMeasure;           // Calculated from queue tick
    unsigned int currentQueueTempo;        // as uSec per tick
    unsigned int outputFree;               // Queue free event space
    unsigned int outputSize;               // Queue Actual Size
    unsigned int currentIsRunning;         // This is defined as "status bits" plural - haven't found definition

    // ALSA connection/queue working data structures
    snd_seq_t* handle;
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
    QMutex mutex;                  // Coordinate access to thread
    QWaitCondition condition;
    enum {none, abort, startPlay, playing, playWaiting, stopPlay} requestType;
    QString errorEncountered;  // Does this need to be somehow linked to parent?
    midiPlayerV2 * ourParent;    // can't define as explicit pdfdocument due to header load sequences
    MainWindow* mParent;
    QTimer *workTimer;         // Periodic time interval when we don't have anything to do (but need to check frequently)
    QTimer *queueInfoDebug;    // Only used if we are debugging queue info periodically
    int lastTickProcessed;
    void drainQueue();
    void sendAllOff();
    // Parameters from start play call
    int m_startAtMeasure;
    int m_volumeScale;
    int m_tempoScale;
    bool debugMidiSendDetails;
};

#endif // MIDIPLAYERV2THREAD_H
