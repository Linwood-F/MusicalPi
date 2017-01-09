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

#include <string>
#include <list>

#include "tse3/Transport.h"

namespace TSE3_Utilities_Play
{
    /**
     * This class is used to produce the on screen VU-bar style output for
     * TSE3Play.
     *
     * It uses vt100 terminal codes to produce on screem output, using
     * colour change and cursor movement commands.
     *
     * @short   TSE3Play real time visual output
     * @author  Pete Goodliffe
     * @version 1.00
     */
    class TSE3PlayVisual : public TSE3::TransportCallback,
                           public TSE3::Listener<TSE3::TransportListener>
    {
        public:

            TSE3PlayVisual(TSE3::Transport *t, TSE3::MidiScheduler *s);
            virtual ~TSE3PlayVisual();

            /**
             * @reimplemented
             */
            virtual void Transport_MidiIn(TSE3::MidiCommand c);

            /**
             * @reimplemented
             */
            virtual void Transport_MidiOut(TSE3::MidiCommand c);

            /**
             * Call this method once before calling @ref poll to draw the
             * VU bar frame and credits. Also call @ref blankFrame before @ref
             * poll to remove the credits.
             *
             * @see blankFrame
             */
            void drawFrame();

            /**
             * Blanks the frame credits ready for a call to @ref poll.
             *
             * @see poll
             */
            void blankFrame();

            /**
             * Call this method to place a 'loading'  or similar message on
             * the screen.
             */
            void message(const std::string &message);

            /**
             * Call this before @poll to set up the position bar.
             * Otherwise the position bar will not work.
             *
             * @see poll
             */
            void setLastClock(TSE3::Clock lastClock);

            /**
             * This entry point must be called frequently. It updates the
             * on-screen display.
             *
             * Before calling poll for the first time, you must have called
             * @ref drawFrame, @ref blankFrame, and @ref setLastClock.
             */
            void poll();

            /**
             * @reimplemented
             */
            virtual void Notifier_Deleted(TSE3::Transport *);

        private:

            /**
             * Updates the text VU bars.
             */
            void updateBars();

            /**
             * Moves the text cursor to the given position.
             * After plotting at the cursor, update x and y to reflect
             * where you've left it.
             */
            void move(int x, int y);

            /**
             * Does a move and then puts char 'c'.
             */
            void moveout(int x, int y, char c);

            static const int     max = 10;  // The height of the vu bars
            TSE3::Transport     *transport;
            TSE3::MidiScheduler *scheduler;
            int                  now[16];   // channels value on screen *now*
            int                  next[16];  // channel next value
            int                  lastMsec;  // last msec bars were updated
            int                  x, y;      // text cursor pos (0,0) bottom left
            TSE3::Clock          lastClock; // end time for position bar
            int                  barPos;    // current bar position
    };

    /**
     * The TSE3Play class implements a command line file playback utility
     * using the TSE3 system.
     *
     * It can play both TSE3MDL and standard MIDI files.
     *
     * It produces graphical terminal output using the @ref TSE3PlayVisual
     * class.
     *
     * @short   A TSE3MDL/MIDI file player.
     * @author  Pete Goodliffe
     * @version 1.00
     */
    class TSE3Play : public TSE3::Listener<TSE3::MidiSchedulerListener>
    {
        public:

            TSE3Play(int argc, char *argv[]);
            virtual ~TSE3Play();
            int go();

            /**
             * @reimplemented
             */
            virtual void MidiScheduler_Stopped(TSE3::MidiScheduler *);

            /**
             * @reimplemented
             */
            virtual void Notifier_Deleted(TSE3::MidiScheduler *);

        private:

            /*
             * Command switch cunningness
             */
            struct Switch
            {
                typedef void (TSE3Play::*handler_t)(int argpos, char *argv[]);
                std::string    lng;
                std::string    srt;
                int            nargs;
                std::string    help;
                handler_t      handler;
                Switch(std::string l, std::string s, int n, std::string h,
                       handler_t hd);
            };
            std::vector <Switch> switches;

            /*
             * Command switch handlers
             */
            void handle_help(int argpos, char *argv[]);
            void handle_version(int argpos, char *argv[]);
            void handle_verbose(int argpos, char *argv[]);
            void handle_listports(int argpos, char *argv[]);
            void handle_novisual(int argpos, char *argv[]);
            void handle_noplay(int argpos, char *argv[]);
            void handle_loop(int argpos, char *argv[]);
            void handle_outmidi(int argpos, char *argv[]);
            void handle_outmidiformat0(int argpos, char *argv[]);
            void handle_outmidicompact(int argpos, char *argv[]);
            void handle_outtse3mdl(int argpos, char *argv[]);
            void handle_mapchannel(int argpos, char *argv[]);
            void handle_metronome(int argpos, char *argv[]);
            void handle_reset_midi(int argpos, char *argv[]);
            void handle_reset_gm(int argpos, char *argv[]);
            void handle_reset_gs(int argpos, char *argv[]);
            void handle_reset_xg(int argpos, char *argv[]);
            void handle_unix(int argpos, char *argv[]);
            void handle_oss(int argpos, char *argv[]);
            void handle_alsa(int argpos, char *argv[]);
            void handle_arts(int argpos, char *argv[]);
            void handle_stream(int argpos, char *argv[]);
            void handle_start(int argpos, char *argv[]);
            void handle_solo(int argpos, char *argv[]);
            void handle_sleep(int argpos, char *argv[]);
            void handle_nostop(int argpos, char *argv[]);
            void handle_echo(int argpos, char *argv[]);
            void handle_mute(int argpos, char *argv[]);
            void handle_patchesdir(int argpos, char *argv[]);
            void handle_internal(int argpos, char *argv[]);
            void handle_external(int argpos, char *argv[]);
            void handle_port(int argpos, char *argv[]);
            void handle_fast(int argpos, char *argv[]);

            static const int   _version = 100;
            static const char *_name;

            // Setup variables altered by command line parameters
            std::string       filename;
            int               verbose;
            bool              listports;
            bool              doplay;
            bool              dovisual;
            bool              doloop;
            std::string       outmidi;
            int               outmidiformat;
            bool              outmidicompact;
            std::string       outtse3mdl;
            bool              dometronome;
            bool              midi, gm, gs, xg;
            TSE3::Clock       startClock;
            int               soloTrack;
            int               usleepPeriod;
            bool              dostop;
            bool              doecho;
            enum st {Unix, OSS, Alsa, Arts, Stream} schedtype;
            std::list<size_t> muteList;
            std::string       patchesDir;
            int               port;
            bool              fastMidi;

            /**
             * Sets up the given panic object according to the variables
             * midi, gm, gs, xg.
             */
            void setPanic(TSE3::Panic *panic);

            // Play-loop variables
            bool                 more_to_come;
            TSE3::MidiScheduler *sch;
    };
}

#endif
