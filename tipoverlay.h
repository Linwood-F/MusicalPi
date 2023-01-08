#ifndef TIPOVERLAY_H
#define TIPOVERLAY_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include <QLabel>

class MainWindow;
class QPropertyAnimation;
class QGraphicsOpacityEffect;

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
