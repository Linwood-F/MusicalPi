#ifndef DOCPAGELABEL_H
#define DOCPAGELABEL_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QImage>
#include <QThread>
#include <QTimer>
#include <QtDebug>
#include <QPainter>
#include <QBitmap>
#include <QColor>

#include <cassert>

#include "piconstants.h"

class docPageLabel : public QLabel
{
    Q_OBJECT

public:
    enum docTransition {noTransition, halfPage, fullPage, fullPageNow};
        // noTransition - display immediately, no transition at all
        // halfPage     - display top half immediately, rest after delay, highlight briefly
        // fullPage     - display nothing immediately, replace after delay, highlight briefly
        // fullPageNow  - display immediately but highlight whole page briefly
    docPageLabel(QWidget *parent);
    ~docPageLabel();
    void placeImage(docPageLabel::docTransition thisTransition, QColor color);
    void placeImage(docTransition thisTransition, QImage *newImageBuffer, QColor color);
    void HideAnyInProgressTransitions();
    QTimer ourOverlayTimer;
    QTimer ourHighlightShowTimer;  // for full page or first half of half page
    QTimer ourHighlightHideTimer;
    QTimer our2ndHighlightShowTimer;  // for second half of half page
    QTimer our2ndHighlightHideTimer;

private:
    QLabel ourOverlay;
    QLabel ourHighlightOverlay;
    QLabel our2ndHighlightOverlay;
    bool oldImageIsBlank;
    bool newImageIsBlank;
};

#endif // DOCPAGELABEL_H
