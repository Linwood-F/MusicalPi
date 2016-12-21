#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H
#include <string>
#include <poppler/qt5/poppler-qt5.h>

#include <QImage>
#include <QLabel>
#include <QThread>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include <cassert>
#include <QtDebug>
#include <QPainter>
#include <QBitmap>
#include <stdio.h>
#include <QColor>
#include <QMutex>

#include "piconstants.h"
#include "docpagelabel.h"
#include "renderthread.h"

class PDFDocument : public QObject
{
    Q_OBJECT

public:
    PDFDocument(QString filepath);
    ~PDFDocument();
    void getOnePage(docPageLabel * where, int pageToGet, docPageLabel::docTransition); // Starting page 1
    QString filepath;
    int numPages;  // Number of pages in document
    int imageWidth;  // Current width of image window we are using (adjusted for roomForMenu if needed)
    int imageHeight;   // Current height of image window we are using
    Poppler::Document* document;   // Document (or null)
    void checkResetImageSize(int width, int height);
    QImage *pageImages[MUSICALPI_MAXPAGES];
    bool pageImagesAvailable[MUSICALPI_MAXPAGES]; // do not use image unless true
    void checkCaching();
    void adjustCache(int leftmostPage);
    QMutex PDFMutex;

private:


    // These are the threads we use for caching - these variables are managed ONLY from this thread
    renderThread *pageThreads[MUSICALPI_THREADS];
    bool pageThreadActive[MUSICALPI_THREADS];
    int pageThreadPageLoading[MUSICALPI_THREADS];

    // Target range for cache (it's a moving target so may or may not actually be present)
    int cacheRangeStart;  // Beginning page (ref 1)
    int cacheRangeEnd;    // End page (ref 1)

signals:
    void newImageReady();

private slots:
    void updateImage(int which, int page, int maxWidth, int maxHeight);

};

#endif // PDFDOCUMENT_H
