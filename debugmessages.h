#ifndef DEBUGMESSAGES_H
#define DEBUGMESSAGES_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include "qdatetime.h"
#include <QDebug>

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif // DEBUGMESSAGES_H
