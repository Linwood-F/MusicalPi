#ifndef DOCPAGELABEL_H
#define DOCPAGELABEL_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QTimer>
#include <QtDebug>
#include <QLabel>


class MainWindow;

class docPageLabel : public QLabel
{
    Q_OBJECT

public:
    enum docTransition {noTransition, halfPage, fullPage, fullPageNow};
        // noTransition - display immediately, no transition at all
        // halfPage     - display top half immediately, rest after delay, highlight briefly
        // fullPage     - display nothing immediately, replace after delay, highlight briefly
        // fullPageNow  - display immediately but highlight whole page briefly
    docPageLabel(QWidget *parent, MainWindow* mp);
    ~docPageLabel();
    void placeImage(docTransition thisTransition, QString color);
    void placeImage(docTransition thisTransition, QImage *newImageBuffer, QString color);
    void HideAnyInProgressTransitions();
    QTimer ourOverlayTimer;
    QTimer ourHighlightShowTimer;  // for full page or first half of half page
    QTimer ourHighlightHideTimer;
    QTimer our2ndHighlightShowTimer;  // for second half of half page
    QTimer our2ndHighlightHideTimer;
    QWidget* ourParent;
    MainWindow* mParent;

private:
    QLabel ourOverlay;
    QLabel ourHighlightOverlay;
    QLabel our2ndHighlightOverlay;
    bool oldImageIsBlank;
    bool newImageIsBlank;
    int pageTurnDelay;
    int pageHighlightDelay;
    int pageHighlightHeight;

};

#endif // DOCPAGELABEL_H
