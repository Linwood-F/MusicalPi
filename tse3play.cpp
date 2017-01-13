// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

// Desired =
//    play (resume or start at beginning as appropriate)
//    stop (end, resets to beginning)
//    pause (toggles to resume)
//    Rewind
//    Back ?? measures
//    Goto measure n
//    Volume slider
//    Tempo slider
#include "piconstants.h"
#include "tse3play.h"

tse3play::tse3play(QString _midiFile)
{
    midiFile = _midiFile;
}

tse3play::~tse3play()
{

}

void tse3play::go()
{

    int bar, beat, pulse;  // Used to find where measures start/end
    int newBar, newBeat, newPulse; // Used to fill in bars

    TSE3::Clock* barsClock[MUSICALPI_MAX_MEASURE];
    int lastBar = 0;
    for(int i = 0; i<MUSICALPI_MAX_MEASURE; i++) barsClock[i] = new TSE3::Clock(0);

    TSE3::MidiFileImport* mfi = new TSE3::MidiFileImport(midiFile.toStdString().c_str(), 1);
    TSE3::Metronome *metronome = new TSE3::Metronome();

    TSE3::Song *song = mfi->load();
    qDebug() << "Loaded song";
    TSE3::TimeSigTrack* tst = song->timeSigTrack(); // used to get timing below


    for (unsigned int trk=0; trk < song->size(); trk++)
    {
        TSE3::Track* Tk = (*song)[trk];
        for (unsigned int prt=0; prt < Tk->size(); prt++)
        {
            TSE3::Part* Pt = (*Tk)[prt];

            tst->barBeatPulse(Pt->lastClock(), bar, beat, pulse);  // This gives the end, but we need to iterate to see where each measure is for go-to function
            qDebug() << "Trk=" << trk << ", Prt=" << prt << ", barBeatPulse=(" << bar << "," << beat << "," << pulse << ")";
            lastBar = std::max(bar,lastBar);

            for (unsigned int mCnt=0; mCnt < Pt->phrase()->size(); mCnt++)
            {
                TSE3::MidiEvent me = (*(Pt->phrase()))[mCnt];   // Part contains phrase which contains [] MidiEvents
                tst->barBeatPulse(me.time, newBar, newBeat, newPulse);
                tst->barBeatPulse(*barsClock[newBar],bar, beat, pulse);  // This is current setting (may be zero if not set)
                if (bar < newBar || (bar == newBar && beat > newBeat) || (bar == newBar && beat == newBeat && pulse > newPulse ) ) barsClock[newBar] = new TSE3::Clock(me.time);   // We want the first event in the bar
            }
        }
    }
    qDebug() << "Last measure = " << lastBar;

    TSE3::Plt::UnixMidiSchedulerFactory::setPreferredPlatform(TSE3::Plt::UnixMidiSchedulerFactory::UnixPlatform_Alsa);
//      TSE3::Util::StreamMidiScheduler* sch = new TSE3::Util::StreamMidiScheduler();
    TSE3::MidiSchedulerFactory msf;
    TSE3::MidiScheduler* sch = msf.createScheduler();
    TSE3::Transport* transport = new TSE3::Transport(metronome, sch);
    transport->filter()->setPort(20);



    qDebug() << "transport->filter()->maxVelocity()=" << transport->filter()->maxVelocity() <<
                ", transport->filter()->minVelocity()=" << transport->filter()->minVelocity() <<
                ", transport->filter()->timeScale()=" << transport->filter()->timeScale() <<
                ", transport->filter()->velocityScale()=" << transport->filter()->velocityScale();
    transport->filter()->setVelocityScale(10);
    transport->play(song, 0);

    int cnt = 0;
    while (transport->status() != TSE3::Transport::Resting && cnt < 10)
    {
        transport->poll();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cnt++;
        //qDebug() << "Going velocity up 10, timescale up 20";
        //transport->filter()->setVelocityScale(transport->filter()->velocityScale() + 10);
        //transport->filter()->setTimeScale(transport->filter()->timeScale() + 20);
        tst->barBeatPulse(sch->clock(), bar, beat, pulse);
        qDebug() << "Time = " << sch->clock() / TSE3::Clock::PPQN << ":" << sch->clock() % TSE3::Clock::PPQN <<
                    "TimeSigTrack::barBeatPulse=(" << bar << "," << beat << "," << pulse << ")";
    }



    transport->play(0,0);
    delete song;
    delete transport;
    delete metronome;
    delete sch;
    return;
}
