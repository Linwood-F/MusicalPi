#ifndef DEBUGMESSAGES_H
#define DEBUGMESSAGES_H

#include <qapplication.h>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);


#endif // DEBUGMESSAGES_H
