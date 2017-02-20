// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "tipoverlay.h"
#include "mainwindow.h"


TipOverlay::TipOverlay(QWidget* p, MainWindow * mp) : QLabel(p)
{
    qDebug() << "created";
    assert(p);
    assert(mp);
    mParent = mp;
    ourParent = p;
    overlayFade = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(overlayFade);
    overlayAnimation = new QPropertyAnimation(overlayFade,"opacity");
    overlayAnimation->setEasingCurve(QEasingCurve::Linear);
    overlayAnimation->setDuration(mParent->ourSettingsPtr->overlayDuration / 3);  // Must be <= first timer below

}
void TipOverlay::showEvent(QShowEvent* event)
{
    int overlayFullWidth = ourParent->size().width();
    int overlayTopHeight = mParent->ourSettingsPtr->overlayTopPortion * ourParent->size().height();
    int overlayPanelWidth = mParent->ourSettingsPtr->overlaySidePortion * overlayFullWidth;
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
    painter.setFont(QFont("Arial",MUSICALPI_SETTINGS_TIPOVERLAY_FONT_SIZE,1,false));
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
    QTimer::singleShot(mParent->ourSettingsPtr->overlayDuration / 2,this,  // Must be >= duration above
       [=]
        {
            qDebug() << "TipOverlay:: in animation after first event";
            overlayAnimation->setEndValue(0.0);
            overlayAnimation->setStartValue(1.0);
            overlayAnimation->start();
        }
    );
    QTimer::singleShot(mParent->ourSettingsPtr->overlayDuration,this,  // Must be >= 2* duration above
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
void TipOverlay::paintEvent(QPaintEvent *)  // This is here so we can use styles
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
