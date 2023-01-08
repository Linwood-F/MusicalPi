// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include <QTimer>
#include <QPainter>
#include <QStyleOption>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <cassert>

#include "piconstants.h"

#include "tipoverlay.h"
#include "mainwindow.h"
#include "oursettings.h"

TipOverlay::TipOverlay(QWidget* p, MainWindow * mp) : QLabel(p)
{
    // Instructional overlay (optionally) showed during play mode to show areas of the screen
    // where touches (clicks) turn pages or return to the library.

    qDebug() << "created";
    assert(p);
    assert(mp);
    mParent = mp;
    ourParent = p;
    overlayFade = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(overlayFade);
    overlayAnimation = new QPropertyAnimation(overlayFade,"opacity");
    overlayAnimation->setEasingCurve(QEasingCurve::Linear);
    overlayAnimation->setDuration(mParent->ourSettingsPtr->getSetting("overlayDuration").toInt() / 3);  // Must be <= first timer below

}
void TipOverlay::showEvent(QShowEvent* event)
{
    int overlayFullWidth = ourParent->size().width();
    int overlayTopHeight = mParent->ourSettingsPtr->getSetting("overlayTopPortion").toInt() * ourParent->size().height() / 100;
    int overlayPanelWidth = mParent->ourSettingsPtr->getSetting("overlaySidePortion").toInt() * overlayFullWidth / 100;
    QRect endPlayRect(0,0,overlayFullWidth,overlayTopHeight);
    QRect backPlayRect(0,overlayTopHeight,overlayPanelWidth,ourParent->size().height() - overlayTopHeight);
    QRect forwardPlayRect(overlayFullWidth - overlayPanelWidth,overlayTopHeight,overlayPanelWidth,ourParent->size().height() - overlayTopHeight);
    this->setGeometry(QRect(0,0,ourParent->geometry().width(),ourParent->geometry().height()));
    this->setAttribute(Qt::WA_TranslucentBackground);
    QPixmap tmpPix = QPixmap(ourParent->size());
    tmpPix.fill(Qt::transparent);
    QPainter painter(&tmpPix);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setOpacity(0.3);  // For panels
    painter.setBrush(QBrush(Qt::red));
    painter.drawRect(endPlayRect);
    painter.setBrush(QBrush(Qt::blue));
    painter.drawRect(backPlayRect);
    painter.setBrush(QBrush(Qt::green));
    painter.drawRect(forwardPlayRect);
    painter.setOpacity(1.0);
    painter.setFont(QFont("Arial",QString(MUSICALPI_SETTINGS_TIPOVERLAY_FONT_SIZE).replace("px","").toInt(),1,false));  // ??? This breaks if we don't use pixels -- ??
    painter.setPen(QColor("red"));
    painter.drawText(endPlayRect,Qt::AlignCenter,"End Play");
    painter.setPen(QColor("blue"));
    painter.drawText(backPlayRect,Qt::AlignCenter,"Prior Page");
    painter.setPen(QColor("green"));
    painter.drawText(forwardPlayRect,Qt::AlignCenter,"Next Page");
    qDebug() << "pixmap hasAlpha()=" << tmpPix.hasAlpha() << ", depth=" << tmpPix.depth()
             << ", size=(" << tmpPix.size().width() << "," << tmpPix.size().height() << ")";

    this->setPixmap(tmpPix);
    qDebug() << "TipOverlay:: create, after show, before animation start";

    overlayAnimation->setEasingCurve(QEasingCurve::Linear);
    overlayAnimation->setStartValue(0.0);
    overlayAnimation->setEndValue(1.0);
    overlayAnimation->start();
    QTimer::singleShot(mParent->ourSettingsPtr->getSetting("overlayDuration").toInt() / 2,this,  // Must be >= duration above
       [=]
        {
            qDebug() << "TipOverlay:: in animation after first event";
            overlayAnimation->setEndValue(0.0);
            overlayAnimation->setStartValue(1.0);
            overlayAnimation->start();
        }
    );
    QTimer::singleShot(mParent->ourSettingsPtr->getSetting("overlayDuration").toInt(),this,  // Must be >= 2* duration above
       [=]
        {
            qDebug() << "TipOverlay:: in animation after second event";
            this->hide();
         }
    );
    QLabel::showEvent(event);
}
TipOverlay::~TipOverlay()
{
    qDebug() << "TipOverlay:: in destructor";
    if(overlayAnimation) delete overlayAnimation;
    if(overlayFade) delete overlayFade;
}

// Note the replacement paint event used in other routines cannot be used here -- probably needs something more sophisticated.

