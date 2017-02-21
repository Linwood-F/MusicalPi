#ifndef TIPOVERLAY_H
#define TIPOVERLAY_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

//#include <QObject>
#include <QLabel>
#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

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
    void paintEvent(QPaintEvent *);
};

#endif // TIPOVERLAY_H
