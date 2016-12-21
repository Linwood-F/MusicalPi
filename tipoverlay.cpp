#include "tipoverlay.h"

TipOverlay::TipOverlay(QWidget* parent) : QLabel(parent)
{
    qDebug() << "created";
    assert(parent);
    p = parent;
    overlayFade = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(overlayFade);
    overlayAnimation = new QPropertyAnimation(overlayFade,"opacity");
    overlayAnimation->setEasingCurve(QEasingCurve::Linear);
    overlayAnimation->setDuration(MUSICALPI_OVERLAY_DURATION / 3);  // Must be <= first timer below

}
void TipOverlay::showEvent(QShowEvent* event)
{
    int overlayFullWidth = p->size().width();
    int overlayTopHeight = MUSICALPI_OVERLAY_TOP_PORTION * p->size().height();
    int overlayPanelWidth = MUSICALPI_OVERLAY_SIDE_PORTION * overlayFullWidth;
    QRect endPlayRect(0,0,overlayFullWidth,overlayTopHeight);
    QRect backPlayRect(0,overlayTopHeight,overlayPanelWidth,p->size().height() - overlayTopHeight);
    QRect forwardPlayRect(overlayFullWidth - overlayPanelWidth,overlayTopHeight,overlayPanelWidth,p->size().height() - overlayTopHeight);
    this->setGeometry(QRect(0,0,p->geometry().width(),p->geometry().height()));
    this->setAttribute(Qt::WA_TranslucentBackground);
    QPixmap tmpPix = QPixmap(p->size());
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
    painter.setFont(QFont("Arial",36,1,false));
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
    QTimer::singleShot(MUSICALPI_OVERLAY_DURATION / 2,this,  // Must be >= duration above
       [=]
        {
            qDebug() << "TipOverlay:: in animation after first event";
            overlayAnimation->setEndValue(0.0);
            overlayAnimation->setStartValue(1.0);
            overlayAnimation->start();
        }
    );
    QTimer::singleShot(MUSICALPI_OVERLAY_DURATION,this,  // Must be >= 2* duration above
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
