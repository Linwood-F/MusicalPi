// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "renderthread.h"
#include "pdfdocument.h"   // has to be here not header as it needs renderThread defined

renderThread::renderThread(QObject *parent, int which ): QThread(parent)
{
    // Modeled from http://doc.qt.io/qt-5/qtcore-threads-mandelbrot-example.html
    // Note constructor is in the parent thread
    qDebug() << "in constructor for thread " << which;
    abort = false;
    running = false;
    mParent = parent; // the PDF object
    mWhich = which;   // the number of the thread array this is attached to
    mWidth = 0;       // the target width we were asked to scale to (don't do anything with it zero)
    mHeight = 0;      // the target height we were asked to scale to (don't do anything with it zero)
}

renderThread::~renderThread()
{
    qDebug() << "In destructor";
    mutex.lock();
    abort=true;
    condition.wakeOne();
    mutex.unlock();
    wait();  // Since constructor/destructor are in the parent thread, this waits for the worker thread to exit before the base class destructor is called;
}

void renderThread::render(QImage** image, int thePage, int maxWidth, int maxHeight)
{
    // This is also run in the parent thread

    // Note that we attempt to never do another render call until this one is done, so in
    // theory here running has to be false.
    mutex.lock();
    assert(!running);
    // Our local variables (because of the above) ar valid in thread and parent, but we have to copy to the
    // local instance, and not depend on these parameters.
    targetImagePtr = image;
    mPage = thePage;
    mWidth = maxWidth;
    mHeight = maxHeight;

    if(!isRunning())
    {
        qDebug() << "Starting new thread " << mWhich;
        start(LowPriority);
    }
    else
    {
        qDebug() << "Waking up thread " << mWhich;
        condition.wakeOne();
    }
    mutex.unlock();
    return;
}

void renderThread::run()
{
    forever
    {
        running = true;
        if(abort) return;

        Poppler::Page* tmpPage = ((PDFDocument*)mParent)->document->page(mPage - 1);  // Document starts at page 0, we use 1
        assert(tmpPage!=NULL);
        QSizeF thisPageSize = tmpPage->pageSizeF();  // in 72's of inch
        float scaleX = mWidth / (thisPageSize.width() / 72.0);
        float scaleY = mHeight / (thisPageSize.height() / 72.0);
        float desiredScale = std::min(scaleX, scaleY);
        qDebug() << "Starting render on thread " << mWhich << " for page " << mPage << ", pt size " << thisPageSize.width() << "x" << thisPageSize.height() << " at scale " << desiredScale << " targeting " << mWidth << "x" << mHeight;
        QImage* theImage = new QImage(tmpPage->renderToImage(desiredScale,desiredScale));
        assert(theImage);
        qDebug() << "Page " << mPage << " was rendered on thread " << mWhich << " produced size " << theImage->width() << "x" << theImage->height();

        { // Put in a block so it will remove the painter and not leave it attached to the passed-out QImage
            QPainter painter(theImage);
            painter.setFont(QFont("Arial", 16, 1, false));
            painter.setPen(QColor("green"));
            painter.drawText(QPoint(MUSICALPI_PAGE_HIGHLIGHT_HEIGHT + 10,MUSICALPI_PAGE_HIGHLIGHT_HEIGHT + 20),QString("%1").arg(mPage)); // extra space is room for number, in addition to highlight
        }
        mutex.lock();  // Avoid race conditions in the parent thread in the render() that controls this loop
        running = false;
        ((PDFDocument*)mParent)->PDFMutex.lock();
        *targetImagePtr = theImage;
        emit renderedImage( mWhich, mPage, mWidth, mHeight);
        ((PDFDocument*)mParent)->PDFMutex.unlock();
        if(abort) return;
        condition.wait(&mutex);
        mutex.unlock();  // If the above waits, how do we ever get here if the awake above is also inside a mutex lock???
    }
}
