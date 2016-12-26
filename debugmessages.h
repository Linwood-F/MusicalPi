#ifndef DEBUGMESSAGES_H
#define DEBUGMESSAGES_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <qapplication.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);


#endif // DEBUGMESSAGES_H
