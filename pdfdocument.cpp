// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QPainter>
#include <QFileInfo>
#include <QThread>

#include "pdfdocument.h"
#include "mainwindow.h"
#include "docpagelabel.h"
#include "renderthread.h"
#include "mainwindow.h"
#include "oursettings.h"
#include "piconstants.h"

#include <string>
#include <stdio.h>

//  PDFDOcument - responsible for managing the document iself
//
//  This also manages the image buffers needed for display, deciding which
//  to pro-actively fetch and cache, which to remove if memory is short
//
//  It formats the page for display as well, based on dimensions of a passed-in
//  QLabel in which to display it, pre-sized to the maximum.


double degree2radian(int d) { return (double)d * 3.1415926535 / 180.0 ;}

PDFDocument::PDFDocument(MainWindow* parent, QString _filePath, QString _titleName) : QObject(parent)
{
    qDebug() << "initializing with " << _filePath;
    mParent = parent;
    filepath = _filePath;
    titleName = _titleName;
    imageWidth = 0;
    imageHeight = 0;
    totalPagesRequested = totalPagesRendered = 0;
    for(int i=0; i<MUSICALPI_MAXPAGES; i++)
    {
        pageImagesAvailable[i] = false;  // pages will start at 1 but stored at index 0, so using 0 for page number means empty
    }
    for(int i=0; i<MUSICALPI_THREADS; i++)
    {
        pageThreads[i] = new renderThread(this, i, mParent);
        connect(pageThreads[i], SIGNAL(renderedImage(int, int, int, int)),
                this,               SLOT(updateImage(int, int, int, int)));
        pageThreadActive[i]=false;
        pageThreadPageLoading[i]=0;
    }
    if(filepath.endsWith(".pdf",Qt::CaseInsensitive))
    {
        midiFilePath = filepath;
        midiFilePath.replace(filepath.length() - 4,4,".mid");  // tentative path
        QFileInfo check_file(midiFilePath);
        if(!check_file.exists() || !check_file.isFile()) midiFilePath = ""; // if not there, just blank it out to tell others
    }
    else midiFilePath = "";

    document = Poppler::Document::load(filepath);
    document->setRenderBackend(MUSICALPI_POPPLER_BACKEND);
    assert(document && !document->isLocked());
    numPages = document->numPages();   // Count of pages in document
#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
    DELETE_LOG(document); // If we are letting the thread render we don't need this any more, close it
#endif

    assert(numPages <= MUSICALPI_MAXPAGES);
    cacheRangeStart = 1;  // Start at the beginning, then adjust as we get asked for images
    maxCache = cacheRangeEnd = mParent->ourSettingsPtr->getSetting("maxCache").toInt();


//        Poppler::Page *p = document->page(3);
//        double radius  = 0.3;
//        for(int angle = 0; angle<360; angle += 30)
//        {
//            // test annotation
//            Poppler::LineAnnotation *ia = new
//            Poppler::LineAnnotation(Poppler::LineAnnotation::StraightLine);
//            QLinkedList<QPointF> points;
//            points.push_back(QPointF(0.5 + std::cos(angle) * radius, 0.5 + std::sin(angle) * radius));
//            points.push_back(QPointF(0.5 + std::cos(angle + 12) * radius, 0.5 + std::sin(angle + 12) * radius));
//            ia->setLinePoints(points);
//            Poppler::Annotation::Style style;
//            style.setColor(QColor(0, 0, 0));
//            style.setWidth(1.0);
//            ia->setStyle(style);
//            p->addAnnotation(ia);
//        }
   checkCaching();
}

PDFDocument::~PDFDocument()
{
    qDebug() << "In destructor ";
    for(int i=0; i<MUSICALPI_THREADS; i++)
    {
        DELETE_LOG(pageThreads[i]);
    }
    for (int i = 0; i<MUSICALPI_MAXPAGES; i++)
    {
        lockOrUnlockMutex(true);
        if (pageImagesAvailable[i])
        {
            DELETE_LOG(pageImages[i]);
            pageImagesAvailable[i]=false;
        }
        lockOrUnlockMutex(false);
    }
    DELETE_LOG(document);
}

// Slot
void PDFDocument::updateImage(int which, int page, int maxWidthUsed, int maxHeightUsed)
{
    // This just records the returned image it doesn't display it itself
    pageImagesAvailable[page - 1] = true;
    if(maxWidthUsed < imageWidth || maxHeightUsed < imageHeight)
    {
        qDebug() << "Received image is too small, discarding, [" << maxWidthUsed << "," << maxHeightUsed << "] vs [" << imageWidth << "," << imageHeight << "]";
        delete pageImages[page - 1];
        pageImagesAvailable[page - 1] = false;
    }
    // This thread is available
    pageThreadActive[which] = false;
    pageThreadPageLoading[which] = 0;
    totalPagesRendered++;
    checkCaching();
    emit newImageReady();  // ask parent to display anything we got (it checks everything so it should be OK even if we rejected this one)
}

void PDFDocument::checkCaching()
{
    // A fundamental assumption is that the caching holds a window surrounding the pages needed
    // since the parent routines do not themselves ask for a specific page of the cache management,
    // they just expect them to show up,.
    //
    // Given soime priority to forward rather than backward so 2 forward for each back
    //
    if(imageWidth == 0 || imageHeight == 0)
    {
        qDebug() << "exiting without checking cache as we haven't calculated window sizes";
        return;
    }
    // In theory we ought to start in a biased (forward) middle working out for any given time
    // later but the most important pages are the first few.  From then on we just assume we will
    // keep up.

    lockOrUnlockMutex(true); // We don't want to delete something half delivered so lock out thread; this is a bit broad but this section is pretty fast.
    for(int i=0; i<numPages; i++)
    {
        if(i+1 < cacheRangeStart || i+1 > cacheRangeEnd)  // outside of caching range
        {
            if(pageImagesAvailable[i])
            {
                qDebug() << "Removing page " << i + 1 << " from cache as expired.";
                delete pageImages[i];
                pageImagesAvailable[i]=false;
            }
        }
        else // inside of caching range
        {
            if(!pageImagesAvailable[i]) // if we don't have the page cached hunt for it loading and/or room to start (no interlock since if it's half-loaded and true not false we just skip it)
            {
                bool found=false;
                int availableThread = -1;
                for(int t=0; t < MUSICALPI_THREADS; t++)
                {
                    if (pageThreadPageLoading[t] == i+1) // Already doing this page
                    {
                        found = true;  // but don't "continue" as we are also looking for available threads
                    }
                    if(!pageThreadActive[t]) availableThread = t;
                }
                if(!found) // page is needed and not found so try to run with it
                {
                    // The second half of this convoluted check is only to affect the very first "up"
                    // pages, so we request no more than "up" until we get at least "up", so the user does not
                    // wait as long; from then on we just do as many as threads available
                    if(availableThread != -1 && (totalPagesRequested >  mParent->pagesNowAcross * mParent->pagesNowDown ||
                                                 totalPagesRequested <  mParent->pagesNowAcross * mParent->pagesNowDown ||
                                                 totalPagesRendered  >= mParent->pagesNowAcross * mParent->pagesNowDown) ) // We have room and aren't waiting on first "up"
                    {
                        totalPagesRequested++;
                        pageThreadPageLoading[availableThread]=i+1;
                        pageThreadActive[availableThread]=true;
                        //qDebug() << "Trying to render page " << i+1 << "on thread " << availableThread;
                        pageThreads[availableThread]->render(&pageImages[i],i+1,imageWidth,imageHeight);
                    }
                    else // we don't have available threads (and because !found we looked the whole way)
                    {
                        //qDebug() << "Need to load page " << i + 1 << " but no threads (or first set).";
                        break;  // this will break the scan of pages (i)
                    }
                } // we found it so just keep looking for another in outer loop
                // else qDebug() << "Checking page " << i+1 << "showing it is loading now";
            }
            //else qDebug() << "Checking page " << i+1 << " showing available.";
        }
    }
    lockOrUnlockMutex(false);
}

void PDFDocument::adjustCache(int leftmostPage)
{
    // This seems to do the same calculation twice, but we want to extend anything outside of the normal range
    // to the other side if we hit one end.

    int nominalStart = std::max(1,std::min(numPages, leftmostPage - (int)(0.33 * maxCache)));
    int nominalEnd    = std::max(1,std::min(numPages, nominalStart + maxCache - 1));
    nominalStart  = std::max(1,std::min(numPages, nominalEnd  - maxCache + 1));
    if(nominalStart != cacheRangeStart || nominalEnd != cacheRangeEnd)
    {
        qDebug() << "With leftmostpage as " << leftmostPage << " cache changed from [" << cacheRangeStart << "," << cacheRangeEnd << "] to ["<< nominalStart << "," << nominalEnd << "]";
        cacheRangeStart = nominalStart;
        cacheRangeEnd = nominalEnd;
    }
    checkCaching();
}

void PDFDocument::checkResetImageSize(int width, int height)
{
    if(width == imageWidth && height == imageHeight)
    {
        qDebug() << "Image sizes match, returning with nothing to do";
        return;
    }
    // First just reset future size to the new size
    imageWidth = std::max(width,imageWidth);
    imageHeight = std::max(height,imageHeight);
    // Now see if any are smaller on BOTH dimensions (if longer/long-as on one we assume we are OK)

    qDebug() << "Reset cache, looking to see if any images need to go, needed = [" << width << "x" << height << "]";
    lockOrUnlockMutex(true);
    for(int i = 0; i < MUSICALPI_MAXPAGES; i++)
        if(pageImagesAvailable[i] && pageImages[i]->width() < width && pageImages[i]->height() < height)
        {
                qDebug() << "Discarding pageImages[" << i << "] with size [" << pageImages[i]->width() << "x" << pageImages[i]->height() << "]";
                delete pageImages[i];
                pageImagesAvailable[i]=false;
        }
    lockOrUnlockMutex(false);
    checkCaching();
}
void PDFDocument::lockOrUnlockMutex(bool lockFlag)
{
    // Put this in a separate place so we can add debugging or other instrumentation if needed, but
    // it is called too frequently to debug output all the time.
    if(lockFlag)
    {
//        qDebug()<< "Locking mutex called from thread " << QThread::currentThreadId();
        PDFMutex.lock();
    }
    else
    {
//        qDebug()<< "UnLocking mutex called from thread " << QThread::currentThreadId();
        PDFMutex.unlock();
    }
}
