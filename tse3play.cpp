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
      TSE3::MidiFileImport mfi(midiFile.toStdString().c_str());
      TSE3::Metronome *metronome = new TSE3::Metronome();
      metronome->setStatus(TSE3::Transport::Playing,0);  // Mark as not played
      TSE3::Song *song = mfi.load();
      qDebug() << "Song loaded, lastClock beat = " << song->lastClock().beat() << ", pulse = " << song->lastClock().pulse();
      int cnt = 0;
//      TSE3::Plt::UnixMidiSchedulerFactory::setPreferredPlatform(TSE3::Plt::UnixMidiSchedulerFactory::UnixPlatform_Alsa);
      TSE3::Util::StreamMidiScheduler* sch = new TSE3::Util::StreamMidiScheduler();
//      TSE3::MidiSchedulerFactory msf;
//      TSE3::MidiScheduler* sch = msf.createScheduler();
      TSE3::Transport* transport = new TSE3::Transport(metronome, sch);
      transport->filter()->setPort(20);
      qDebug() << "transport->filter()->maxVelocity()=" << transport->filter()->maxVelocity() <<
                  ", transport->filter()->minVelocity()=" << transport->filter()->minVelocity() <<
                  ", transport->filter()->timeScale()=" << transport->filter()->timeScale();
      TSE3::Mixer* mix = new TSE3::Mixer(sch->numPorts(),transport);
      for(unsigned int pt=0; pt < sch->numPorts(); pt++)
      {
          TSE3::MixerPort* mp = (*mix)[pt];
          for(unsigned int ch=0; ch < 16; ch++)
          {
              qDebug() << "For port[" << pt << "] volume = " << (*mix)[pt]->volume() << ", channel[" << ch << "] volume = " << (*mp)[ch]->volume();
              (*mp)[ch]->setVolume(10);

          }
      }

      transport->play(song, 0);
      while (transport->status() != TSE3::Transport::Resting && cnt < 10)
      {
          transport->poll();
          std::this_thread::sleep_for(std::chrono::seconds(1));
          cnt++;
          qDebug() << "Going up 10";
          (*(*mix)[0])[10]->setVolume((*(*mix)[0])[10]->volume()+10);
      }
      transport->play(0,0);
      delete song;
      delete transport;
      delete metronome;
      delete sch;
      return;
}
