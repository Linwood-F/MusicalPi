#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QWaitCondition>
#include <QDebug>

#include <cassert>
#include "poppler/qt5/poppler-qt5.h"

class renderThread : public QThread
{
    Q_OBJECT

public:
    renderThread(QObject *parent=0, int which = -1);   // which is index of thread so we can find it on return
    ~renderThread();
    void render(QImage** image, int thisPage, int maxWidth, int maxHeight);

protected:
    void run() Q_DECL_OVERRIDE;

signals:
    void renderedImage(int which, int thePage, int maxWidth, int maxHeight);

private:
    bool abort;
    bool running;
    QMutex mutex;
    QWaitCondition condition;
    QImage** targetImagePtr;

    float theScale;
    QObject* mParent;  // can't define as explicit pdfdocument due to header load sequences
    int mWhich;   // which containing thread called us (so on slot we can suitably adjust
    int mPage;
    int mWidth;
    int mHeight;

};

#endif // RENDERTHREAD_H
