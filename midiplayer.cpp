#include "midiplayer.h"

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#define DELETE_LOG(X) if(X != NULL) { qDebug() << "Freeing " #X; delete X; }

midiPlayer::midiPlayer(QWidget *parent, QString _midiFile, QString _titleName) : QWidget(parent)
{
    qDebug() << "Entered";
    setWindowTitle("Midi Player - " + _titleName);
    errorEncountered = "";  // Once set this cannot be unset in this routine - close and open again
    this->setWindowFlags(Qt::Window|Qt::Dialog);
    midiFile = _midiFile;
    doPlayingLayout();  // This is needed even if not playing to show error
    openAndLoadFile();
    updateVolume(MUSICALPI_INITIAL_VELOCITY_SCALE);
    updateTempo(MUSICALPI_INITIAL_TIME_SCALE);
    updateSliders();  // This is really only needed for error conditions since the above won't update then
}


midiPlayer::~midiPlayer()
{
    DELETE_LOG(transport);
    DELETE_LOG(sch);
    DELETE_LOG(msf);
    DELETE_LOG(metronome);
    DELETE_LOG(song);
    DELETE_LOG(mfi);
}


void midiPlayer::openAndLoadFile()
{
    canPlay = false;

    // Open and see if we can find key information for file

    int bar, beat, pulse;  // Used to find where measures start/end
    int newBar, newBeat, newPulse; // Used to fill in bars

    lastBar = 0;
    QString step;

    try
    {
        step = "Importing file";
        mfi = new TSE3::MidiFileImport(midiFile.toStdString().c_str(), 1);
        step = "Loading song";
        song = mfi->load();
        step = "Creating time sig track";
        tst = song->timeSigTrack(); // used to get timing below
        step = "Searching for tracks and parts and bars, track = none yet.";
        for (unsigned int trk=0; trk < song->size(); trk++)
        {
            TSE3::Track *Tk = (*song)[trk];
            step = "Searching for tracks and parts and bars, trk = " + QString(trk);
            for (unsigned int prt=0; prt < Tk->size(); prt++)
            {
                TSE3::Part *Pt = (*Tk)[prt];
                step = "Searching for tracks and parts and bars, trk = " + QString(trk) + ", part = " + QString(prt);
                tst->barBeatPulse(Pt->lastClock(), bar, beat, pulse);  // This gives the end, but we need to iterate to see where each measure is for go-to function
                lastBar = std::max(bar,lastBar);
                for (unsigned int mCnt=0; mCnt < Pt->phrase()->size(); mCnt++)
                {
                    TSE3::MidiEvent me = (*(Pt->phrase()))[mCnt];   // Part contains phrase which contains [] MidiEvents
                    if(me.data.isNote())
                    {
                        tst->barBeatPulse(me.time, newBar, newBeat, newPulse);
                        qDebug() << "New midi event = " << me.time << " = [" << newBar << "," << newBeat << "," << newPulse << "], " << (me.data.isNote() ? me.data.data1 : 0);
                        tst->barBeatPulse(barsClock[newBar],bar, beat, pulse);  // This is current setting (may be zero if not set)
                        if (bar < newBar || (bar == newBar && beat > newBeat) || (bar == newBar && beat == newBeat && pulse > newPulse ) ) barsClock[newBar] = TSE3::Clock(me.time);   // We want the first event in the bar
                    }
                }
            }
        }
        for(int i=0; i < sizeof(barsClock)/sizeof(barsClock[0]); i++)
        {
            if(barsClock[i]!=0 || i == 0)
            {
                tst->barBeatPulse(barsClock[i], newBar, newBeat, newPulse);
                qDebug() << "   barsClock[" << i << "] contains " << barsClock[i] << " = [" << newBar << "," << newBeat << "," << newPulse << "]";
            }
        }
        qDebug() << "Last measure = " << lastBar;
        step = "Creating metronome";
        metronome = new TSE3::Metronome();
        step = "Setting preferred scheduler";
        TSE3::Plt::UnixMidiSchedulerFactory::setPreferredPlatform(TSE3::Plt::UnixMidiSchedulerFactory::UnixPlatform_Alsa);
        step = "Creating factory";
        msf = new TSE3::MidiSchedulerFactory();
        step = "Creating scheduler";
        sch = msf->createScheduler();
        step = "Creating transport";
        transport = new TSE3::Transport(metronome, sch);
        step = "Setting port";
        transport->filter()->setPort(MUSICALPI_MIDI_PORT);
        step = "Setting panic";
        setPanic(transport->startPanic());
        setPanic(transport->endPanic());
        qDebug() << "Adaptive lookahead is " << transport->adaptiveLookAhead() << " (setting to false, and lookahead to 512).";
        transport->setAdaptiveLookAhead(false);
        transport->setLookAhead(TSE3::Clock(512));
        qDebug() << "Lookahead = " << transport->lookAhead();
    }
    catch (const TSE3::Error &e)
    {
        qDebug() << "Error=" << TSE3::errString(e.reason());
        errorEncountered = "Failure loading and processing song, step = " + step + ", error = " + TSE3::errString(e.reason());
        return;
    }
    catch (...)
    {
        qDebug() << "Got an error we didn't catch with TSE3::error";
    }

    qDebug() << "Transport all set, ready to play";
    canPlay = true;
}

void midiPlayer::doPlayingLayout()
{
    outLayout = new QVBoxLayout(this);
    outLayout->setSpacing(15);
    gridLayout = new QGridLayout();
    gridLayout->setVerticalSpacing(15);

    qDebug() << "Layouts created";

    outLayout->addLayout(gridLayout);

    measureGo = new QPushButton("???",this);
    measureGo->setStyleSheet("padding: 2px;");
    connect(measureGo,&QPushButton::clicked, this, &midiPlayer::go);

    measureInLabel = new QLabel(this);
    measureInLabel->setText("Go to measure#: ");
    measureIn = new QLineEdit("",this);
    measureNowAt = new QLabel(this);
    measureNowAt->setText("");

    gridLayout->addWidget(measureGo,0,0,1,1);
    gridLayout->addWidget(measureInLabel,0,1,1,1);
    gridLayout->addWidget(measureIn,0,2,1,1);
    gridLayout->addWidget(measureNowAt,0,3,1,1);

    helpLabel = new QLabel("Adjust tempo and volume only when stopped");
    gridLayout->addWidget(helpLabel,1,1,1,3);

    tempoLabel = new QLabel("Tempo: ",this);
    tempoSlider = new QSlider(Qt::Horizontal,this);
    tempoSlider->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop:1 Grey);");
    tempoSlider->setMaximum(200);  // Is this large enough?
    tempoSlider->setMinimum(30);   // Small enough
    connect(tempoSlider,SIGNAL(valueChanged(int)),this,SLOT(updateTempo(int)));
    tempoValueLabel = new QLabel("???",this);
    gridLayout->addWidget(tempoLabel,2,1,1,1);
    gridLayout->addWidget(tempoSlider,2,2,1,1);
    gridLayout->addWidget(tempoValueLabel,2,3,1,1);

    volumeLabel = new QLabel("Volume %", this);
    volumeSlider = new QSlider(Qt::Horizontal,this);
    volumeSlider->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop:1 Grey); ");
    volumeValueLabel = new QLabel("???",this);
    volumeSlider->setMaximum(200);
    volumeSlider->setMinimum(1);
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(updateVolume(int)));
    gridLayout->addWidget(volumeLabel,3,1,1,1);
    gridLayout->addWidget(volumeSlider,3,2,1,1);
    gridLayout->addWidget(volumeValueLabel,3,3,1,1);
    qDebug() << "Volume Slider setup done";

    errorLabel = new QLabel("",this);
    outLayout->addWidget(errorLabel);
}

void midiPlayer::updateSliders()
{
    int bar, beat, pulse;

    if(canPlay)
    {
        qDebug() << "Updating sliders while can play";
        playStatus = transport->status();
        if(playStatus == TSE3::Transport::Playing) transport->poll();  // Must call this frequently to keep data going, only if playing
         // Don't update the slider unnecessarily as it causes the change routine to fire even if not changed

        if(tempoSlider->value() != sch->tempo())
        {
            tempoSlider->setValue(sch->tempo());
            tempoValueLabel->setText(QString::number(tempoSlider->value()) + " bpm");
        }

        if(volumeSlider->value() != transport->filter()->velocityScale())
        {
            volumeSlider->setValue(transport->filter()->velocityScale());
            volumeValueLabel->setText(QString::number(volumeSlider->value()) + " %");
        }

        // use error slot but not highlighted for status
        switch(playStatus)
        {
            case TSE3::Transport::Resting:
                errorLabel->setText("Song is resting (done or not started)");
                measureGo->setText(" Play ");
                tempoSlider->setEnabled(true);
                volumeSlider->setEnabled(true);
                break;
            case TSE3::Transport::Playing:
                errorLabel->setText("Song is playing");
                measureGo->setText(" Stop ");
                tempoSlider->setDisabled(true);
                volumeSlider->setDisabled(true);
                tst->barBeatPulse(sch->clock(), bar, beat, pulse);
                measureNowAt->setText(QString::number(bar + 1));
                break;
            case TSE3::Transport::Recording:
                errorLabel->setText("Song is recording (never should get here!!)");
                break;
            case TSE3::Transport::SynchroPlaying:
                errorLabel->setText("Song is Synchro-playing (never should get here!!)");
                break;
            case TSE3::Transport::SynchroRecording:
                errorLabel->setText("Song is Synchro-Recording (never should get here!!)");
                break;
            default:
                errorLabel->setText("Received invalid status from transport - bug");
        }
    }
    else
    {
        qDebug() << "Updating sliders but can't play";
        // Disable controls
        tempoSlider->setDisabled(true);
        volumeSlider->setDisabled(true);
        measureGo->setDisabled(true);
        measureGo->setText("Error");

        // Highlight and show error
        errorLabel->setText(errorEncountered);
        errorLabel->setStyleSheet("color: red;");
    }
}

void midiPlayer::updateVolume(int newVolume)
{
    qDebug() << "Entered";
    if(!canPlay) return;  // Shouldn't get here but just in case
    transport->filter()->setVelocityScale(newVolume);
    updateSliders();  // hasten reflection of new info
}

void midiPlayer::updateTempo(int newTempo)
{
    qDebug() << "Entered";
    if(!canPlay) return;  // Shouldn't get here but just in case
    if(sch->tempo() != newTempo)
    {
        qDebug() << "Changing temp from " << sch->tempo() << " to " << newTempo;
        sch->setTempo(newTempo, sch->clock());
    }
    updateSliders();  // hasten reflection of new info
}

void midiPlayer::go()
{
    if(canPlay && playStatus == TSE3::Transport::Resting)  // Hitting play from here uses measure or starts over if blank
    {
        qDebug() << "Entered Go and resting, switching to playing";
        TSE3::Clock newClock(0);
        if(measureIn->text() != "" )
        {
            int newBar = measureIn->text().toInt() - 1;  // Internally bars are reference 0 not 1; only the gui shows ref 1
            newBar = std::max(0,std::min(lastBar,newBar));
            newClock = barsClock[newBar];
            qDebug() << "Picked new bar " << newBar;
        }
        transport->play(song,newClock);
        if(timer==NULL)
        {
            timer = new QTimer(this);
            connect(timer, SIGNAL(timeout()), this, SLOT(updateSliders()));
            timer->start(100);
            qDebug() << "First time through, starting timer";
        }
        measureGo->setText("Stop");
    }
    else if (canPlay && playStatus == TSE3::Transport::Playing)  // Hitting stop ends play entirely but updates measure to where we were
    {
        qDebug() << "Playing, so changing to stopped";
        sch->stop();
        transport->play(0,0);
        measureGo->setText("Play");
    }
    else if (!canPlay)
    {
        qDebug() << "Can't play so doing nothing in go -- shouldn't be able to get here";
        measureGo->setDisabled(true);  // Shouldn't get here but if we do we can't play
    }
    else qDebug() << "Bad logic in Go, fell through status = " << playStatus;
}

void midiPlayer::closeEvent(QCloseEvent *event)
{
    event->ignore();  // We ignore it here and signal the caller to delete us, but do stop any playing
    if(canPlay && playStatus == TSE3::Transport::Playing) transport->play(0,0);
    emit requestToClose();
}

void midiPlayer::setPanic(TSE3::Panic *panic)
{
    panic->setMidiReset(true);
    panic->setGmReset(true);
    panic->setGsReset(true);
    panic->setXgReset(true);
}
