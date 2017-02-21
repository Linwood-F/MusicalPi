#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QThread>
#include <QMutex>
#include <QImage>
#include <QWaitCondition>


class MainWindow;
class renderThread : public QThread
{
    Q_OBJECT

public:
    renderThread(QObject *parent=0, int which = -1, MainWindow* mp = 0);   // which is index of thread so we can find it on return
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
    QObject* ourParent;  // Pointer to PDF document
    MainWindow* mParent; // Pointer to main window
    int mWhich;   // which containing thread called us (so on slot we can suitably adjust
    int mPage;
    int mWidth;
    int mHeight;

};

#endif // RENDERTHREAD_H
