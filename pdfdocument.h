#ifndef PDFDOCUMENT_H
#define PDFDOCUMENT_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include <QMutex>

#include "docpagelabel.h"
#include "piconstants.h"
#include <poppler/qt6/poppler-qt6.h>

#include <cassert>

class MainWindow;
class renderThread;

class PDFDocument : public QObject
{
    Q_OBJECT

public:
    PDFDocument(MainWindow* parent, QString filepath, QString _titleName);
    ~PDFDocument();
    void getOnePage(docPageLabel * where, int pageToGet, docPageLabel::docTransition); // Starting page 1
    QString filepath;
    QString midiFilePath;  // As a convenience this object checks if there is a midi file as well
    QString titleName;
    int numPages;  // Number of pages in document
    int imageWidth;  // Current width of image window we are using (adjusted for roomForMenu if needed)
    int imageHeight;   // Current height of image window we are using
    std::unique_ptr<Poppler::Document> document;   // Document (or null)
    void checkResetImageSize(int width, int height);
    QImage *pageImages[MUSICALPI_MAXPAGES];
    bool pageImagesAvailable[MUSICALPI_MAXPAGES]; // do not use image unless true
    void checkCaching();
    void adjustCache(int leftmostPage);
    QMutex PDFMutex;
    void lockOrUnlockMutex(bool lockFlag);
    MainWindow* mParent;

private:


    // These are the threads we use for caching - these variables are managed ONLY from this thread
    renderThread *pageThreads[MUSICALPI_THREADS];
    bool pageThreadActive[MUSICALPI_THREADS];
    int pageThreadPageLoading[MUSICALPI_THREADS];

    // Target range for cache (it's a moving target so may or may not actually be present)
    int cacheRangeStart;  // Beginning page (ref 1)
    int cacheRangeEnd;    // End page (ref 1)
    int maxCache;
    int totalPagesRendered; // Used as a slight optimization, we only use "up" pages until we have rendered that many so the first "up" come up fast.
    int totalPagesRequested; // as above.
signals:
    void newImageReady();

private slots:
    void updateImage(int which, int page, int maxWidth, int maxHeight);

};

#endif // PDFDOCUMENT_H
