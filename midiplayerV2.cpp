// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QStyleOption>
#include <QPainter>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>

#include "midiplayerV2.h"
#include "mainwindow.h"
#include "oursettings.h"
#include "piconstants.h"
#include "midiplayerv2thread.h"

// These support the MidiFile library routine
#include "MidiFile.h"
#include "MidiEvent.h"
#include "MidiEventList.h"
#include "MidiMessage.h"

midiPlayerV2::midiPlayerV2(MainWindow *parent, QString _midiFile, QString _titleName) : QWidget((QWidget*)parent)
{
    // This routine handles reading the file (well, it calls a library) and UI during play, but
    // depends on an independent thread to actually do the playing and interact with ALSA.

    // Note that the volume and tempo slider, as of this implementation, is active only when stopped.
    // That's because the effect of these is baked into the midi commands sent to the alsa sequencer,
    // so there is a possibly large amount of time before which a change purely in the sliders would be heard.
    // It may be possible to starve the queue, keeping it just barely fed with a measure or so ahead, so
    // slider changes would be quick, but this has not been tried.

    qDebug() << "Entered";
    setWindowTitle("Midi Player - " + _titleName);
    errorEncountered = "";  // Once set this cannot be unset in this routine - close and open again
    this->setWindowFlags(Qt::Window|Qt::Dialog);
    mParent = parent;
    playThread = NULL;
    mfi = new MidiFile();  // Store on the heap so we can delete it when done
    midiFile = _midiFile;
    doPlayingLayout();     // prepare screen
    volumeSlider->setValue(mParent->ourSettingsPtr->getSetting("midiInitialVelocityScale").toInt());  // Set initial values
    tempoSlider->setValue(mParent->ourSettingsPtr->getSetting("midiInitialTempoScale").toInt());
    canPlay = openAndLoadFile();
    if(canPlay) playThread = new midiplayerV2Thread(this,mParent);
    if(canPlay) canPlay = playThread->openSequencerInitialize();  //Initialize sequencer now (we'll use queue/port/etc in the parse as well as play)
    if(canPlay) canPlay = parseFileForPlayables();
    DELETE_LOG(mfi);   // Free up some resources
    updatePlayStatus();   // This may display an error if we couldn't do the things above
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updatePlayStatus()));
    timer->start(MUSICALPI_MIDIPLAYER_STATUSUPDATERATE);
    // User now starts play with button (or not).
}

midiPlayerV2::~midiPlayerV2()
{
    DELETE_LOG(playThread);
    DELETE_LOG(timer);
    DELETE_LOG(mfi);
}

bool midiPlayerV2::parseFileForPlayables() // Only build map, no actual play in this, only run once.
{
    const int textNames_count = 9;
    const QString textNames[9] = {"not valid <","Text Event","Copyright","Sequence/Track Name","Instrument Name","Lyric","Marker","Cue Point","not valid >"};
    // Default if not specified is 120 beats per minute (here QPM) and 4/4 time, and 480 ticks per beat (per quarter)
    // Set these here in case we get a file with no time/tempo signatures
    runningMeasureNumber = 1;
    runningDisplayedMeasureNumber = 0;  // Will indicate not seen (yet)
    runningTempoAsuSec = 0;
    runningTempoAsQPM = 120;
    runningTimeNumerator = 4;
    runningTimeDenominator = 4;
    runningMeasureStartTick = 0;

    MidiEvent *ptr;

    for (int thisEvent = 0; thisEvent < mfi->getEventCount(0); thisEvent++)  // Always starting at zero, just don't send unless we need to
    {
        QString midiDataText;  // Cumulative debug output for this event
        ptr = &((*mfi)[0][thisEvent]);    // Point to an event
        snd_seq_event_t ep;  // Set up empty event
        snd_seq_ev_clear(&ep);
        snd_seq_ev_schedule_tick(&ep, playThread->queue, 0, ptr->tick);  // 3rd parameter 0=absolute, <>0 = relative
        ep.source = playThread->sourceAddress;
        ep.dest = playThread->destAddress;
        // Note we don't touch the event[thisEvent] unless we need it as it's a sparse map, not all thisEvent values go into it
        // Advance running count to next measure
        while ((unsigned int)ptr->tick >= runningMeasureStartTick + runningTicksPerMeasure)
        {
            if(mParent->ourSettingsPtr->getSetting("debugMidiMeasureDetails").toBool())
                qDebug() << "Finished measure " << runningMeasureNumber
                         << ", start tick = " << runningMeasureStartTick
                         << ", ticks per measure = " << runningTicksPerMeasure
                         << ", uSec per tick = " << runninguSecPerTick;
            runningMeasureNumber++;   // Step in whole measures and increments until we are inside a measure
            runningMeasureStartTick += runningTicksPerMeasure;
        }
        // Here we deal with all possible midi events (likely there are some we are missing and need to add)
        if(ptr->isAftertouch()) // i.e. polyphonic key pressure
        {
            midiDataText = "Aftertouch channel " + QString::number(ptr->getChannel()) + " note " + QString::number(ptr->getP1()) + " to value " + QString::number(ptr->getP2());
            snd_seq_ev_set_keypress(&ep, ptr->getChannel(), ptr->getP1(), ptr->getP2());
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isPressure()) // channel aftertouch
        {
            midiDataText = "Pressure channel "   + QString::number(ptr->getChannel()) + " to value " + QString::number(ptr->getP1());
            snd_seq_ev_set_chanpress(&ep, ptr->getChannel(), ptr->getP1());
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
       }
        else if(ptr->isController())
        {
            bool sendFlag = true;
            QString ctrlr = "Controller " + QString::number(ptr->getP1());  // default use number
            switch (ptr->getP1()) // controller is
            {
                case   7: ctrlr = "Channel volume"; break;
                case  10: ctrlr = "Pan"; break;
                case  64: ctrlr = "Damper pedal"; break;
                case  66: ctrlr = "Sustenuto"; break;
                case  91: ctrlr = "Effects 1 Depth"; break;
                case 121: ctrlr = "Reset all";
                          if(mParent->ourSettingsPtr->getSetting("ALSAMidiQuashResetAll").toBool()) sendFlag=false;
                          break;
            }

            midiDataText = ctrlr + " on channel " + QString::number(ptr->getChannel()) + " to value " + QString::number(ptr->getP2());
            if(sendFlag)  // Send these (mostly pedals)
            {
                snd_seq_ev_set_controller(&ep, ptr->getChannel(), ptr->getP1(), ptr->getP2());
                events[thisEvent].measureNum = runningMeasureNumber;
                events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
                events[thisEvent].containsTempo = false;
                events[thisEvent].containsNoteOn = false;
                events[thisEvent].snd_seq_event = ep;
            }
        }
        else if(ptr->isEndOfTrack())  midiDataText = "EndOfTrack";  // No play action needed this is info only
        else if(ptr->isNoteOn())
        {
            midiDataText = "NoteOn " + guessSpelling(ptr->getKeyNumber(),keySig) + ", duration=" +  QString::number(ptr->getTickDuration());
            snd_seq_ev_set_noteon(&ep, ptr->getChannel(), ptr->getKeyNumber(), ptr->getVelocity());
            ep.data.note.duration = 0;  // we aren't linking notes so there's no calculated duration
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = true;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isNoteOff())  // Note the underlying parse will set this with a noteon, velocity=0
        {
            midiDataText = "NoteOff " + guessSpelling(ptr->getKeyNumber(),keySig);
            snd_seq_ev_set_noteoff(&ep, ptr->getChannel(), ptr->getKeyNumber(), 0);
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isPatchChange())
        {
            midiDataText = "Patch Change channel " + QString::number(ptr->getChannel()) + " to " + QString::number(ptr->getP1());
            snd_seq_ev_set_pgmchange(&ep, ptr->getChannel(), ptr->getP1());
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isPitchbend())
        {
            int bend = ptr->getP2()<<7;
            bend = (bend << 7) | ptr->getP1();
            midiDataText = "Pitchbend adjust " + QString::number(bend);
            snd_seq_ev_set_pitchbend(&ep, ptr->getChannel(), bend);
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = false;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isTempo())
        {
            runningTempoAsuSec = ptr->getP3();
            runningTempoAsuSec = (runningTempoAsuSec << 8) | ptr->data()[4];
            runningTempoAsuSec = (runningTempoAsuSec << 8) | ptr->data()[5];
            runningTempoAsQPM = (unsigned int)(60.0/runningTempoAsuSec*1000000.0 + 0.5);
            midiDataText = "Tempo is " + QString::number(runningTempoAsQPM) + " QPM, or " + QString::number(runningTempoAsuSec) + " uSec per quarter note, " + QString::number(runninguSecPerTick) + " per tick";
            ep.type = SND_SEQ_EVENT_TEMPO;
            ep.data.queue.queue = playThread->queue;
            ep.data.queue.param.value = runningTempoAsuSec;
            ep.dest.client = SND_SEQ_CLIENT_SYSTEM;
            ep.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
            events[thisEvent].measureNum = runningMeasureNumber;
            events[thisEvent].displayedMeasureNum = runningDisplayedMeasureNumber;
            events[thisEvent].containsTempo = true;
            events[thisEvent].containsNoteOn = false;
            events[thisEvent].snd_seq_event = ep;
        }
        else if(ptr->isTimeSignature())
        {   // Note this is ignoring the 5th and 6th bytes which are clocks per quarter (usually 24) -- not sure if we need them
            runningTimeNumerator = ptr->getP3();
            runningTimeDenominator = 2 << ((uchar)(ptr->data()[4]) - 1);
            midiDataText = "Time Sig " + QString::number(runningTimeNumerator) + "/" + QString::number(runningTimeDenominator) + ", ticks per measure = " + QString::number(runningTicksPerMeasure);
            // no time signature is sent while playing, used only in measure counts
        }
        else if(ptr->isKeySignature())
        {
            keySig = (signed char)ptr->getP3();  // Save running key
            keySigMajor = (ptr->data()[4] == 0);  // We change this to a boolean!
            midiDataText = "Key Sig " + keySigs[keySig + keySig_offset] + (keySigMajor ? "-Major" : "-Minor");
            // no key signature is sent while playing, its info only
        }
        else if(ptr->isMetaMessage())
        {
            switch (ptr->getMetaType())
            {
                case 0x21: // Midi Port
                    midiDataText = "Midi port " + QString::number(ptr->data()[4]);
                    // Need to send?
                    break;
                case 0x01: // text
                case 0x02: // Copyright
                case 0x03: // Sequence/Track name
                case 0x04: // Instrument name
                case 0x05: // Lyric
                case 0x06: // Marker  == we use this for measures, but let it fall through to get the text
                case 0x07: // Cue Point
                    {
                        int len = ptr->getP2();
                        char* msg = (char*)malloc(len+1);  // There's got to be a better way to get the string out
                        memcpy(msg, &ptr->data()[3],len);
                        msg[len]=0;
                        midiDataText = textNames[max(0,min(textNames_count - 1,ptr->getMetaType()))] + " = '" + QString(msg) + "'";
                        // None of these are sent while playing, though the Marker is checked to see if it has measure names
                        if(strncmp(msg,MUSICALPI_MEAURE_MARKER_TAG,strlen(MUSICALPI_MEAURE_MARKER_TAG)) == 0 )
                        {
                            QString num(&msg[strlen(MUSICALPI_MEAURE_MARKER_TAG)]);
                            int newMeasure = num.toInt();
                            qDebug() << "Found measure " << msg << " pulled out " << num << " and converted to " << newMeasure;
                            runningDisplayedMeasureNumber = newMeasure;
                            lastDisplayedMeasure = std::max(lastDisplayedMeasure, runningDisplayedMeasureNumber);
                        }
                        free(msg);
                    }
                    break;
                default:
                    midiDataText = "MetaMessage - undefined -- additional case item needed";
            }
        }
        else midiDataText = "Type of event record not defined - check cases - skipping ";

        QString midiDataNumbers;
        for (int j=0; j < (int)ptr->size(); j++)
        {
           if (j == 0) midiDataNumbers += "0x" + QString::number((int)(*ptr)[j],16) + " ";
           else midiDataNumbers += QString::number((int)(*ptr)[j],10) + " ";
        }
        if(mParent->ourSettingsPtr->getSetting("debugMidiFileParseDetails").toBool())
            qDebug() << "Event " << thisEvent << " at ticks " << ptr->tick << ", on track " << ptr->track << ", measure " << runningMeasureNumber
                     << ", seconds = " << mfi->getTimeInSeconds(0,thisEvent) << ", " << midiDataText << ", " << midiDataNumbers;
    }
    if(mParent->ourSettingsPtr->getSetting("debugMidiMeasureDetails").toBool())
        qDebug() << "Finished measure  " << runningMeasureNumber
                 << " (displayed = " << runningDisplayedMeasureNumber << ")"
                 << ", start tick = " << runningMeasureStartTick
                 << ", ticks per measure = " << runningTicksPerMeasure
                 << ", uSec per tick = " << runninguSecPerTick;
    lastMeasure = runningMeasureNumber;
    return true; // This will set canPlay
}

bool midiPlayerV2::openAndLoadFile()
{
    try
    {
        qDebug() << "Reading file";
        mfi->read(midiFile.toStdString());
        overallTicksPerQuarter = mfi->getTicksPerQuarterNote();
        qDebug() << "File read, tracks = " << mfi->getTrackCount() << ", TPQ=" << overallTicksPerQuarter
                 << ", Time in quarters=" << mfi->getTotalTimeInQuarters() << ", Time in seconds=" << mfi->getTotalTimeInSeconds() << ", Time in ticks=" << mfi->getTotalTimeInTicks();
        // TPQ (Ticks per quarter) is supposed to remain constant
        // QPM (Quarter notes per minute) is tempo and is set in tempo signatures and stashed in the runningTempAsQPM and uSec variables
        mfi->joinTracks();    // merge tracks to one timeline
        qDebug() << "Joined tracks";
        return true;
    }
    catch (exception &e)
    {
        qDebug() << "Failure reading file - aborting -- error = " << e.what();
        errorEncountered = "Failure reading file - aborting -- error = " + QString(e.what());
        return false;
    }
}

void midiPlayerV2::doPlayingLayout()
{
    this->setStyleSheet("QLabel       {background-color: " MUSICALPI_POPUP_BACKGROUND_COLOR "; }"
                        "midiPlayerV2 {background-color: " MUSICALPI_POPUP_BACKGROUND_COLOR "; }"
                        );
    outLayout = new QVBoxLayout(this);
    outLayout->setSpacing(15);
    gridLayout = new QGridLayout();
    gridLayout->setVerticalSpacing(15);
    outLayout->addLayout(gridLayout);

    measureGo = new QPushButton("???",this);
    connect(measureGo,&QPushButton::clicked, this, &midiPlayerV2::go);

    measureInLabel = new QLabel(this);
    measureInLabel->setText("Go to measure#: ");
    measureIn = new QLineEdit("",this);
    measureNowAt = new QLabel(this);
    measureNowAt->setText("");
    measureMax = new QLabel(this);
    measureMax->setText("");

    gridLayout->addWidget(measureGo,     0,0,1,1);
    gridLayout->addWidget(measureInLabel,0,1,1,1);
    gridLayout->addWidget(measureIn,     0,2,1,1);
    gridLayout->addWidget(measureNowAt,  0,3,1,1);
    gridLayout->addWidget(measureMax,    0,4,1,1);

    helpLabel = new QLabel("Adjust tempo and volume only when stopped");
    gridLayout->addWidget(helpLabel,1,1,1,3);

    tempoLabel = new QLabel("Tempo %: ",this);
    tempoSlider = new QSlider(Qt::Horizontal,this);
    tempoSlider->setMaximum(200);  // Is this large enough?
    tempoSlider->setMinimum(30);   // Small enough
    connect(tempoSlider,SIGNAL(valueChanged(int)),this,SLOT(updateTempo()));
    tempoValueLabel = new QLabel("???",this);
    gridLayout->addWidget(tempoLabel,     2,1,1,1);
    gridLayout->addWidget(tempoSlider,    2,2,1,1);
    gridLayout->addWidget(tempoValueLabel,2,3,1,1);

    volumeLabel = new QLabel("Volume %", this);
    volumeSlider = new QSlider(Qt::Horizontal,this);
    volumeValueLabel = new QLabel("???",this);
    volumeSlider->setMaximum(150);
    volumeSlider->setMinimum(1);
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(updateVolume()));
    gridLayout->addWidget(volumeLabel,     3,1,1,1);
    gridLayout->addWidget(volumeSlider,    3,2,1,1);
    gridLayout->addWidget(volumeValueLabel,3,3,1,1);

    errorLabel = new QLabel("",this);
    outLayout->addWidget(errorLabel);
}

void midiPlayerV2::updatePlayStatus()
{
    if(playThread==NULL) return;  // if we haven't gotten far enough to have worker don't try the below yet
    // Depend on playing thread to keep current* variables updated
    if(canPlay)
    {
        measureMax->setText(QString("of ") + QString::number(lastDisplayedMeasure < 1 ? lastMeasure : lastDisplayedMeasure));
        if (playThread->currentIsRunning)  // make go => stop
        {
            errorLabel->setText("Song is playing (stop to adjust sliders)");
            measureGo->setText(" Stop ");
            measureGo->setEnabled(true);
            tempoSlider->setEnabled(false);
            volumeSlider->setEnabled(false);
            measureNowAt->setText(QString::number(playThread->currentMeasure));
        }
        else // make go => play
        {
            errorLabel->setText("Play is stopped");
            measureGo->setText(" Play ");
            measureGo->setEnabled(true);
            tempoSlider->setEnabled(true);
            volumeSlider->setEnabled(true);
        }
    }
    else
    {
        qDebug() << "Updating sliders but can't play";
        // Disable controls
        tempoSlider->setEnabled(false);
        volumeSlider->setEnabled(false);
        measureGo->setEnabled(false);
        measureGo->setText("Error");
        // Highlight and show error
        errorLabel->setText(errorEncountered);
        errorLabel->setProperty("ErrorMessage",true);  // So we can style it
    }
    return;
}

void midiPlayerV2::updateVolume()
{
    volumeValueLabel->setText(QString::number(volumeSlider->value()));
}

void midiPlayerV2::updateTempo()
{
    tempoValueLabel->setText(QString::number(tempoSlider->value()));
}

void midiPlayerV2::go()
{
    if(!canPlay) return;
    if(!playThread->currentIsRunning)  // Hitting play from here uses measure or starts over if blank
    {
        int measureToPlay = 1;
        if(measureIn->text() != "")
        {
            unsigned int newBar = measureIn->text().toInt();  // Internally bars are reference 0 not 1; only the gui shows ref 1
            measureToPlay = std::max((unsigned int)0,std::min(lastMeasure,newBar));
        }
        qDebug() << "Go: Not playing, Starting play=true, measure= " << measureToPlay;
        playThread->play(measureToPlay, volumeSlider->value(), tempoSlider->value());
    }
    else
    {
        qDebug() << "Go: Playing, so changing to stopped, stopping queue ";
        playThread->stop();
        measureGo->setText("Play");
    }
}

QString midiPlayerV2::guessSpelling(int note, int keySigNum)
{
    // Get a reasonable spelling of the note based on key signature (e.g. give preference to flats if sig includes it, sharps if includes, etc.)
    // This is all just guesswork, so I did a lot of duplication in case it is desirable to fine tune any
    // It's only used for debugging
    int noteMod = note % 12;  // C=0
    QString Octave = "-" + QString::number(note / 12 - 1);
    switch (keySigNum)
    {
        case -7: // Cb Ab (Flats: B, E, A, D, G, C, F)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -6: // Gb Eb (Flats: B, E, A, D, G, C)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -5: // Db Bb (Flats: B, E, A, D, G)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -4: // Ab F  (Flats: B, E, A, D)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -3: // Eb C  (Flats: B, E, A)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -2: // Bb G  (Flats: B, E)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -1: // F  D  (Flats: B)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  0: // C  A
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  1: // G  E  (Sharps: F)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  2: // D  B  (Sharps: F, C)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  3: // A  F# (Sharps: F, C, G)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  4: // E  C# (Sharps: F, C, G, D)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  5: // B  G# (Sharps: F, C, G, D, A)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  6: // F# D# (Sharps: F, C, G, D, A, E)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  7: // C# A# (Sharps: F, C, G, D, A, E, B)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        default: return "Bad";
    }
}

void midiPlayerV2::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
