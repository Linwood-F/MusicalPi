#ifndef MIDIPLAYER2_H
#define MIDIPLAYER2_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0


#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>

#include "piconstants.h"

#include "MidiFile.h"
#include "MidiEvent.h"
#include "MidiEventList.h"
#include "MidiMessage.h"

//extern "C" {
#include <alsa/asoundlib.h>
//}
#include <string>


class midiPlayerV2 : public QWidget
{
    Q_OBJECT
public:
    midiPlayerV2(QWidget *parent, QString midiFile, QString titleName);
    ~midiPlayerV2();

    int lastBar;

private:
    QVBoxLayout *outLayout;
    QGridLayout *gridLayout;
    QLabel      *helpLabel;
    QLabel      *measureInLabel;
    QLineEdit   *measureIn;
    QLabel      *measureNowAt;
    QPushButton *measureGo;
    QSlider     *tempoSlider;
    QSlider     *positionSlider;
    QLabel      *tempoLabel, *tempoValueLabel;
    QLabel      *positionLabel, *positionValueLabel;
    QSlider     *volumeSlider;
    QLabel      *volumeLabel, *volumeValueLabel;
    QLabel      *songLabel;
    QLabel      *errorLabel;
    QString     midiFile;     // Passed in file

    void doPlayingLayout();
    void openAndLoadFile();
    void getQueueInfo();
    void go();

    bool canPlay;   // Set to indicate if the song is loaded and playable
    QString errorEncountered;  // Blank is no error, otherwise a fatal error
    void closeEvent(QCloseEvent*);
    QString guessSpelling(int note, int keySigNum, bool majorKey);
    void sendIt(snd_seq_event_t* ep_ptr);
    bool parseFileAndPlay(bool playFlag, int playAtMeasure);
    bool openSequencerInitialize();

    int keySig; // Last encountered key signature while scanning file (as coded in midi)
    const int keySig_offset = 7; // Add to key signature to index our name array (-7 -> 0)
    bool keySigMajor; // Was last key signature minor or magor
    const QString keySigs[15] = {"Cb","Gb","Db","Ab","Eb","Bb","F","C","G","D","A","E","B","F#","C#"};
    MidiFile mfi;  // Structure the library creates on read (we'll only do one at a time)
    struct measureInfo_type
    {
        // Index 0 = measure number 1
        int startTick;
        int ticksPerMeasure;
        int uSecPerTick;
        int startEventNumber;

    } measures[MUSICALPI_MAX_MEASURE];

    QTimer* timer;
    // These describe the (only) queue and context we use.
    snd_seq_t* handle;
    int queue;
    snd_seq_addr_t sourceAddress;
    snd_seq_addr_t destAddress;
    int currentTick;
    int currentEvents;
    int currentMeasure;
    int currentTempo;  // as uSec per tick
    int currentSkew;
    int currentSkewBase;
    int currentIsRunning;
    int overallTicksPerQuarter;  // from header

    int runningMeasureNumber;
    unsigned int runningTempoAsuSec;
    unsigned int runningTempoAsQPM;
    int runningTimeNumerator;
    int runningTimeDenominator;
    int runningMeasureStartTick;
#define runningTicksPerMeasure (overallTicksPerQuarter * 4 * runningTimeNumerator / runningTimeDenominator)
#define runninguSecPerTick ((1000000*60)/(overallTicksPerQuarter * runningTempoAsQPM))

    int tempoScale;      // integral percentage, 100 = as written, 50 = half speed, 150 = twice speed
    int velocityScale;   // integral percentage (e.g. 50 = half, 150 = half-again volume as written)
    int startAtTick;     // What tick are we starting from (this is an offset so the queue starts at zero)

private slots:
    void updateSliders();
    void updateVolume(int);
    void updateTempo(int);

signals:
    void requestToClose();
};

#endif // MIDIPLAYER2_H
