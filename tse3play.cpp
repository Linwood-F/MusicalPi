// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "tse3play.h"

tse3play::tse3play()
{

}

tse3play::~tse3play()
{

}

void tse3play::go()
{
      TSE3::MidiFileImport mfi("/home/ferguson/bumble_bee.mid");
      TSE3::Metronome metronome;
      TSE3::Plt::UnixMidiSchedulerFactory::setPreferredPlatform(TSE3::Plt::UnixMidiSchedulerFactory::UnixPlatform_Alsa);
      TSE3::MidiSchedulerFactory msf;
      TSE3::MidiScheduler* sch = msf.createScheduler();
      TSE3::Transport transport(&metronome, sch);
      transport.filter()->setPort(20);
      TSE3::Song *song = mfi.load();
      int cnt = 0;
      transport.play(song, 0);
      while (transport.status() != TSE3::Transport::Resting && cnt < 10)
      {
          transport.poll();
          std::this_thread::sleep_for(std::chrono::seconds(1));
          cnt++;
      }
      delete song;
      return;
}
