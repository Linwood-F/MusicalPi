/*
 * @(#)tse3play.h 3.00 6 July 1999
 *
 * Copyright (c) 2000 Pete Goodliffe (pete@cthree.org)
 *
 * This file is part of TSE3 - the Trax Sequencer Engine version 3.00.
 *
 * This library is modifiable/redistributable under the terms of the GNU
 * General Public License.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING. If not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#ifndef TSE3PLAY_TSE3PLAY_H
#define TSE3PLAY_TSE3PLAY_H

#include <QString>
#include <QDebug>

#include <iostream>

#include "tse3/MidiFile.h"

#include "tse3/Metronome.h"
#include "tse3/util/MidiScheduler.h"
#include "tse3/Transport.h"
#include "tse3/plt/Factory.h"
#include "tse3/Track.h"
#include "tse3/Part.h"
#include "tse3/Song.h"
#include "tse3/Mixer.h"

#include <chrono>
#include <thread>


class tse3play
{
public:
    tse3play(QString);
    ~tse3play();
    void go();
private:
    QString midiFile;
};

#endif
