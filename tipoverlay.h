#ifndef TIPOVERLAY_H
#define TIPOVERLAY_H

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

class TipOverlay : public QLabel
{
    Q_OBJECT

private:
    QPropertyAnimation* overlayAnimation;
    QGraphicsOpacityEffect* overlayFade;
    QWidget* p; // parent;

public:
    TipOverlay(QWidget* parent);  // Call with the widget over which to overlay
    ~TipOverlay();
    void showEvent(QShowEvent* event);
};

#endif // TIPOVERLAY_H
