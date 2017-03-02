#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include "piconstants.h"

#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
#include <poppler/qt5/poppler-qt5.h>
#endif

class MainWindow;
class PDFDocument;

class renderThread : public QThread
{
    Q_OBJECT

public:
    renderThread(PDFDocument *parent=0, int which = -1, MainWindow* mp = 0);   // which is index of thread so we can find it on return
    ~renderThread();
    void render(QImage** image, int thisPage, int maxWidth, int maxHeight);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void renderedImage(int which, int thePage, int maxWidth, int maxHeight);

private:
    bool abort;
    bool running;
    QMutex mutex;             // Used only for wait condition and restart below, not used for critical sections
    QWaitCondition condition; // Associated with the above to control queue wait/run state
    QImage** targetImagePtr;

    float theScale;
    PDFDocument* ourParent;  // Pointer to PDF document
    MainWindow* mParent; // Pointer to main window
    int mWhich;   // which containing thread called us (so on slot we can suitably adjust
    int mPage;
    int mWidth;
    int mHeight;
    int pageHighlightHeight;
#ifdef MUSICALPI_OPEN_DOC_IN_THREAD
    Poppler::Document* document;   // Document (or null) - this is experimental to see if opening the document in the thread is better
#endif

};

#endif // RENDERTHREAD_H
