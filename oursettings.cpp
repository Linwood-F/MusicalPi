#include "oursettings.h"

ourSettings::ourSettings(MainWindow* parent)
{
    qDebug()<< "Entered";
    mParent = parent;
    setPtr = new QSettings("LE Ferguson LLC","MusicalPi",(QObject*)parent);
    qDebug() << "Settings file=" << setPtr->fileName() << ", Format=" << setPtr->format();
    // Now go through key items
    midiPort    = setPtr->value("midiPort",20).toUInt();                    // Get from aconnect -l or pmidi -l, or define by qmidinet
    // Force the midi port to always be in the file (this incidentally creates an initial file on new installs)
    setPtr->setValue("midiPort",midiPort); // It's a noop if it already exists

    calibrePath       = setPtr->value("calibrePath","/mnt/lef/t/Calibre Library").toString(); // Path must exist in OS ahead of time
    calibreDatabase   = setPtr->value("calibreDatabase","metadata.db").toString();            // Database file name
    calibreMusicTag   = setPtr->value("calibreMusicTag","music").toString();                  // Tag must be setup in Calibre
    logoPct           = setPtr->value("logoPct",0.20).toFloat();                                   // How wide should logo be on non-play window
    pageBorderWidth   = setPtr->value("pageBorderWidth",10).toInt();                          // Border around pages

    // This one can be smaller than calculated below, but is set to allow twice the number of pages
    // ahead of, and the same number behind, the max "up" display, plus some slop, so page forward/back works.
    // The 2:1 ratio of ahead to behind is hard coded into the cache range calculation routine
    // Large impact if larger; if "up" changes and this gets too large it can be smaller and will
    // only really impact performance when paging through with very high "up" size.

    maxCache    = setPtr->value("maxCache",(4 * (MUSICALPI_MAXROWS * MUSICALPI_MAXCOLUMNS) + 3)).toInt();

    // Duration and sizing of "where to touch" overlay hint went switched to play mode

    overlayDuration    = setPtr->value("overlayDuration",3000).toInt();
    overlayTopPortion  = setPtr->value("overlayTopPortion",0.20).toFloat();
    overlaySidePortion = setPtr->value("overlaySidePortion",0.40).toFloat();

    // During page turns, how long does it delay for the turn, highlights, etc. and cosmetics

    pageTurnDelay        = setPtr->value("pageTurnDelay",3200).toInt();
    pageHighlightDelay   = setPtr->value("pageHighlightDelay",1500).toInt();
    pageHighlightHeight  = setPtr->value("pageHighlightHeight",10).toInt();

    // ALSA Interface and player tuning
    ALSAlowWater             = setPtr->value("ALSAlowWater",70).toUInt();             // Events at which we wait for queue to empty
    ALSAhighWater            = setPtr->value("ALSAhighWater",170).toUInt();           // Events at which we start queuing again
    ALSAmaxOutputBuffer      = setPtr->value("ALSAmaxOutputBuffer",512).toUInt();     // Maximum we'll go in trying to set buffer size
    ALSAqueueChunkSize       = setPtr->value("ALSAqueueChunkSize",50).toUInt();       // How many notes to send each loop (max)
    ALSApacingInterval       = setPtr->value("ALSApacingInterval",10).toUInt();       // How long in mSec we wait when queue is full before checking again
    ALSAMidiQuashResetAll    = setPtr->value("debugMidiQuashResetAll",true).toBool(); // Should we above sending ResetAll's (some devices may not handle, or may miss the first notes)
    midiInitialVelocityScale = setPtr->value("midiInitialVelocityScale",20).toUInt(); // How loud do we play by default (% of full)
    midiInitialTempoScale    = setPtr->value("midiInitialTempoScale",100).toUInt();   // How fast do we play by default (% of full)
    statusUpdateRate         = setPtr->value("statusUpdateRate",500).toUInt();        // How fast in msec to update measure display (and related)

    // Debugging control

    debugMidiSendDetails      = setPtr->value("debugMidiSendDetails",false).toBool();      // Should midi outputs be itemized
    debugMidiFileParseDetails = setPtr->value("debugMidiFileParseDetails",false).toBool(); // Should parse of file be itemized
    debugMidiMeasureDetails   = setPtr->value("debugMidiMeasureDetails",false).toBool();   // During file parse should measure contains be itemized
    debugQueueInfoInterval    = setPtr->value("debugQueueInfoInterval",500).toUInt();      // How often should queue status debugging be written (msec)

}
ourSettings::~ourSettings()
{
    if(setPtr!=NULL) setPtr->sync();
    DELETE_LOG(setPtr);
}

void ourSettings::setSetting(QString key, QVariant value)
{
    setPtr->setValue(key,value);
    setPtr->sync();
}
