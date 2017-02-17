#include "midiplayerv2thread.h"

#define checkALSAreturn(ret,Msg) \
    if( (ret) < 0 )   \
    { \
       qDebug() <<  Msg << " error=" << snd_strerror(errno); \
       assert(ret>=0); \
    }

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

// This object is created as soon as we know we need to play
// The thread is also started as soon as we create the object and it just sits waiting

// The thread plays after all data written into the events map, with
// stop and abort available.

// See http://www.alsa-project.org/main/index.php/SMP_Design
// Basically all handle related operations need to be interlocked with the calling thread

// ??? need to send some kinds of resets on start and stop ????

midiplayerV2Thread::midiplayerV2Thread(midiPlayerV2 *parent): QThread(parent)
{
    qDebug() << "in constructor, tid = " << syscall(__NR_gettid) << ",currentThreadId()=" << currentThreadId();
    requestType = none;
    mParent = parent; // the parent widget
    handle = 0;
    queryQueueInterval = 0;
    lastTickProcessed = -1;
    maxFree = 0;
    minFree = 123456789;
    currentIsRunning = false;
    workTimer = new QTimer();
    connect(workTimer, SIGNAL(timeout()), this, SLOT(gooseThread()));
    workTimer->start(MUSICALPI_ALSA_PACING_INTERVAL);
    start();
}


midiplayerV2Thread::~midiplayerV2Thread()
{
    qDebug() << "In destructor for object";
    mutex.lock();
    requestType=abort;
    condition.wakeOne();
    mutex.unlock();
    wait();  // Since constructor/destructor are in the parent thread, this waits for the worker thread to exit before the base class destructor is called;
    if(handle) snd_seq_free_queue(handle,queue);
    //        snd_seq_drop_output(ctxp->handle);
    //		t = 0;
    //		seq_midi_event_init(ctxp, &ev, t, 0);
    //		seq_midi_control(ctxp, &ev, 0, MIDI_CTL_ALL_SOUNDS_OFF, 0);
    //		seq_send_to_all(ctxp, &ev);
    //		snd_seq_drain_output(ctxp->handle);

    //		snd_seq_free_queue(ctxp->handle, ctxp->queue);
    //		snd_seq_close(ctxp->handle);
    //        g_free(ctxtp);
    delete workTimer;
}

void midiplayerV2Thread::play(int startAtMeasure, int volumeScale, int tempoScale)
{
    // This is also run in the parent thread

    qDebug() << "Main thread requesting thread play, tid=" << syscall(__NR_gettid);
    mutex.lock();
    requestType = startPlay;
    m_startAtMeasure = startAtMeasure;
    m_volumeScale = volumeScale;
    m_tempoScale = tempoScale;
    condition.wakeOne();  // if it's stopped (might not be)
    mutex.unlock();
    return;
}
void midiplayerV2Thread::stop()
{
    qDebug() << "Main thread requesting thread stop, tid=" << syscall(__NR_gettid);
    mutex.lock();
    requestType = stopPlay;
    condition.wakeOne();  // if it's stopped (might not be)
    mutex.unlock();
    return;
}
void midiplayerV2Thread::gooseThread() // Encourage it to run a loop if it's waiting for work, just to check queue/etc.
{
    condition.wakeOne(); // we don't need to look anything out, just give it a start if it's waiting
}

bool midiplayerV2Thread::openSequencerInitialize()  // Can be called by parent thread but ONLY before play thread started
{
    // See if we can open the sequencer
    checkALSAreturn(snd_seq_open(&handle,"hw",SND_SEQ_OPEN_OUTPUT,0),"Failed to open ALSO sequencer")
    checkALSAreturn(queue = snd_seq_alloc_queue(handle),"Failed to create ALSA queue")
    sourceAddress.client = snd_seq_client_id(handle);
    int ret = snd_seq_create_simple_port(handle,NULL,SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,  SND_SEQ_PORT_TYPE_MIDI_GENERIC);
    checkALSAreturn(ret,"Failed to create ALSA port")
    sourceAddress.port = ret;
    destAddress.client = MUSICALPI_MIDI_PORT;  // yeah, alsa calls this the client not port, port is zero for our use
    destAddress.port = 0;
    checkALSAreturn(snd_seq_connect_to(handle, sourceAddress.port, destAddress.client, destAddress.port),"Failed to connect to destination (MIDI) port")

    // Set initial tempo (do not use this form while playing only for defaults)
    snd_seq_queue_tempo_t *qtempo;
    snd_seq_queue_tempo_alloca(&qtempo);
    memset(qtempo, 0, snd_seq_queue_tempo_sizeof());
    snd_seq_queue_tempo_set_ppq(qtempo, mParent->overallTicksPerQuarter);
    snd_seq_queue_tempo_set_tempo(qtempo, 60*1000000/120);  // Default to 120 changes can come later

    checkALSAreturn(snd_seq_set_queue_tempo(handle, queue, qtempo),"Failure to set initial (default) tempo")
    qDebug() << "Queue = " << queue << ", handle=" << handle << ", source client=" << sourceAddress.client
             << ", source port = " << sourceAddress.port;

    // Set the output room to zero and output size as large as it will permit
    checkALSAreturn(snd_seq_set_client_pool_output_room(handle,0),"Setting client pool room size to zero")
    // By trial and error try increasing buffer - start
    outBufferSize=0;
    for(int testSize = 64; testSize < MUSICALPI_ALSA_MAX_OUTPUT_BUFFER ; testSize += 16)
    {
        ret = snd_seq_set_client_pool_output(handle,testSize);
        if(ret==0) outBufferSize = testSize;
        else break;
    }
    qDebug() << "Determined output size max = " << outputSize << ", tid=" << syscall(__NR_gettid);
    return true;
}

void midiplayerV2Thread::getQueueInfo() //  CALL ONLY FROM THREAD!!!!
{
    if(handle==0) return;  // not ready yet (not sure we can get here)
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


    snd_seq_client_pool_t* pool;
    snd_seq_client_pool_alloca(&pool);
    memset(pool,0,snd_seq_client_pool_sizeof());
    ret = snd_seq_get_client_pool(handle,pool);
    assert(ret>=0);
    outputFree = snd_seq_client_pool_get_output_free(pool);
    outputRoom = snd_seq_client_pool_get_output_room(pool);
    outputSize = snd_seq_client_pool_get_output_pool(pool);
    maxFree = max(maxFree,outputFree);
    minFree = min(minFree,outputFree);

    assert(outBufferSize == outputSize);  // Let's make sure we actually got what we think we did

    // With the (adjusted) tick we can find measure
    for(std::map<int,midiPlayerV2::playableEvent_t>::iterator thisEvent = mParent->events.begin(); thisEvent != mParent->events.end(); thisEvent++)
        if(currentTick + startAtTick <= thisEvent->second.snd_seq_event.time.tick)
        {
            currentMeasure = thisEvent->second.measureNum;
            break;
        }

    snd_seq_queue_timer_t* qtime;
    snd_seq_queue_timer_alloca(&qtime);
    memset(qtime,0,snd_seq_queue_timer_sizeof());
    ret = snd_seq_get_queue_timer(handle, queue, qtime);
    assert(ret>=0);
    snd_seq_queue_timer_type_t qtype = snd_seq_queue_timer_get_type(qtime);

    snd_seq_queue_info_t* qinfo;
    snd_seq_queue_info_alloca(&qinfo);
    memset(qinfo,0,snd_seq_queue_info_sizeof());
    ret = snd_seq_get_queue_info(handle,queue,qinfo);
    int qlocked = snd_seq_queue_info_get_locked(qinfo);
    qDebug() << "Measure=" << currentMeasure << ", events="
             << currentEvents << (currentIsRunning ? ", Running" : ", Stopped")
             << ", Queue Tick=" << currentTick << ", lastTick=" << lastTickProcessed
             << ", tempo=" << currentTempo << ", queue type=" << qtype << ", locked=" << qlocked;
}

void midiplayerV2Thread::run()
{
    qDebug() << "In run, tid=" << syscall(__NR_gettid) << ",currentThreadId()=" << currentThreadId();
    std::map<int,midiPlayerV2::playableEvent_t>::iterator playStartEvent;
    forever
    {
        if(++queryQueueInterval % 10 == 0 )
        {
            getQueueInfo();  // Find out what we know about the queue (if anything)

            if(currentIsRunning && currentMeasure >= mParent->lastMeasure && currentEvents == 0)
            {
                qDebug() << "In thread, play is stopping as reached end of song.";
                requestType = stopPlay;
            }
        }

        // General loop checks what we are being asked to do -- different sections are NOT mutually exclusive
        if(requestType == abort)
        {
            qDebug() << "Aborting on playing thread";
            return;
        }

        if(requestType == stopPlay || requestType == startPlay)  // interestingly we must stop the queue for start (if running), as well as stop
        {
            if(currentIsRunning)
            {
                if(requestType == stopPlay) qDebug() << "Stopping on playing thread by stop request with queue running";
                else if(currentIsRunning) qDebug() << "Stopping on playing thread because it is running and we have a new play request";
                checkALSAreturn(snd_seq_stop_queue(handle, queue, NULL),"Stopping queue to reposition")
                drainQueue();
                // Note we leave it stopped until we get the first event below so as not to rush it if this code takes a while
                currentIsRunning = false;
            }
            else if(requestType == stopPlay) qDebug() << "Stop request on playing thread but queue is not running ";
            if(requestType == stopPlay) requestType = none;
            getQueueInfo();
        }

        if(requestType == startPlay)  // If we get here we should be stopped from above
        {
            qDebug() << "Playing on playing thread from measure " << m_startAtMeasure << ", volume=" << m_volumeScale << ", tempo=" << m_tempoScale;
            requestType = playing;  // Shift into playing mode after we finish the below

            // Hunt up any preliminary settings we have to send first.
            snd_seq_event_t prelimTempoEvent;    // Note we might not find any - this is LAST tempo change before we play
            bool tempoFound = false;
            startAtTick = 0;  // will adjust this when we've found the first measure, and use it as a skew
            for(std::map<int,midiPlayerV2::playableEvent_t>::iterator thisEvent = mParent->events.begin(); thisEvent != mParent->events.end(); thisEvent++ )
            {
                if(thisEvent->second.measureNum >= m_startAtMeasure) // If we are at start point, send any accumulated and leave queue running.
                {
                    playStartEvent = thisEvent;
                    qDebug() << "Starting queue since it is stopped and we are playing";
                    checkALSAreturn(snd_seq_start_queue(handle, queue, NULL),"Starting queue on first send returned error ");
                    drainQueue();
                    currentIsRunning = true;
                    startAtTick = thisEvent->second.snd_seq_event.time.tick;  // We'll scale everything else to this
                    if(tempoFound)
                    {
                        // schedule it forward to starting tick, which is always zero since queue starts at zero
                        qDebug() << "In Play thread, sending preliminary tempo event, value=" << (prelimTempoEvent.data.queue.param.value * 100 / m_tempoScale);
                        prelimTempoEvent.time.tick = 0;
                        prelimTempoEvent.data.queue.param.value = prelimTempoEvent.data.queue.param.value * 100 / m_tempoScale;  // Scale tempo if needed
                        #ifdef MUSICALPI_DEBUG_MIDI_SEND_DETAILS
                        qDebug() << "Sending prelim tempo to queue=" << prelimTempoEvent.queue
                                 << ", tick (incl offset)=" << prelimTempoEvent.time.tick
                                 << ", tempo(usec) = " << prelimTempoEvent.data.queue.param.value;
                        #endif
                        checkALSAreturn(snd_seq_event_output(handle,&prelimTempoEvent),"Send prelim tempo event")
                    }
                    break;   // Don't keep looping here
                }
                else if(thisEvent->second.containsTempo) // This is before our start, but we have to keep the LAST of these
                {
                    prelimTempoEvent = thisEvent->second.snd_seq_event;
                    tempoFound = true;
                }
            }
        }
        else if(requestType == playing)
        {
            // We keep playing from the next item after playStartEvent and increment it until done
            // Since we may scale things and don't want to screw with the saved data, copy the event
            snd_seq_event_t ep;
            ep = playStartEvent->second.snd_seq_event;
            if(playStartEvent->second.containsTempo)
            {
                qDebug() << "In Play thread, sending tempo event, value=" << (ep.data.queue.param.value * 100 / m_tempoScale);
                ep.data.queue.param.value = ep.data.queue.param.value * 100 / m_tempoScale;
            }
            if(playStartEvent->second.containsNoteOn)  ep.data.note.velocity = ep.data.note.velocity * m_volumeScale / 100;
            ep.time.tick = ep.time.tick - startAtTick; // offset for where we started queue (queue is always 0 start)
            #ifdef MUSICALPI_DEBUG_MIDI_SEND_DETAILS
            qDebug() << "Sending tick (w/offset)=" << ep.time.tick + startAtTick << ", source client=" << ep.source.client << ", dest client=" << ep.dest.client << ", type=" << ep.type;
            #endif
            checkALSAreturn(snd_seq_event_output(handle,&ep),"Send regular event from playables")
            playStartEvent++;
            drainQueue();
            if (playStartEvent == mParent->events.end())
            {
                requestType = none;
                drainQueue();
                getQueueInfo();
            }
            lastTickProcessed = ep.time.tick;
        }

        // If we are playing, but the queue is near full, put us into a voluntary wait state

        if(requestType == playing     && outputSize * MUSICALPI_ALSA_LOW_WATER / 100 >= outputFree)
        {
            requestType = playWaiting;
            qDebug() << "Waiting for queue to empty...";
        }
        if(requestType == playWaiting && outputSize * MUSICALPI_ALSA_HIGH_WATER / 100  < outputFree)
        {
            requestType = playing;
            qDebug() << "Restarting as queue reached high water mark...";
        }
        mutex.lock();  // Avoid race conditions in setting flags
        if(requestType != playing)   // Everything other than playing is a once-and-done
        {
            condition.wait(&mutex);  // if playing we just keep looping and not wait
        }
        mutex.unlock();
    }
}

void midiplayerV2Thread::drainQueue()  //  Call from main or worker thread but separately (procedurally interlocked)
{
    int ret;
    do
    {
        ret = snd_seq_drain_output(handle);
        assert(ret>=0);
    } while(ret>0);
}
