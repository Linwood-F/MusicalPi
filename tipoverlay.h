#ifndef TIPOVERLAY_H
#define TIPOVERLAY_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QObject>
#include <QLabel>
#include <QTimer>
#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <cassert>

#include "piconstants.h"

class MainWindow;

class TipOverlay : public QLabel
{
    Q_OBJECT

private:
    QPropertyAnimation* overlayAnimation;
    QGraphicsOpacityEffect* overlayFade;
    QWidget* ourParent; // parent;
    MainWindow* mParent;

public:
    TipOverlay(QWidget* p, MainWindow* mparent);  // Call with the widget over which to overlay
    ~TipOverlay();
    void showEvent(QShowEvent* event);
};

#endif // TIPOVERLAY_H
