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
#include "midiplayerv2thread.h"

#include <alsa/asoundlib.h>
#include <string>

class midiplayerV2Thread;

class midiPlayerV2 : public QWidget
{
    Q_OBJECT
public:
    midiPlayerV2(QWidget *parent, QString midiFile, QString titleName);
    ~midiPlayerV2();

    unsigned int lastMeasure;
    typedef struct
    {
        snd_seq_event_t snd_seq_event;
        int measureNum;
        bool containsTempo;   // implies we may need to scale velocity before sending
        bool containsNoteOn;  // implies we may need to scale tempbefore sending
        int ticksPerMeasure;
        int uSecPerTick;
    } playableEvent_t;
    std::map<int, playableEvent_t> events;  // sparese structure for only playable (at least sendable) items
    int overallTicksPerQuarter;  // from midi file header
    QString errorEncountered;  // Blank is no error, otherwise a fatal error to appear on player


private:
    QVBoxLayout *outLayout;
    QGridLayout *gridLayout;
    QLabel      *helpLabel;
    QLabel      *measureInLabel;
    QLineEdit   *measureIn;
    QLabel      *measureNowAt;
    QLabel      *measureMax;
    QPushButton *measureGo;
    QSlider     *tempoSlider;
    QSlider     *positionSlider;
    QLabel      *tempoLabel;
    QLabel      *tempoValueLabel;
    QLabel      *positionLabel;
    QLabel      *positionValueLabel;
    QSlider     *volumeSlider;
    QLabel      *volumeLabel;
    QLabel      *volumeValueLabel;
    QLabel      *songLabel;
    QLabel      *errorLabel;

    QString     midiFile;     // Passed in file

    void doPlayingLayout();
    bool openAndLoadFile();
    bool go();
    void closeEvent(QCloseEvent*);
    bool parseFileForPlayables();
    void startOrStopUpdateSliderTimer(bool start);
    QString guessSpelling(int note, int keySigNum);

    bool canPlay;   // Set to indicate if the song is loaded and playable

    int keySig; // Last encountered key signature while scanning file (as coded in midi)
    const int keySig_offset = 7; // Add to key signature to index our name array (-7 -> 0)
    bool keySigMajor; // Was last key signature minor or magor
    const QString keySigs[15] = {"Cb","Gb","Db","Ab","Eb","Bb","F","C","G","D","A","E","B","F#","C#"};

    MidiFile mfi;  // Structure the library creates on read (we'll only do one at a time)

    QTimer* timer;  // Used to keep sliders updated -- should this be dynamic instead now??

    // Used in the parse only -0 not used in play
    unsigned int runningMeasureNumber;
    unsigned int runningTempoAsuSec;
    unsigned int runningTempoAsQPM;
    unsigned int runningTimeNumerator;
    unsigned int runningTimeDenominator;
    unsigned int runningMeasureStartTick;
    #define runningTicksPerMeasure (overallTicksPerQuarter * 4 * runningTimeNumerator / runningTimeDenominator)
    #define runninguSecPerTick ((1000000*60)/(overallTicksPerQuarter * runningTempoAsQPM))

    // Value of sliders -- ??? do these need to be realized as opposed to coming from slider?
    unsigned int tempoScale;      // integral percentage, 100 = as written, 50 = half speed, 150 = twice speed
    unsigned int velocityScale;   // integral percentage (e.g. 50 = half, 150 = half-again volume as written)

    midiplayerV2Thread* playThread;

private slots:
    void updateSliders();
    void updateVolume(int);
    void updateTempo(int);

signals:
    void requestToClose();
};

#endif // MIDIPLAYER2_H
