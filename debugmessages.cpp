// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "debugmessages.h"
#include <QTime>
#include <QMutex>
#include <QRegularExpression>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QTime t;
    static int msElapsed = -1;
    int msThisTime = 0;
    QMutex mutex;

    mutex.lock();
    if(msElapsed<0)
    {
        t.start();
        msElapsed=0;
        msThisTime=0;
        fprintf(stderr, "Elapsed - this : Message (Times are in milliseconds, elapsed time not CPU\n");
    }
    else
    {
        msThisTime = t.elapsed() - msElapsed;
        msElapsed = t.elapsed();
    }
    mutex.unlock();

    QByteArray localMsg = msg.toLocal8Bit();
    QString qfunc = context.function;
    qfunc.remove(QRegularExpression("\\(.+\\).*"));
    std::string sfunc(qfunc.toStdString());
    const char* func = sfunc.c_str();
    switch (type)
    {
        case QtDebugMsg:
            fprintf(stderr, "%'9d - %'6d : Debug: %s (%s)\n", msElapsed, msThisTime, func, localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(stderr, "%'9d - %'6d : Info: %s:%u, %s (%s)\n", msElapsed, msThisTime, context.file, context.line, func, localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "%'9d - %'6d : Warning: %s:%u, %s (%s)\n", msElapsed, msThisTime, context.file, context.line, func, localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "%'9d - %'6d : Critical: %s:%u, %s (%s)\n", msElapsed, msThisTime, context.file, context.line, func, localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "%'9d - %'6d : Fatal: %s:%u, %s (%s)\n", msElapsed, msThisTime, context.file, context.line, func, localMsg.constData());
            abort();
    }
}

