#include "midiplayer.h"

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0


midiPlayer::midiPlayer(QWidget *parent, QString midiFile) : QWidget(parent)
{
    qDebug() << "Entered";
    setWindowTitle("Midi Player");
    this->setWindowFlags(Qt::Window);
    _midiFile = midiFile;
    openAndLoadFile();
    doPlayingLayout();
    if(canPlay)
    {
        transport->play(song,0);
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateSliders()));
        timer->start(1000);
    }
}

midiPlayer::~midiPlayer()
{
    qDebug() << "In destructor";

    delete mfi;  // ?? what if it's not created - how do I know?
    delete song;
    delete tst;
    delete metronome;
    delete msf;
    delete sch;
    delete transport;

    qDebug() << "Exiting destructor";
}

void midiPlayer::openAndLoadFile()
{
    canPlay = false;

    // Open and see if we can find key information for file

    int bar, beat, pulse;  // Used to find where measures start/end
    int newBar, newBeat, newPulse; // Used to fill in bars

    lastBar = 0;

    try
    {
        mfi = new TSE3::MidiFileImport(_midiFile.toStdString().c_str(), 1);
    }
    catch (const TSE3::Error &e)
    {
        errorLabel->setText("Unable to import file " + _midiFile + ", Error='" + e.reason() + "'.");
        return;
    }
    try
    {
        song = mfi->load();
    }
    catch (const TSE3::Error &e)
    {
        errorLabel->setText("Unable to load file " + _midiFile + ", Error='" + e.reason() + "'.");
        return;
    }
    qDebug() << "Loaded song";
    try
    {
        tst = song->timeSigTrack(); // used to get timing below
        for (unsigned int trk=0; trk < song->size(); trk++)
        {
            TSE3::Track *Tk = (*song)[trk];
            for (unsigned int prt=0; prt < Tk->size(); prt++)
            {
                TSE3::Part *Pt = (*Tk)[prt];

                tst->barBeatPulse(Pt->lastClock(), bar, beat, pulse);  // This gives the end, but we need to iterate to see where each measure is for go-to function
                qDebug() << "Trk=" << trk << ", Prt=" << prt << ", barBeatPulse=(" << bar << "," << beat << "," << pulse << ")";
                lastBar = std::max(bar,lastBar);

                for (unsigned int mCnt=0; mCnt < Pt->phrase()->size(); mCnt++)
                {
                    TSE3::MidiEvent me = (*(Pt->phrase()))[mCnt];   // Part contains phrase which contains [] MidiEvents
                    tst->barBeatPulse(me.time, newBar, newBeat, newPulse);
                    tst->barBeatPulse(barsClock[newBar],bar, beat, pulse);  // This is current setting (may be zero if not set)
                    if (bar < newBar || (bar == newBar && beat > newBeat) || (bar == newBar && beat == newBeat && pulse > newPulse ) ) barsClock[newBar] = TSE3::Clock(me.time);   // We want the first event in the bar
                }
                delete Pt;
            }
            delete Tk;
        }
        qDebug() << "Last measure = " << lastBar;
    }
    catch (const TSE3::Error &e)
    {
        errorLabel->setText("Failure looking for bar positions, Error='" + QString::number(e.reason()) + "'.");
        return;
    }
    try
    {
        metronome = new TSE3::Metronome();
        TSE3::Plt::UnixMidiSchedulerFactory::setPreferredPlatform(TSE3::Plt::UnixMidiSchedulerFactory::UnixPlatform_Alsa);
        msf = new TSE3::MidiSchedulerFactory();
        sch = msf->createScheduler();
        transport = new TSE3::Transport(metronome, sch);
        transport->filter()->setPort(MUSICALPI_MIDI_PORT);

        qDebug() << "transport->filter()->maxVelocity()=" << transport->filter()->maxVelocity() <<
                    ", transport->filter()->minVelocity()=" << transport->filter()->minVelocity() <<
                    ", transport->filter()->timeScale()=" << transport->filter()->timeScale() <<
                    ", transport->filter()->velocityScale()=" << transport->filter()->velocityScale();
        transport->filter()->setVelocityScale(80);
    }
    catch (const TSE3::Error &e)
    {
        errorLabel->setText("Failure creating transport and such, Error='" + QString::number(e.reason()) + "'.");
        return;
    }

    qDebug() << "Transport all set, ready to play";
    canPlay = true;
}

void midiPlayer::doPlayingLayout()
{
    outLayout = new QVBoxLayout(this);

    h1Layout = new QHBoxLayout();
    h2Layout = new QHBoxLayout();
    h3Layout = new QHBoxLayout();
    h4Layout = new QHBoxLayout();

    qDebug() << "Layouts created";

    outLayout->addLayout(h1Layout);
    outLayout->addLayout(h2Layout);
    outLayout->addLayout(h3Layout);
    outLayout->addLayout(h4Layout);

    measureGo = new QPushButton("Start Playing",this);
    measureInLabel = new QLabel(this);
    measureInLabel->setText("Go to measure number: ");
    measureIn = new QLineEdit("1",this);
    measureInRange = new QLabel(this);
    measureInRange->setText("1-" + QString::number(lastBar));

    h1Layout->addWidget(measureGo);
    h1Layout->addWidget(measureInLabel);
    h1Layout->addWidget(measureIn);
    h1Layout->addWidget(measureInRange);

    qDebug() << "Goto measure setup done";

    tempoLabel = new QLabel("Tempo %: ",this);
    tempoSlider = new QSlider(Qt::Horizontal,this);
    tempoSlider->setMaximum(500);  // I can't find a variable for this
    tempoSlider->setMinimum(1);
    tempoValueLabel = new QLabel("???",this);
    h2Layout->addWidget(tempoLabel);
    h2Layout->addWidget(tempoSlider);
    h2Layout->addWidget(tempoValueLabel);

    qDebug() << " Tempo Slider setup done";

    positionLabel = new QLabel("Position", this);
    positionSlider = new QSlider(Qt::Horizontal,this);
    positionSlider->setMaximum(lastBar);
    positionSlider->setMinimum(1);
    positionSlider->setValue(1);
    positionValueLabel = new QLabel("???",this);
    h3Layout->addWidget(positionLabel);
    h3Layout->addWidget(positionSlider);
    h3Layout->addWidget(positionValueLabel);
    qDebug() << "Position Slider setup done";

    volumeLabel = new QLabel("Volume %", this);
    volumeSlider = new QSlider(Qt::Horizontal,this);
    volumeValueLabel = new QLabel("???",this);
    volumeSlider->setMaximum(200);
    volumeSlider->setMinimum(1);
    h4Layout->addWidget(volumeLabel);
    h4Layout->addWidget(volumeSlider);
    h4Layout->addWidget(volumeValueLabel);
    qDebug() << "Volume Slider setup done";

    songLabel = new QLabel(_midiFile,this);
    outLayout->addWidget(songLabel);
    errorLabel = new QLabel("",this);
    outLayout->addWidget(errorLabel);
}

void midiPlayer::updateSliders()
{
    transport->poll();  // Must call this frequently to keep data going
    qDebug() << "Updating sliders";
    int bar, beat, pulse;
    tempoSlider->setValue(transport->filter()->timeScale());
    tempoValueLabel->setText(QString::number(tempoSlider->value()) + " %");
    volumeSlider->setValue(transport->filter()->velocityScale());
    volumeValueLabel->setText(QString::number(volumeSlider->value()) + " %");
    tst->barBeatPulse(sch->clock(), bar, beat, pulse);
    positionSlider->setValue(bar);
    positionValueLabel->setText(QString::number(bar));
    qDebug() << "Exiting slider update";
    int status = transport->status();
    switch(status)
    {
    case TSE3::Transport::Resting:
        errorLabel->setText("Song is resting (done or not started)");
        break;
    case TSE3::Transport::Playing:
        errorLabel->setText("Song is playing");
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
