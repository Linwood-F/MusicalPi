// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QPainter>
#include <QDebug>

#include "renderthread.h"
#include "pdfdocument.h"
#include "mainwindow.h"
#include "oursettings.h"

#include <cassert>
#include <cmath>

#include "piconstants.h"
#include "poppler/qt5/poppler-qt5.h"

renderThread::renderThread(PDFDocument *parent, int which, MainWindow* mp ): QThread(parent)
{
    // This routine is used for actual graphics page rendering, in separate threads

    // Note constructor is running in the parent thread

    qDebug() << "in constructor for thread " << which;
    abort = false;
    running = false;   // Used as a code logic sanity check, this does not control the loop, just aborts if inconsistent
    ourParent = parent; // the PDF object
    mParent = mp;    // MainWindow object
    mWhich = which;   // the number of the thread array this is attached to
    mWidth = 0;       // the target width we were asked to scale to (don't do anything with it zero)
    mHeight = 0;      // the target height we were asked to scale to (don't do anything with it zero)
    pageHighlightHeight = mParent->ourSettingsPtr->getSetting("pageHighlightHeight").toInt();
    mutex.unlock();   // make sure we are in an unlocked state
}

renderThread::~renderThread()
{
    qDebug() << "In destructor";
    abort=true;
    condition.wakeOne();
    qDebug() << "Entering wait";
    wait();  // Since constructor/destructor are in the parent thread, this waits for the worker thread to exit before the base class destructor is called;
    mutex.unlock(); // Why is this needed?   Otherwise it gives warnings
    qDebug() << "Wait finished, leaving destructor";
}

void renderThread::render(QImage** image, int thePage, int maxWidth, int maxHeight)
{
    // This is also run in the parent thread -- it starts a render

    // Note that we never do another render call until any running one is done, so in
    // theory here running has to be false, check it with the assert, since if it fails we just screwed up the logic.
    mutex.lock(); // just in case
    assert(!running);

    // Our local variables (because of the above) ar valid in thread and parent, since we synchronize with the parent's
    // tracking of running state (not the "running" variable) so we don't need a mutex lock here.

    targetImagePtr = image;
    mPage = thePage;
    mWidth = maxWidth;
    mHeight = maxHeight;

    if(this->isRunning()) condition.wakeOne(); // thread is sleeping (since !running) so wake it up.
    else start(QThread::LowPriority);          // first time through start thread
    mutex.unlock();
    return;
}

void renderThread::run()
{
    forever
    {
        running = true;
#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
        qDebug()<<"Opening PDF document inside of thread now " << ourParent->filepath;
        document = Poppler::Document::load(ourParent->filepath);
        document->setRenderBackend(MUSICALPI_POPPLER_BACKEND);
        assert(document && !document->isLocked());
        Poppler::Page* tmpPage = document->page(mPage - 1);
#else
        Poppler::Page* tmpPage = ((PDFDocument*)ourParent)->document->page(mPage - 1);
#endif
        assert(tmpPage!=NULL);
        QSizeF thisPageSize = tmpPage->pageSizeF();  // in 72's of inch
        double scaleX = (double)mWidth / ((double)thisPageSize.width() / (double)72.0);
        double scaleY = (double)mHeight / ((double)thisPageSize.height() / (double)72.0);
        double desiredScale = std::trunc(std::min(scaleX, scaleY));  // For notational scores integers seem to give better alignment, sometimes.

        qDebug() << "Starting render on thread " << mWhich << " id " << currentThreadId() << " for page " << mPage << ", pt size " << thisPageSize.width() << "x" << thisPageSize.height() << " at scale " << desiredScale << " targeting " << mWidth << "x" << mHeight;
#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
        document->setRenderHint(Poppler::Document::Antialiasing, true);    // Note you can't ignore paper color as some PDF's apparently come up black backgrounds
        document->setRenderHint(Poppler::Document::TextAntialiasing, true);
        document->setRenderHint(Poppler::Document::TextHinting, false);
        document->setRenderHint(Poppler::Document::OverprintPreview, false);
        document->setRenderHint(Poppler::Document::ThinLineSolid,true);

#else
        // Note you can't ignore paper color as some PDF's apparently come up black backgrounds
        ourParent->document->setRenderHint(Poppler::Document::Antialiasing,true);
        ourParent->document->setRenderHint(Poppler::Document::TextAntialiasing,true);
        ourParent->document->setRenderHint(Poppler::Document::TextHinting, false);
        ourParent->document->setRenderHint(Poppler::Document::OverprintPreview, false);
        ourParent->document->setRenderHint(Poppler::Document::ThinLineSolid,true);
#endif
        QImage* theImage = new QImage(tmpPage->renderToImage(desiredScale,desiredScale));
        assert(theImage);
        qDebug() << "Page " << mPage << " was rendered on thread " << mWhich << " produced size " << theImage->width() << "x" << theImage->height();

        { // Put in a block so it will remove the painter and not leave it attached to the passed-out QImage
            QPainter painter(theImage);
            painter.setFont(QFont("Arial", QString(MUSICALPI_SETTINGS_PAGENUMBER_FONT_SIZE).replace("px","").toInt(), 1, false));  // This breaks if we aren't using pixels ???
            painter.setPen(QColor("green"));
            painter.drawText(QPoint(pageHighlightHeight + 10,pageHighlightHeight + 20),QString("%1").arg(mPage)); // extra space is room for number, in addition to highlight
        }
        // critical section:  This interlock is with the parent thread for returning the image; the parent records we are not running afterwards
        ourParent->lockOrUnlockMutex(true);
#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
        DELETE_LOG(document);  // also closes this copy
#endif
        *targetImagePtr = theImage;
        emit renderedImage( mWhich, mPage, mWidth, mHeight);
        mutex.lock();
        running = false;    // Mark we were done - note we do this inside parent's critical section AND this thread's to prevent race conditions
        ourParent->lockOrUnlockMutex(false);
        // End of critical section
        if(abort) { qDebug() << "Returning with abort"; return; }  // will hit this one first if requested while running but
        condition.wait(&mutex);
        mutex.unlock();
        if(abort) { qDebug() << "Returning with abort"; return; };   // will hit this one if requested while not running
    }
}
