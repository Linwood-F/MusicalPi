#include "midiplayerV2.h"

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#define DELETE_LOG(X) if(X != NULL) { qDebug() << "Freeing " #X; delete X; }

midiPlayerV2::midiPlayerV2(QWidget *parent, QString _midiFile, QString _titleName) : QWidget(parent)
{
    qDebug() << "Entered";
    setWindowTitle("Midi Player - " + _titleName);
    errorEncountered = "";  // Once set this cannot be unset in this routine - close and open again
    this->setWindowFlags(Qt::Window|Qt::Dialog);
    timer = NULL;
    handle = 0; // indicates we haven't initialized sequencer
    currentIsRunning = 0; // and of course we can't be running the queue yet
    midiFile = _midiFile;
    doPlayingLayout();  // This is needed even if not playing to show error
    updateVolume(MUSICALPI_INITIAL_VELOCITY_SCALE);
    updateTempo(MUSICALPI_INITIAL_TIME_SCALE);
    openAndLoadFile();
    updateSliders();  // This is really only needed for error conditions since the above won't update then
}

midiPlayerV2::~midiPlayerV2()
{
    // Clean up allocations???
}

bool midiPlayerV2::parseFileAndPlay(bool playFlag, int playAtMeasure)
{
    // Because looping through the file is nearly identical for playing and the initial measure
    // position calculation, the same code is used for both. Also, there may be context we may use later
    // (e.g. key signatures) so when called, it ALWAYS starts over in the review of the file, but
    // doesn't send (much) data until it gets to the right measure (tempo changes are sent so we are set
    // up right for example - one day we may need some of those other weird controls sent to get in the
    // right state)

    // this is written so that you COULD play on the first call before parsing (i.e. canPlay true
    // playFlag true ) to go immediately into play mode; as of this writing
    // instead the player starts in stopped mode so you can reposition the sliders for initial play,
    // but we do go ahead and make sure we can parse the whole file (so we can give errors and possibly update
    // some displayable stuff.
    //
    // canPlay indicates an irredemable error somewhere if false; routines should just do nothing if ! canPlay.

    assert( ! (playFlag && !canPlay) );
    assert( ! (!playFlag && playAtMeasure>0) );
    const int textNames_count = 9;
    const QString textNames[9] = {"not valid <","Text Event","Copyright","Sequence/Track Name","Instrument Name","Lyric","Marker","Cue Point","not valid >"};
    // Default if not specified is 120 beats per minute (here QPM) and 4/4 time, and 480 ticks per beat (per quarter)
    // Set these here in case we get a file with no time/tempo signatures
    runningMeasureNumber = 1;
    runningTempoAsuSec = 0;
    runningTempoAsQPM = 120;
    runningTimeNumerator = 4;
    runningTimeDenominator = 4;
    runningMeasureStartTick = 0;

    if(handle == 0 and canPlay) canPlay= openSequencerInitialize();  // set up early just in case we are playing and analyzing both
    if(!canPlay) return canPlay;
    if(playFlag)
    {
        getQueueInfo();
        if (currentIsRunning&& 1==2)  // ???
        {
            qDebug() << "Stopping queue so we can (if needed) reposition during a play call";
            snd_seq_stop_queue(handle, queue, NULL);  // to reposition stop queue first
            snd_seq_drain_output(handle);
            // Note we leave it stopped until we get the first event below so we have a tick for start time, it's started in sendIt()
            currentIsRunning = false;
        }
    }

    MidiEvent *ptr;
    int noteOnSent = 0;
    // This look goes through events, starting on requested position (or first time at beginning)
    int startAtEvent = (playAtMeasure <= 1 ? 0 : measures[playAtMeasure - 1].startEventNumber);
    startAtTick = (&(mfi[0][startAtEvent]))->tick;
    qDebug() << "Starting event processing at event " << startAtEvent << ", using startAtTick=" << startAtTick;
    for (int i = 0; i < mfi.getEventCount(0); i++)  // Always starting at zero, just don't send unless we need to
    {
        QString midiDataText;  // Cumulative debug output for this event
        ptr = &(mfi[0][i]);    // Point to an event

        // initialize an ALSA event in case we need it based on this event's time (only really needed if playing but easier to keep in scope by always doing)
        // Note that the tick is calculated, since any restart has the queue time at zero
        snd_seq_event_t ep;  // new event
        snd_seq_ev_clear(&ep);
        // The max() on the tick below prevents early events (like tempo) from being missed when sent ahead of the real notes
        snd_seq_ev_schedule_tick(&ep, queue, 1, max(0,ptr->tick - startAtTick));  // 3rd parameter 0=absolute, <>0 = relative  ?? support for absolute files?   ???

        ep.source = sourceAddress;
        ep.dest = destAddress;

        // Since we always start at zero, it's not significantly more work to recalculate measures (and the code is cleaner without all the exceptions)
        while (ptr->tick >= runningMeasureStartTick + runningTicksPerMeasure)
        {
            if(2==1) // Need a better test for detailed debugging
                qDebug() << "Finished measure " << runningMeasureNumber
                         << ", start tick = " << measures[runningMeasureNumber - 1].startTick
                         << ", ticks per measure = " << measures[runningMeasureNumber - 1].ticksPerMeasure
                         << ", uSec per tick = " << measures[runningMeasureNumber - 1].uSecPerTick;
            runningMeasureNumber++;   // Step in whole measures and increments until we are inside a measure
            runningMeasureStartTick += runningTicksPerMeasure;
            // Note we initialize these based on running, but tempo/time sign will change them
            measures[runningMeasureNumber - 1].startTick = runningMeasureStartTick;
            measures[runningMeasureNumber - 1].ticksPerMeasure = runningTicksPerMeasure;
            measures[runningMeasureNumber - 1].uSecPerTick = runninguSecPerTick;
            measures[runningMeasureNumber - 1].startEventNumber = i;
        }

        if(ptr->isAftertouch())
        {
            midiDataText = "Aftertouch channel " + QString::number(ptr->getChannel()) + " note " + QString::number(ptr->getP1()) + " to value " + QString::number(ptr->getP2());
            if(playFlag) qDebug() << "Not sending aftertouch ??? at tick " << ptr->tick;
        }
        else if(ptr->isPressure())
        {
            midiDataText = "Pressure channel "   + QString::number(ptr->getChannel()) + " to value " + QString::number(ptr->getP1());
            if(playFlag) qDebug() << "Not sending pressure at tick " << ptr->tick;
        }
        else if(ptr->isController())
        {
            QString ctrlr = "Controller " + QString::number(ptr->getP1());  // default use number
            switch (ptr->getP1()) // controller is
            {
                case   7: ctrlr = "Channel volume"; break;
                case  10: ctrlr = "Pan"; break;
                case  64: ctrlr = "Damper pedal"; break;
                case  66: ctrlr = "Sustenuto"; break;
                case  91: ctrlr = "Effects 1 Depth"; break;
                case 121: ctrlr = "Reset all"; break;
            }

            midiDataText = ctrlr + " on channel " + QString::number(ptr->getChannel()) + " to value " + QString::number(ptr->getP2());
            if(playFlag && i >= startAtEvent)  // Don't send these (basically pedals) if we don't need them
            {
                ep.type = SND_SEQ_EVENT_CONTROLLER;
                ep.data.control.channel = ptr->getChannel();
                ep.data.control.param = ptr->getP1();
                ep.data.control.value = ptr->getP2();
                sendIt(&ep);
            }
        }
        else if(ptr->isEndOfTrack())  midiDataText = "EndOfTrack";  // No play action needed this is info only
        else if(ptr->isNoteOn())
        {
            midiDataText = "NoteOn " + guessSpelling(ptr->getKeyNumber(),keySig,keySigMajor);
            if(playFlag && i >= startAtEvent)
            {
                ep.type = SND_SEQ_EVENT_NOTEON;
                ep.data.note.channel = ptr->getChannel();
                ep.data.note.note = ptr->getKeyNumber();
                ep.data.note.velocity = max(0,min(127,(velocityScale * ptr->getVelocity() / 100)));
                sendIt(&ep);
                noteOnSent++;
            }
        }
        else if(ptr->isNoteOff())
        {
            midiDataText = "NoteOff " + guessSpelling(ptr->getKeyNumber(),keySig,keySigMajor);
            if(playFlag && i>= startAtEvent)
            {
                ep.type = SND_SEQ_EVENT_NOTEOFF;
                ep.data.note.channel = ptr->getChannel();
                ep.data.note.note = ptr->getKeyNumber();
                ep.data.note.velocity = max(0,min(127,(velocityScale * ptr->getVelocity() / 100)));  // ok, should always be zero, but just in case go ahead and scale
                sendIt(&ep);
                // Is there just a "note" that needs to be dealt with???   Turn note-on with velocity 0 = note off?
            }
        }
        else if(ptr->isPatchChange())
        {
            midiDataText = "Patch Change channel " + QString::number(ptr->getChannel()) + " to " + QString::number(ptr->getP1());
            if(playFlag)  // I don't really know what these do, so am  going to send ahead of restarts
            {
                ep.type = SND_SEQ_EVENT_PGMCHANGE;
                ep.data.control.channel = ptr->getChannel();
                ep.data.control.value = ptr->getP1();
                sendIt(&ep);
            }
        }
        else if(ptr->isPitchbend())
        {
            int bend = ptr->getP2()<<7;
            bend = (bend << 7) | ptr->getP1();
            midiDataText = "Pitchbend adjust " + QString::number(bend);
            if(playFlag) qDebug() << "Not sending Ptichbend adust during play at tick " << ptr->tick;
        }
        else if(ptr->isTempo())
        {
            runningTempoAsuSec = ptr->getP3();
            runningTempoAsuSec = (runningTempoAsuSec << 8) | ptr->data()[4];
            runningTempoAsuSec = (runningTempoAsuSec << 8) | ptr->data()[5];
            // If we have been asked to scale adjust it here
            runningTempoAsuSec = 100 * runningTempoAsuSec / tempoScale;
            runningTempoAsQPM = (unsigned int)(60.0/runningTempoAsuSec*1000000.0 + 0.5);
            measures[runningMeasureNumber - 1].ticksPerMeasure = runningTicksPerMeasure;  // This treats all tempo changes as at the beginning of measures, which is (?) not true?   But shouldn't matter really for this purpose;
            measures[runningMeasureNumber - 1].uSecPerTick = runninguSecPerTick;
            midiDataText = "Tempo is " + QString::number(runningTempoAsQPM) + " QPM, or " + QString::number(runningTempoAsuSec) + " uSec per quarter note, " + QString::number(runninguSecPerTick) + " per tick";
            if(playFlag) // These have to go each time to get the right tempo -- does tick matter??
            {
                ep.type = SND_SEQ_EVENT_TEMPO;
                ep.data.queue.queue = queue;
                ep.data.queue.param.value = runningTempoAsuSec;
                ep.dest.client = SND_SEQ_CLIENT_SYSTEM;
                ep.dest.port = SND_SEQ_PORT_SYSTEM_TIMER;
                sendIt(&ep);
            }
        }
        else if(ptr->isTimeSignature())
        {   // Note this is ignoring the 5th and 6th bytes which are clocks per quarter (usually 24) -- not sure if we need them
            runningTimeNumerator = ptr->getP3();
            runningTimeDenominator = 2 << ((uchar)(ptr->data()[4]) - 1);
            measures[runningMeasureNumber - 1].ticksPerMeasure = runningTicksPerMeasure;  // This reats sig changes as always at start of measure, which may not be true, and may throw off count??
            measures[runningMeasureNumber - 1].uSecPerTick = runninguSecPerTick;
            midiDataText = "Time Sig " + QString::number(runningTimeNumerator) + "/" + QString::number(runningTimeDenominator) + ", ticks per measure = " + QString::number(runningTicksPerMeasure);
            // no time signature is sent while playing
        }
        else if(ptr->isKeySignature())
        {
            keySig = (signed char)ptr->getP3();  // Save running key
            keySigMajor = (ptr->data()[4] == 0);  // We change this to a boolean!
            midiDataText = "Key Sig " + keySigs[keySig + keySig_offset] + (keySigMajor ? "-Major" : "-Minor");
            // no key signature is sent while playing
        }
        else if(ptr->isMetaMessage())
        {
            switch (ptr->getMetaType())
            {
                case 0x21: // Midi Port
                    midiDataText = "Midi port " + QString::number(ptr->data()[4]);
                    if(playFlag) qDebug() << "No midi port is sent while playing at tick " << ptr->tick;  // needed??
                    break;
                case 0x01: // text
                case 0x02: // Copyright
                case 0x03: // Sequence/Track name
                case 0x04: // Instrument name
                case 0x05: // Lyric
                case 0x06: // Marker
                case 0x07: // Cue Point
                    {
                        int len = ptr->getP2();
                        char* msg = (char*)malloc(len+1);  // There's got to be a better way to get the string out
                        memcpy(msg, &ptr->data()[3],len);
                        msg[len]=0;
                        midiDataText = textNames[max(0,min(textNames_count - 1,ptr->getMetaType()))] + " = '" + QString(msg) + "'";
                        free(msg);
                        // None of these are sent while playing
                    }
                    break;
                default:
                    midiDataText = "MetaMessage - undefined -- additional case item needed";
            }
        }
        else midiDataText = "Type of event record not defined - check cases - skipping ";

        QString midiDataNumbers;
        for (int j=0; j < (int)ptr->size(); j++)
        {
           if (j == 0) midiDataNumbers += "0x" + QString::number((int)(*ptr)[j],16) + " ";
           else midiDataNumbers += QString::number((int)(*ptr)[j],10) + " ";
        }
        if(1==2)  // Find better way to flag this >???
            qDebug() << "Event " << i << " at ticks " << ptr->tick << ", on track " << ptr->track << ", measure " << runningMeasureNumber
                     << ", seconds = " << mfi.getTimeInSeconds(0,i) << ", " << midiDataText << ", " << midiDataNumbers;
    }
    if(1==2)  // flag whether to show detailed debugging of parse - need better way ???
    {
        qDebug() << "Last measure " << runningMeasureNumber
                 << ", start tick = " << measures[runningMeasureNumber - 1].startTick
                 << ", ticks per measure = " << measures[runningMeasureNumber - 1].ticksPerMeasure
                 << ", uSec per tick = " << measures[runningMeasureNumber - 1].uSecPerTick;
    }
    lastBar = runningMeasureNumber;
    snd_seq_drain_output(handle);  // Do I need to drain more frequently?   Buffer output in stages?
    return canPlay;
}

bool midiPlayerV2::openSequencerInitialize()
{
    // See if we can open the sequencer
    if(snd_seq_open(&handle,"hw",SND_SEQ_OPEN_OUTPUT,0) < 0 )
    {
        qDebug() << "Failed to open sequencer, error = " << snd_strerror(errno);
        return false;
    }
    queue = snd_seq_alloc_queue(handle);
    if (queue < 0 )
    {
        qDebug() << "Failed to create queue, error=" << snd_strerror(errno);
        return false;
    }
    sourceAddress.client = snd_seq_client_id(handle);
    int ret = snd_seq_create_simple_port(handle,NULL,SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_READ,  SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    if (ret < 0 )
    {
        qDebug() << "Failed to create port, error=" << snd_strerror(errno);
        return false;
    }
    else sourceAddress.port = ret;
    destAddress.client = MUSICALPI_MIDI_PORT;  // yeah, alsa calls this the client not port, port is zero for our use
    destAddress.port = 0;
    ret = snd_seq_connect_to(handle, sourceAddress.port, destAddress.client, destAddress.port);
    if (ret < 0 )
    {
        qDebug() << "Failed to connect to, error = " << snd_strerror(errno);
        return false;
    }

    // Set initial tempo (do not use this form while playing only for defaults)
    snd_seq_queue_tempo_t *qtempo;
    snd_seq_queue_tempo_alloca(&qtempo);
    memset(qtempo, 0, snd_seq_queue_tempo_sizeof());
    snd_seq_queue_tempo_set_ppq(qtempo, mfi.getTicksPerQuarterNote());
    snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/120);  // Default to 120 changes can come later

    ret = snd_seq_set_queue_tempo(handle, queue, qtempo);
    if (ret < 0 )
    {
        qDebug() << "Failure to set initial tempo error = " << snd_strerror(errno);
        return false;
    }
    snd_seq_drain_output(handle);
    qDebug() << "Queue = " << queue << ", handle=" << handle << ", source client=" << sourceAddress.client
             << ", source port = " << sourceAddress.port;

    // Is this where this should occur??

    if(timer==NULL)
    {
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(updateSliders()));
        timer->start(1000);
        qDebug() << "First time through, starting timer";
    }
    return true;
}


void midiPlayerV2::openAndLoadFile()
{
    // Initialize the whole array just so it makes sense if we run off the end or such
    // Should this be a dynamic strucure???
    for(int i=0; i<MUSICALPI_MAX_MEASURE; i++)  // Note [0] is measure # 1
    {
        measures[i].startTick = (i==0 ? 0 : 987654321);
        measures[i].ticksPerMeasure = runningTicksPerMeasure;
        measures[i].uSecPerTick = runninguSecPerTick;
        measures[i].startEventNumber = (i==0 ? 0 : 987654321) ;
    }
    canPlay = false;  // Will indicate if file is usable
    qDebug() << "Reading file";
    mfi.read(midiFile.toStdString());
    overallTicksPerQuarter = mfi.getTicksPerQuarterNote();
    qDebug() << "File read, tracks = " << mfi.getTrackCount() << ", TPQ=" << overallTicksPerQuarter
             << ", Time in quarters=" << mfi.getTotalTimeInQuarters() << ", Time in seconds=" << mfi.getTotalTimeInSeconds() << ", Time in ticks=" << mfi.getTotalTimeInTicks();
    // TPQ (Ticks per quarter) is supposed to remain constant
    // QPM (Quarter notes per minute) is tempo and is set in tempo signatures and stashed in the runningTempAsQPM and uSec variables

    mfi.joinTracks();    // merge tracks to one timeline
    canPlay = true;      // If we read file, this is a tentative setting corrected as more checks are done
    canPlay = parseFileAndPlay(false,0); // For now, don't start playing so they can change sliders
                                         // We should be able to make this true to cause an immediate play-while-parse.
    /// Need try/catch for MidiFile read???  Set canPlay=false
}

void midiPlayerV2::sendIt(snd_seq_event_t* ep_ptr)
{
    // If we are not actually running now, we start the queue (this is so we have an event defined for a tick
    if(!currentIsRunning)
    {

        qDebug() << "Starting queue since it is stopped and we are playing";
        int ret = snd_seq_start_queue(handle, queue, NULL);  // Note this always starts at tick 0.
        if(ret < 0)
        {
            qDebug() << "Starting queue returned error " << strerror(errno);
            return;
        }
        snd_seq_drain_output(handle);
        currentIsRunning = true;
    }
//    if(2==1)   // Change if you want detailed note transmission debugging
        qDebug() << "Sending to queue=" << ep_ptr->queue << ", tick (incl offset)=" << ep_ptr->time.tick + startAtTick
                 << ", source client=" << ep_ptr->source.client << ", source port" << ep_ptr->source.port
                 << ", dest client=" << ep_ptr->dest.client << ", dest port=" << ep_ptr->dest.port
                 << ", type=" << ep_ptr->type;
    int ret = snd_seq_event_output(handle,ep_ptr);
    if(ret < 0)
    {
        qDebug() << "send note event returned error " << strerror(errno);
        return;
    }
}

void midiPlayerV2::doPlayingLayout()
{
    outLayout = new QVBoxLayout(this);
    outLayout->setSpacing(15);
    gridLayout = new QGridLayout();
    gridLayout->setVerticalSpacing(15);

    qDebug() << "Layouts created";

    outLayout->addLayout(gridLayout);

    measureGo = new QPushButton("???",this);
    measureGo->setStyleSheet("padding: 2px;");
    connect(measureGo,&QPushButton::clicked, this, &midiPlayerV2::go);

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

    tempoLabel = new QLabel("Tempo %: ",this);
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
    volumeSlider->setValue(velocityScale);
    volumeValueLabel = new QLabel("???",this);
    volumeSlider->setMaximum(150);
    volumeSlider->setMinimum(1);
    connect(volumeSlider,SIGNAL(valueChanged(int)),this,SLOT(updateVolume(int)));
    gridLayout->addWidget(volumeLabel,3,1,1,1);
    gridLayout->addWidget(volumeSlider,3,2,1,1);
    gridLayout->addWidget(volumeValueLabel,3,3,1,1);

    errorLabel = new QLabel("",this);
    outLayout->addWidget(errorLabel);
}

void midiPlayerV2::updateSliders()
{
    if(canPlay)
    {
        volumeValueLabel->setText(QString::number(velocityScale));
        tempoValueLabel->setText(QString::number(tempoScale));

        getQueueInfo();

        qDebug() << "Updating sliders, canPlay=true, Tick=" << currentTick << ", Tick(including offset)=" << currentTick + startAtTick
                 << ", Events in queue = " << currentEvents << ", measure=" << currentMeasure
                 << ", tempo=" << currentTempo << ", skew=" << currentSkew
                 << ", skewBase=" << currentSkewBase << ", running=" << currentIsRunning;

        // use error slot but not highlighted for status
        if (currentIsRunning)
        {
            errorLabel->setText("Song is playing (stop to adjust sliders)");
            measureGo->setText(" Stop ");
            tempoSlider->setDisabled(true);
            volumeSlider->setDisabled(true);
            measureNowAt->setText(QString::number(currentMeasure));
        }
        else
        {
            errorLabel->setText("Play is stopped");
            measureGo->setText(" Play ");
            tempoSlider->setEnabled(true);
            volumeSlider->setEnabled(true);
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

void midiPlayerV2::updateVolume(int newVolume)
{
    qDebug() << "Entered";
    velocityScale = newVolume;  // Use this when sending notes
    if(volumeSlider->value() != newVolume) volumeSlider->setValue(newVolume);  // This allows internal call to force position
    updateSliders();  // hasten reflection of new info
}

void midiPlayerV2::updateTempo(int newTempo)
{
    qDebug() << "Entered";
    tempoScale = newTempo;
    if(tempoSlider->value() != newTempo) tempoSlider->setValue(newTempo);
    updateSliders();  // hasten reflection of new info
}

void midiPlayerV2::go()
{
    if(canPlay && !currentIsRunning)  // Hitting play from here uses measure or starts over if blank
    {
        int measureToPlay = 1;
        if(measureIn->text() != "")
        {
            int newBar = measureIn->text().toInt();  // Internally bars are reference 0 not 1; only the gui shows ref 1
            measureToPlay = std::max(0,std::min(lastBar,newBar));
        }
        qDebug() << "Starting play=true, measure= " << measureToPlay;
        parseFileAndPlay(true,measureToPlay);
    }
    else if (canPlay && currentIsRunning)  // Hitting stop ends play entirely but updates measure to where we were
    {
        qDebug() << "Go: Playing, so changing to stopped, stopping queue ";
        int ret = snd_seq_stop_queue(handle,queue, NULL);
        if(ret<0)
        {
            qDebug() << "Stop queue failed, returned error " << strerror(errno) << " (continuing anyway)";
        }
        snd_seq_drain_output(handle);
        measureGo->setText("Play");
    }
    else
    {
        qDebug() << "Can't play so doing nothing in go -- shouldn't be able to get here";
        measureGo->setDisabled(true);  // Shouldn't get here but if we do we can't play
    }
}
void midiPlayerV2::getQueueInfo() // set the "current" fields for the queue
{
    if(handle==0) return;  // not ready yet
    snd_seq_queue_status_t* qStatus;
    snd_seq_queue_status_alloca(&qStatus);
    memset(qStatus,0,snd_seq_queue_status_sizeof());
    int ret = snd_seq_get_queue_status(handle, queue, qStatus);
    assert(ret>=0);
    currentTick = snd_seq_queue_status_get_tick_time(qStatus);
    currentEvents = snd_seq_queue_status_get_events(qStatus);
    currentIsRunning = snd_seq_queue_status_get_status(qStatus);
    snd_seq_queue_tempo_t* qTempo;
    snd_seq_queue_tempo_alloca(&qTempo);
    memset(qTempo,0,snd_seq_queue_tempo_sizeof());
    ret = snd_seq_get_queue_tempo(handle, queue, qTempo);
    assert(ret>=0);
    currentTempo = snd_seq_queue_tempo_get_tempo(qTempo);
    currentSkew = snd_seq_queue_tempo_get_skew_base(qTempo);
    currentSkewBase = snd_seq_queue_tempo_get_skew(qTempo);
    // With the tick we can find measure (note we have to skew by the startAtTick as the queue tick always starts at zero
    // remember measure counts are also skewed by 1, so 1 below is [0]
    for(currentMeasure = 1; measures[currentMeasure].startTick <= currentTick+ startAtTick; currentMeasure++);
}

void midiPlayerV2::closeEvent(QCloseEvent *event)
{
    event->ignore();  // We ignore it here and signal the caller to delete us, but do stop any playing
//    if(canPlay && playStatus == TSE3::Transport::Playing) transport->play(0,0);
    emit requestToClose();
}

QString midiPlayerV2::guessSpelling(int note, int keySigNum, bool majorKey)
{
    // Get a reasonable spelling of the note based on key signature (e.g. give preference to flats if sig includes it, sharps if includes, etc.)
    // This is all just guesswork, so I did a lot of duplication in case it is desirable to fine tune any
    int noteMod = note % 12;  // C=0
    QString Octave = "-" + QString::number(note / 12 - 1);
    switch (keySigNum)
    {
        case -7: // Cb Ab (Flats: B, E, A, D, G, C, F)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -6: // Gb Eb (Flats: B, E, A, D, G, C)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -5: // Db Bb (Flats: B, E, A, D, G)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -4: // Ab F  (Flats: B, E, A, D)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -3: // Eb C  (Flats: B, E, A)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -2: // Bb G  (Flats: B, E)
            return QString("C DbD EbE F GbG AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case -1: // F  D  (Flats: B)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  0: // C  A
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  1: // G  E  (Sharps: F)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  2: // D  B  (Sharps: F, C)
            return QString("C C#D EbE F F#G AbA BbB ").mid(noteMod*2,2).trimmed() + Octave;
        case  3: // A  F# (Sharps: F, C, G)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  4: // E  C# (Sharps: F, C, G, D)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  5: // B  G# (Sharps: F, C, G, D, A)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  6: // F# D# (Sharps: F, C, G, D, A, E)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        case  7: // C# A# (Sharps: F, C, G, D, A, E, B)
            return QString("C C#D D#E F F#G G#A A#B ").mid(noteMod*2,2).trimmed() + Octave;
        default: return "Bad";
    }
}
