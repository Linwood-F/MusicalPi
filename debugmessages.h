#ifndef DEBUGMESSAGES_H
#define DEBUGMESSAGES_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <qapplication.h>
#include <QDebug>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);


#endif // DEBUGMESSAGES_H
