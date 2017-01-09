/*
 * @(#)tse3play.cpp 3.00 30 July 1999
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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE // Euch! I need this to get usleep to be included
#endif

#include "tse3play.h"

#include "tse3/plt/Factory.h"
#include "tse3/util/MidiScheduler.h"
#include "tse3/TSE3MDL.h"
#include "tse3/TSE2MDL.h"
#include "tse3/MidiFile.h"
#include "tse3/Transport.h"
#include "tse3/Song.h"
#include "tse3/Track.h"
#include "tse3/TSE3.h"
#include "tse3/Error.h"
#include "tse3/Metronome.h"
#include <fstream>
#include <cstdlib>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TSE3_WITH_OSS
#include "tse3/plt/OSS.h"
#endif

#ifdef TSE3_WITH_ALSA
#include "tse3/plt/Alsa.h"
#endif

#ifdef TSE3_WITH_ARTS
#include "tse3/plt/Arts.h"
#endif

#include <unistd.h> // for usleep
//#include <time.h> // for nanosleep

using namespace TSE3;
using namespace TSE3_Utilities_Play;

using std::cout;
using std::cerr;

namespace
{
    /**
     * Used in VT100 terminal escape sequences, for example
     * move cursor, change colour, etc.
     */
    const char escChar = char(0x1b);

    const int USE_INTERNAL_PORT    = -100;
    const int USE_EXTERNAL_PORT    = -101;
    const int USE_BEST_EFFORT_PORT = -101;
}

/*****************************************************************************
 * TSE3Play class: initialization
 ****************************************************************************/

const char *TSE3Play::_name = "tse3play";

TSE3Play::TSE3Play(int argc, char *argv[])
: verbose(0), listports(false), doplay(true), dovisual(true), doloop(false),
  outmidiformat(1), outmidicompact(false),
  dometronome(false),
  midi(false), gm(false), gs(false), xg(false),
  startClock(-1), soloTrack(-1), usleepPeriod(100),
  dostop(true), doecho(false),
  schedtype(Unix), patchesDir(""), port(USE_BEST_EFFORT_PORT), fastMidi(false)
{
    // The list of switches understood by this program

    switches.push_back(Switch("help", "h", 0, "provide help on tse3play",
                              &TSE3Play::handle_help));
    switches.push_back(Switch("version", "ver", 0, "give version no",
                              &TSE3Play::handle_version));
    switches.push_back(Switch("verbose", "v", 0,
                              "give verbose progress information",
                               &TSE3Play::handle_verbose));
    switches.push_back(Switch("list-ports", "list", 0,
                              "list available port numbers",
                              &TSE3Play::handle_listports));
    switches.push_back(Switch("novisual", "nv", 0,
                              "don't produce visual playback feedback",
                              &TSE3Play::handle_novisual));
    switches.push_back(Switch("noplay", "np", 0, "don't play input file",
                              &TSE3Play::handle_noplay));
    switches.push_back(Switch("loop", "l", 0, "set loop mode",
                              &TSE3Play::handle_loop));
    switches.push_back(Switch("out-midi", "omidi", 1,
                              "convert file to MIDI\n(filename follows switch)",
                              &TSE3Play::handle_outmidi));
    switches.push_back(Switch("out-midi-format-0", "omidi0", 1,
                              "specify MIDI file format as 0\n(default is 1)",
                              &TSE3Play::handle_outmidiformat0));
    switches.push_back(Switch("out-midi-compact", "omidicomp", 1,
                              "specify MIDI file compaction option",
                              &TSE3Play::handle_outmidicompact));
    switches.push_back(Switch("out-tse3mdl", "otse3mdl", 1,
                              "convert file to tse3mdl\n"
                              "(filename follows switch)",
                              &TSE3Play::handle_outtse3mdl));
    switches.push_back(Switch("map-channel", "map", 2,
                              "<f> <t>: map channel f to t",
                              &TSE3Play::handle_mapchannel));
    switches.push_back(Switch("metronome", "m", 0,
                              "produce a metronome tick",
                              &TSE3Play::handle_metronome));
    switches.push_back(Switch("reset", "r", 0,
                              "send a MIDI reset at playback start and stop",
                              &TSE3Play::handle_reset_midi));
    switches.push_back(Switch("gmreset", "gmr", 0,
                              "send a GM reset at playback start and stop",
                              &TSE3Play::handle_reset_gm));
    switches.push_back(Switch("gsreset", "gsr", 0,
                              "send a GS reset at playback start and stop",
                              &TSE3Play::handle_reset_gs));
    switches.push_back(Switch("xgreset", "xgr", 0,
                              "send a XG reset at playback start and stop",
                              &TSE3Play::handle_reset_xg));
    switches.push_back(Switch("unix-scheduler", "unix", 0,
                              "selects the best Unix scheduler device for MIDI "
                              "output (default)",
                              &TSE3Play::handle_unix));
    switches.push_back(Switch("oss-scheduler", "oss", 0,
                              "selects the OSS for MIDI output",
                              &TSE3Play::handle_oss));
    switches.push_back(Switch("alsa-scheduler", "alsa", 0,
                              "selects the ALSA library for MIDI output",
                              &TSE3Play::handle_alsa));
    switches.push_back(Switch("arts-scheduler", "arts", 0,
                              "selects aRts for MIDI output",
                              &TSE3Play::handle_arts));
    switches.push_back(Switch("stream-scheduler", "stream", 0,
                              "selects standard output stream for MIDI output",
                              &TSE3Play::handle_stream));
    switches.push_back(Switch("start", "s", 1,
                              "change start clock to specified pulse no",
                              &TSE3Play::handle_start));
    switches.push_back(Switch("solo-track", "solo", 1,
                              "play with solo on given input (not MIDI) track",
                              &TSE3Play::handle_solo));
    switches.push_back(Switch("sleep", "sl", 1,
                              "sets OS usecs sleep time between updates\n"
                              "(default 100)",
                              &TSE3Play::handle_sleep));
    switches.push_back(Switch("no-stop", "stop", 0,
                              "prevents tse3play from stopping at the end of "
                              "the song",
                              &TSE3Play::handle_nostop));
    switches.push_back(Switch("echo", "e", 0,
                              "enables the MIDI echo facility (soft MIDI Thru)",
                              &TSE3Play::handle_echo));
    switches.push_back(Switch("mute-track", "mute", 1,
                              "mutes the specified track (first is 0)",
                              &TSE3Play::handle_mute));
    switches.push_back(Switch("patches-dir", "pd", 1,
                              "specifies the directory for patch files "
                              "(OSS only)",
                              &TSE3Play::handle_patchesdir));
    switches.push_back(Switch("internal", "int", 0,
                              "force all output to first internal port",
                              &TSE3Play::handle_internal));
    switches.push_back(Switch("external", "ext", 0,
                              "force all output to first external port",
                              &TSE3Play::handle_internal));
    switches.push_back(Switch("force-port", "port", 1,
                              "force all output to be on specified port",
                              &TSE3Play::handle_port));
    switches.push_back(Switch("fast-midi", "fast", 0,
                              "use a faster MIDI import routine",
                              &TSE3Play::handle_fast));

    // Parse the command line

    if (argc <= 1)
    {
        handle_version(0, argv);
        handle_help(0, argv);
        exit(0);
    }

    for (int n = 1; n < argc; n++)
    {
        bool done = false;
        for(std::vector<Switch>::iterator sw = switches.begin();
            !done && sw != switches.end();
            sw++)
        {
            if (argv[n] == std::string("-") + sw->srt
                || argv[n] == std::string("--") + sw->lng)
            {
                done = true;
                if (n + sw->nargs >= argc)
                {
                    cerr << "Error in command format ("
                         << argv[n] << " expects "
                         << sw->nargs
                         << " arguments)\n";
                    exit(1);
                }
                (this->*(sw->handler))(n, argv);
                n += sw->nargs;
            }
        }
        if (!done)
        {
            filename = argv[n];
            if (verbose) cout << "Input filename is: " << filename << "\n";
        }
    }
    if (filename == "")
    {
        cerr << "No filename specified.\n";
        dovisual = false;
        doplay   = false;
    }
}


TSE3Play::~TSE3Play()
{
}


/*****************************************************************************
 * TSE3Play class: The main program code
 ****************************************************************************/

int TSE3Play::go()
{
    if (verbose)
    {
        cout << "\n*** tse3play (verbose mode) ***\n\n";
        handle_version(0, 0);
        cout << "\n";
    }

    Playable      *playable  = 0;
    Song          *song      = 0;
    MidiScheduler *sch       = 0;
    Metronome     *metronome = 0;
    Transport     *transport = 0;

    if (doplay || midi || gm || gs || xg)
    {
        if (patchesDir != std::string())
        {
#ifdef TSE3_WITH_OSS
            Plt::OSSMidiScheduler::setFmPatchesDirectory(patchesDir);
            Plt::OSSMidiScheduler::setGusPatchesDirectory(patchesDir);
#else
            cout << "Patch dir specificed, but no OSS scheduler support\n";
#endif
        }

        if (schedtype == Stream)
        {
            sch = new Util::StreamMidiScheduler();
        }
        else
        {
            using namespace Plt::UnixMidiSchedulerFactory;
            switch (schedtype)
            {
                case Arts:
                {
#ifdef TSE3_WITH_ARTS
                    setPreferredPlatform(UnixPlatform_Arts);
                    break;
#endif
                }
                case Alsa:
                {
#ifdef TSE3_WITH_ALSA
                    setPreferredPlatform(UnixPlatform_Alsa);
                    break;
#endif
                }
                case OSS:
                {
#ifdef TSE3_WITH_OSS
                    setPreferredPlatform(UnixPlatform_OSS);
                    break;
#endif
                }
                case Unix:   // Whatever comes
                case Stream: // Invalid case, but quietens warnings
                {
                    break;
                }
            }
            MidiSchedulerFactory msf;
            sch = msf.createScheduler();
        }

        if (listports || verbose)
        {
            cout << "MidiScheduler details follow\n"
                 << "\n  Implementation name: " << sch->implementationName()
                 << "\n            Num ports: " << sch->numPorts();
            std::vector<int> portNums;
            sch->portNumbers(portNums);
            for (size_t port = 0; port < sch->numPorts(); port++)
            {
                cout << "\n  ------- Port number: " << portNums[port]
                     << "\n                 Type: " << sch->portType(portNums[port])
                     << "\n                 Name: " << sch->portName(portNums[port])
                     << "\n          Is readable: ";
                if (sch->portReadable(portNums[port]))
                    cout << "Yes";
                else
                    cout << "No";
                cout << "\n         Is writeable: ";
                if (sch->portWriteable(portNums[port]))
                    cout << "Yes";
                else
                    cout << "No";
                cout << "\n         Destination:  ";
                if (sch->portInternal(portNums[port]))
                    cout << "Internal";
                else
                    cout << "External";
            }
            cout << "\n\n";
        }

        if (port == USE_INTERNAL_PORT)
        {
            port = sch->defaultInternalPort();
            if (verbose && port == MidiCommand::NoPort)
            {
                cout << "No default internal port, no port forced\n";
            }
        }
        else if (port == USE_EXTERNAL_PORT)
        {
            port = sch->defaultInternalPort();
            if (verbose && port == MidiCommand::NoPort)
            {
                cout << "No default external port, no port forced\n";
            }
        }
        else if (port == USE_BEST_EFFORT_PORT)
        {
            port = sch->defaultInternalPort();
            if (port == MidiCommand::NoPort)
            {
                if (verbose) cout << "No default internal port\n";
                port = sch->defaultExternalPort();
                if (port == MidiCommand::NoPort && verbose)
                    cout << "No default internal port\n";
            }
        }

        attachTo(sch);

        // Create the scheduler and other playback objects
        metronome = new Metronome();
        metronome->setStatus(Transport::Playing, dometronome);
        transport = new Transport(metronome, sch);
        setPanic(transport->startPanic());
        setPanic(transport->endPanic());
        transport->midiEcho()->filter()->setStatus(doecho);
    }

    // Draw the on screen presence

    bool dovisual_nodeferedframe
        = dovisual && !verbose && outmidi != "-" && outtse3mdl != "-";

    TSE3PlayVisual visual(transport, sch);
    if (dovisual_nodeferedframe) visual.drawFrame();

    // Loading the file

    if (filename != "")
    {
        if (dovisual_nodeferedframe) visual.message("Loading file");
        FileRecogniser fr(filename);
        if (verbose) cout << "File type is: ";
        switch (fr.type())
        {
            default:
            case FileRecogniser::Type_Error:
            {
                if (dovisual_nodeferedframe)
                    visual.message("File couldn't be opened. Exiting.");
                if (verbose) cout << "load error - exiting\n";
                cerr << "Load error for \"" << filename.c_str() << "\"\n";
                return 1;
            }
            case FileRecogniser::Type_Unknown:
            {
                if (dovisual_nodeferedframe)
                    visual.message("Filetype unknown. Exiting.");
                if (verbose) cout << "unknown - exiting\n";
                cerr << "Unknown filetype for \"" << filename.c_str() << "\"\n";
                return 1;
            }
            case FileRecogniser::Type_TSE3MDL:
            {
                if (dovisual_nodeferedframe)
                    visual.message("Loading TSE3MDL file");
                if (verbose) cout << "TSE3MDL\n";
                TSE3MDL tse3mdl(_name);
                song     = tse3mdl.load(filename);
                playable = song;
                break;
            }
            case FileRecogniser::Type_TSE2MDL:
            {
                if (dovisual_nodeferedframe)
                    visual.message("Loading TSEMDL file");
                if (verbose) cout << "TSEMDL (legacy TSE version 2)\n";
                TSE2MDL fo(_name, verbose);
                song     = fo.load(filename);
                playable = song;
                break;
            }
            case FileRecogniser::Type_Midi:
            {
                if (dovisual_nodeferedframe)
                    visual.message("Loading MIDI file");
                if (verbose) cout << "MIDI\n";
                MidiFileImport *mfi = new MidiFileImport(filename, verbose);
                if (fastMidi && outmidi == std::string()
                    && outtse3mdl == std::string())
                {
                    playable = mfi;
                }
                else
                {
                    try
                    {
                        song = mfi->load();
                    }
                    catch (const MidiFileImportError &mf)
                    {
                        cout << "Midi File Import was unsuccessful: "
                             << *mf << "\n";
                        return 1;
                    }
                    playable = song;
                }
                break;
            }
        }
        if (verbose) cout << "File loaded.\n";
    }
    else
    {
        song     = new Song();
        playable = song;
    }

    if (dovisual)
    {
        // Don't need to do the calculation if it's not needed
        visual.setLastClock(playable->lastClock());
    }

    // Handle conversion facilities

    if (outmidi != std::string() && song)
    {
        if (dovisual_nodeferedframe)
            visual.message("Performing MIDI output");
        MidiFileExport mfe(outmidiformat, outmidicompact, verbose);
        if (outmidi == "-")
        {
            mfe.save(std::cout, song);
        }
        else
        {
            mfe.save(outmidi, song);
        }
    }
    if (outtse3mdl != std::string() && song)
    {
        if (dovisual_nodeferedframe)
            visual.message("Performing TSE3MDL output");
        TSE3MDL tse3mdl(_name);
        if (outtse3mdl == "-")
        {
            tse3mdl.save(std::cout, song);
        }
        else
        {
            tse3mdl.save(outtse3mdl, song);
        }
    }

    // Now do playback

    if (doplay)
    {
        // Perform modification of Song according to command line parameters

        if (song)
        {
            song->setSoloTrack(soloTrack);
            std::list<size_t>::iterator i = muteList.begin();
            while (i != muteList.end())
            {
                if (*i <= song->size())
                    (*song)[*i]->filter()->setStatus(false);
                i++;
            }
        }
        transport->filter()->setPort(port);

        // Handle playback facilities

        if (dovisual && !dovisual_nodeferedframe) visual.drawFrame();
        if (dovisual)                             visual.blankFrame();

        if (verbose && !dovisual) cout << "Playing...\n";

        more_to_come = true;

        if (startClock == -1)
        {
            // Start from time of first event in the Playable.
            // This may prevent a lengthy empty intro.
            PlayableIterator *pt = playable->iterator(0);
            startClock = (**pt).time;
            delete pt;
        }

        // Play loop
        do
        {
            // Call the transport to do some playing
            transport->play(playable, startClock);
            while (more_to_come || !dostop)
            {
                transport->poll();
                if (dovisual) visual.poll();
                // If we want to scan for keyboard input (in a non-platform
                // specific way) then do it here: if 'q' or 'Q' is pressed
                // then set more_to_come = false;
                if (usleepPeriod)
                {
                    usleep(usleepPeriod);
                    //timespec a = {0, usleepPeriod*10}, b;
                    //nanosleep(&a, &b);
                }
            }


        } while (more_to_come || (doloop || !dostop));


        if (dovisual) visual.blankFrame();
        if (dovisual) visual.message("Finished playing");
        if (verbose && !dovisual) cout << "\n\nFinished playing.\n";
    }
    else
    {
        // If a reset chosen, send it
        if (midi || gm || gs || xg)
        {
            transport->play(0, 0);
        }
    }

    // Reset terminal
    cout << escChar << "[0m\n";

    delete playable;
    delete transport;
    delete metronome;
    delete sch;

    return 0;
}


/*****************************************************************************
 * TSE3Play class: Handling command line switches
 ****************************************************************************/

void TSE3Play::handle_help(int, char*[])
{
    cout << "Usage: " << _name << " [OPTION]... [FILE]\n"
         << "Plays and converts TSE3MDL and MIDI files.\n\n"
         << "OPTIONs are:\n\n";

    // Work out column widths for the nicely formatted output
    unsigned int srtsize = 0;
    unsigned int lngsize = 0;
    for(std::vector<Switch>::iterator n = switches.begin();
        n != switches.end(); n++)
    {
        if (n->srt.size() > srtsize) srtsize = n->srt.size();
        if (n->lng.size() > lngsize) lngsize = n->lng.size();
    }
    srtsize += 2;
    lngsize += 2;

    // Produce the nicely formatted output
    for(std::vector<Switch>::iterator n = switches.begin();
        n != switches.end(); n++)
    {
        cout << "  -"  << n->srt
             << std::string(srtsize-n->srt.size(), ' ')
             << std::string(" --") + n->lng + " "
             << std::string(lngsize-n->lng.size(), ' ')
             << n->help
             << "\n";
    }

    cout << "\nSend bug reports to <pete@cthree.org>\n";
    exit(0);
}


void TSE3Play::handle_version(int, char*[])
{
    cout << _name << " version " << _version/100
         << "." << _version%100 << " built on " << __DATE__ << "\n"
         << "TSE3 library version:   " << TSE3::TSE3_Version() << "\n"
         << "TSE3 library copyright: " << TSE3::TSE3_Copyright() << "\n";
}


void TSE3Play::handle_verbose(int, char*[])
{
    verbose++;
}


void TSE3Play::handle_listports(int, char*[])
{
    listports = true;
}


void TSE3Play::handle_novisual(int, char*[])
{
    dovisual = false;
    if (verbose) cout << "Producing no visual playback feedback.\n";
}


void TSE3Play::handle_noplay(int, char*[])
{
    doplay = false;
    if (verbose) cout << "Will not play input file.\n";
}


void TSE3Play::handle_loop(int, char*[])
{
    doloop = false;
    if (verbose) cout << "Looping playback set.\n";
}


void TSE3Play::handle_outmidi(int argpos, char *argv[])
{
    outmidi = argv[argpos+1];
    if (verbose) cout << "Producing MIDI output in: " << outmidi << "\n";
}


void TSE3Play::handle_outmidiformat0(int, char *[])
{
    outmidiformat = 0;
    if (verbose)
       cout << "Setting output MIDI file format to: " << outmidiformat << "\n";
}


void TSE3Play::handle_outmidicompact(int, char*[])
{
    outmidicompact = true;
    if (verbose) cout << "Setting MIDI file compact mode\n";
}


void TSE3Play::handle_outtse3mdl(int argpos, char *argv[])
{
    outtse3mdl = argv[argpos+1];
    if (verbose) cout << "Producing tse3mdl output in: " << outtse3mdl << "\n";
}


void TSE3Play::handle_mapchannel(int argpos, char *argv[])
{
    int from = atoi(argv[argpos+1]);
    int to   = atoi(argv[argpos+2]);
    if (verbose) cout << "Channel " << from << " to " << to << "\n";
}


void TSE3Play::handle_metronome(int, char*[])
{
    dometronome = true;
    if (verbose) cout << "Enabling metronome\n";
}


void TSE3Play::handle_reset_midi(int, char*[])
{
    midi = true;
    if (verbose) cout << "Enabling MIDI reset\n";
}


void TSE3Play::handle_reset_gm(int, char*[])
{
    gm = true;
    if (verbose) cout << "Enabling GM reset\n";
}


void TSE3Play::handle_reset_gs(int, char*[])
{
    gs = true;
    if (verbose) cout << "Enabling GS reset\n";
}


void TSE3Play::handle_reset_xg(int, char*[])
{
    xg = true;
    if (verbose) cout << "Enabling XG reset\n";
}


void TSE3Play::handle_unix(int, char*[])
{
    schedtype = Unix;
    if (verbose) cout << "Selecting best Unix device for MIDI output\n";
}


void TSE3Play::handle_oss(int, char*[])
{
#ifdef TSE3_WITH_OSS
    schedtype = OSS;
    if (verbose) cout << "Selecting OSS for MIDI output\n";
#else
    std::cerr << "OSS is not supported on this computer\n";
#endif
}


void TSE3Play::handle_alsa(int, char*[])
{
#ifdef TSE3_WITH_ALSA
    schedtype = Alsa;
    if (verbose) cout << "Selecting ALSA for MIDI output\n";
#else
    std::cerr << "ALSA is not supported on this computer\n";
#endif
}


void TSE3Play::handle_arts(int, char*[])
{
#ifdef TSE3_WITH_ARTS
    schedtype = Arts;
    if (verbose) cout << "Selecting aRts for MIDI output\n";
#else
    std::cerr << "aRts is not supported on this computer\n";
#endif
}


void TSE3Play::handle_stream(int, char*[])
{
    schedtype = Stream;
    dovisual  = false;
    if (verbose) cout << "Selecting standard output for MIDI output\n";
}


void TSE3Play::handle_start(int argpos, char *argv[])
{
    startClock = atoi(argv[argpos+1]);
    if (verbose) cout << "Start " << startClock << "\n";
}


void TSE3Play::handle_solo(int argpos, char *argv[])
{
    soloTrack = atoi(argv[argpos+1]);
    if (verbose) cout << "SoloTrack " << soloTrack << "\n";
}


void TSE3Play::handle_sleep(int argpos, char *argv[])
{
    usleepPeriod = atoi(argv[argpos+1]);
    if (verbose) cout << "Sleep " << usleepPeriod << " usecs\n";
}


void TSE3Play::handle_nostop(int, char*[])
{
    dostop = false;
    if (verbose) cout << "Stop at song end disabled\n";
}


void TSE3Play::handle_echo(int, char*[])
{
    doecho = true;
    if (verbose) cout << "MIDI echo enabled\n";
}


void TSE3Play::handle_mute(int argpos, char *argv[])
{
    int trackNo = atoi(argv[argpos+1]);
    if (verbose) cout << "Muting Track " << trackNo << "\n";
    muteList.push_back(trackNo);
}


void TSE3Play::handle_patchesdir(int argpos, char *argv[])
{
#ifdef TSE3_WITH_OSS
    patchesDir = argv[argpos+1];
    if (verbose) cout << "Patches directory is: " << patchesDir.c_str() << "\n";
#else
    std::cerr << "OSS is not supported on this computer\n";
#endif
}


void TSE3Play::handle_internal(int, char *[])
{
    if (verbose) cout << "Forcing output to first internal port\n";
    port = USE_INTERNAL_PORT;
}


void TSE3Play::handle_external(int, char *[])
{
    if (verbose) cout << "Forcing output to first external port\n";
    port = USE_EXTERNAL_PORT;
}


void TSE3Play::handle_port(int argpos, char *argv[])
{
    port = atoi(argv[argpos+1]);
    if (verbose) cout << "Forcing output to port " << port << "\n";
}


void TSE3Play::handle_fast(int, char *[])
{
    fastMidi = true;
    if (verbose) cout << "Using fast MIDI import routine\n";
}


void TSE3Play::MidiScheduler_Stopped(MidiScheduler *)
{
    more_to_come = false;
}


void TSE3Play::Notifier_Deleted(MidiScheduler *)
{
    more_to_come = false;
}


void TSE3Play::setPanic(Panic *panic)
{
    panic->setMidiReset(midi);
    panic->setGmReset(gm);
    panic->setGsReset(gs);
    panic->setXgReset(xg);
}


TSE3Play::Switch::Switch
    (std::string l, std::string s, int n, std::string h, handler_t hd)
: lng(l), srt(s), nargs(n), help(h), handler(hd)
{
    for (std::string::size_type n = 0; n < help.size(); ++n)
    {
        if (help[n] == '\n')
        {
            help.replace(n, 1, "\n                                     ");
        }
    }
}


/*****************************************************************************
 * TSE3PlayVisual class
 ****************************************************************************/

TSE3PlayVisual::TSE3PlayVisual(Transport *t, MidiScheduler *s)
: transport(t), scheduler(s), lastMsec(0), x(0), y(0),
  lastClock(0), barPos(-1)
{
    for (int chan = 0; chan < 16; chan++)
    {
        now[chan] = next[chan] = 0;
    }
    if (transport) transport->attachCallback(this);
    if (transport) attachTo(transport);
}


TSE3PlayVisual::~TSE3PlayVisual()
{
    if (transport) transport->detachCallback(this);
}


void TSE3PlayVisual::setLastClock(Clock lc)
{
    lastClock = lc;
}


void TSE3PlayVisual::Transport_MidiIn(MidiCommand c)
{
    Transport_MidiOut(c);
}


void TSE3PlayVisual::Transport_MidiOut(MidiCommand c)
{
    if (0)
        next[c.channel] = max;
    if (c.status == MidiCommand_NoteOn)
    {
        int newval = c.data2 * max / 127;
        if (newval > next[c.channel] && newval != now[c.channel])
            next[c.channel] = newval;
    }
}


void TSE3PlayVisual::drawFrame()
{
    cout << escChar << "[34m"
         << "-------------------------------\n"
         << escChar << "[37m"
         << "                               \n"
         << "           tse3play            \n"
         << "                               \n"
         << "   (c) 2000-2 Pete Goodliffe   \n"
         << " Incorporates TSE3 technology  \n"
         << escChar << "[34m";
    for (int n = 5; n <= max; n++)
        cout << "                                 \n";
    cout << "0-1-2-3-4-5-6-7-8-9-A-B-C-D-E-F\n"
         << "-------------------------------\n"
         << escChar << "[37m";
}


void TSE3PlayVisual::message(const std::string &message)
{
    move(0, 5);
    cout << "                               \r";
    move(31/2-message.size()/2, 5);
    cout << escChar << "[33;1m"
         << message
         << escChar << "[37m";
    x += message.size();
    move(0, 0);
    cout.flush();
}


void TSE3PlayVisual::blankFrame()
{
    for (int n = 0; n <= max; n++)
    {
        move(0, 3+n);
        cout << "                               \r";
    }
    move(0, 0);
}


void TSE3PlayVisual::poll()
{
    Clock now = scheduler->clock();
    cout << now / Clock::PPQN << ":" << now % Clock::PPQN << "  \r";
    if (1)
    {
        move(10, 0);
        cout << "(LA: " << transport->lookAhead();
        if (transport->breakUps())
            cout << ", BreakUps: " << transport->breakUps();
        cout << ")  \r";
        x = 0;
    }
    int msec = scheduler->msecs();
    if (msec > lastMsec + 100)
    {
        updateBars();
        lastMsec = msec;
    }
}


void TSE3PlayVisual::updateBars()
{
    // VU Bars

    for (int chan = 0; chan < 16; chan++)
    {
        if (now[chan] != next[chan])
        {
            char c   = (now[chan] > next[chan]) ? ' ' : '=';
            int  bot = (now[chan] > next[chan]) ? next[chan] : now[chan];
            int  top = (now[chan] > next[chan]) ? now[chan]  : next[chan];
            for (int n = bot; n <= top; n++)
            {
                // work out colour
                char *colcode = "37"; // white
                if (n > max*2/3)
                    colcode = "31"; // red
                else if (n > max/3)
                    colcode = "33"; // yellow
                else
                    colcode = "32"; // green
                cout << escChar << "[" << colcode << ";1m";

                // print character
                moveout(chan*2, 3+n, c);
            }
            now[chan] = next[chan];
            if (now[chan] > 0) next[chan] = now[chan]-1;
        }
    }

    // Song position bar

    if (lastClock)
    {
        int newBarPos = scheduler->clock() * 31 / lastClock;
        if (newBarPos != barPos && newBarPos <= 31)
        {
            cout << escChar << "[32;1m";
            for (; barPos < newBarPos; barPos++)
            {
                moveout(barPos, 1, '=');
            }
        }
    }

    // Tidy up

    cout << escChar << "[0m";
    move(0, 0);
    cout.flush();
}


void TSE3PlayVisual::move(int newx, int newy)
{
    if (newx < x)
    {
        cout << escChar << "[" << x-newx << "D";
    }
    else if (newx > x)
    {
        cout << escChar << "[" << newx-x << "C";
    }
    x = newx;

    if (newy < y)
    {
        cout << escChar << "[" << y-newy << "B";
    }
    else if (newy > y)
    {
        cout << escChar << "[" << newy-y << "A";
    }
    y = newy;
}


void TSE3PlayVisual::moveout(int newx, int newy, char c)
{
    move(newx, newy);
    cout.put(c);
    x++;
}


void TSE3PlayVisual::Notifier_Deleted(Transport *)
{
    transport = 0;
}

