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
    int bar;
    int beat;
    int pulse;
    TSE3::MidiFileImport* mfi = new TSE3::MidiFileImport(midiFile.toStdString().c_str(), 1);
    TSE3::Metronome *metronome = new TSE3::Metronome();
    //metronome->setStatus(TSE3::Transport::Playing,0);  // Mark as not played

    TSE3::Song *song = mfi->load();

    qDebug() << "Loaded song";

    qDebug() << "Last clock from mfi  / ppqn = " << mfi->lastClock() / TSE3::Clock::PPQN << ":" << mfi->lastClock() % TSE3::Clock::PPQN ;

    TSE3::TimeSigTrack* tst = song->timeSigTrack();

    qDebug() << "Last clock from song / ppqn = " << song->lastClock() / TSE3::Clock::PPQN << ":" << song->lastClock() % TSE3::Clock::PPQN ;

    tst->barBeatPulse(mfi->lastClock().pulse(), bar, beat, pulse);
    qDebug() << "Last mfi's  TimeSigTrack::barBeatPulse=(" << bar << "," << beat << "," << pulse << ")";

    tst->barBeatPulse(song->lastClock().pulse(), bar, beat, pulse);
    qDebug() << "Last song's TimeSigTrack::barBeatPulse=(" << bar << "," << beat << "," << pulse << ")";

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
