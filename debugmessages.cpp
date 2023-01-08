// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include "debugmessages.h"
#include <QTime>
#include <QMutex>
#include <QRegularExpression>
#include <stdio.h>
#include <stdlib.h>
#include <exception>


void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Note this just won't be accurate in terms of time across midnight but who cares it's just debug output.

    static bool initdone;
    static int msStartTime;
    static int msLastTime;
    int msElapsed;
    int msThisTime;
    QTime t;
    static QMutex mutex;
    const static QRegularExpression re = QRegularExpression("\\(.+\\).*");

    mutex.lock();
    if(!initdone)
    {
        t = QTime::currentTime();
        msElapsed=0;
        msThisTime=0;
        msLastTime = t.msecsSinceStartOfDay();
        msStartTime = msLastTime;
        fprintf(stderr, "Elapsed - this : Message (Times are in milliseconds, elapsed time not CPU\n");
        initdone=1;
    }
    else
    {
        t = QTime::currentTime();
        msThisTime = t.msecsSinceStartOfDay() - msLastTime;
        msElapsed  = t.msecsSinceStartOfDay() - msStartTime;
        msLastTime = t.msecsSinceStartOfDay();
    }
    mutex.unlock();

    QByteArray localMsg = msg.toLocal8Bit();
    QString qfunc = context.function;
    qfunc.remove(re);
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

