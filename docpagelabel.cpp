// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include "docpagelabel.h"
#include "mainwindow.h"
#include <QObject>
#include <QImage>
#include <QThread>
#include <QPainter>
#include <QBitmap>
#include <QColor>

#include <cassert>

#include "piconstants.h"
#include "oursettings.h"

// Note terminology:
//   The transition is the page contents appearing somewhere
//   The highlight is the, occasional, surrounding boarder to call one's attention to the page change
// Transitions will influence the type of highlight shown but they are done in different QLabels overlaid

docPageLabel::docPageLabel(QWidget *parent,MainWindow* mp) : QLabel(parent)
{
    // Widget to display one page where/how we want it

    qDebug() << " in constructor";

    ourParent = parent;
    mParent = mp;

    pageHighlightHeight=mParent->ourSettingsPtr->getSetting("pageHighlightHeight").toInt();
    pageHighlightDelay=mParent->ourSettingsPtr->getSetting("pageHighlightDelay").toInt();
    pageTurnDelay=mParent->ourSettingsPtr->getSetting("pageTurnDelay").toInt();

    ourOverlay.setParent(this);
    ourOverlay.hide();
    ourHighlightOverlay.setParent(this);
    ourHighlightOverlay.hide();
    ourHighlightOverlay.setAttribute(Qt::WA_TranslucentBackground);
    our2ndHighlightOverlay.setParent(this);
    our2ndHighlightOverlay.hide();
    our2ndHighlightOverlay.setAttribute(Qt::WA_TranslucentBackground);
    ourOverlayTimer.setInterval(pageTurnDelay);
    ourOverlayTimer.setSingleShot(true);
    connect(&ourOverlayTimer, &QTimer::timeout,
        [=]()
        {
            qDebug() << "Hiding overlay";
            ourOverlay.hide();
        }
     );
    ourHighlightShowTimer.setSingleShot(true);
    connect(&ourHighlightShowTimer, &QTimer::timeout,
        [=]()
        {
            qDebug() << "Showing highlight";
            ourHighlightOverlay.show();
        }
     );
    ourHighlightHideTimer.setSingleShot(true);
    connect(&ourHighlightHideTimer, &QTimer::timeout,
        [=]()
        {
            qDebug() << "Hiding highlight";
            ourHighlightOverlay.hide();
        }
     );
    our2ndHighlightShowTimer.setSingleShot(true);
    connect(&our2ndHighlightShowTimer, &QTimer::timeout,
        [=]()
        {
            qDebug() << "Showing 2nd highlight";
            our2ndHighlightOverlay.show();
        }
     );
    our2ndHighlightHideTimer.setSingleShot(true);
    connect(&our2ndHighlightHideTimer, &QTimer::timeout,
        [=]()
        {
            qDebug() << "Hiding 2nd highlight";
            our2ndHighlightOverlay.hide();
        }
     );
    newImageIsBlank = false;
    oldImageIsBlank = false;
}

docPageLabel::~docPageLabel()
{
   qDebug() << "in destructor";
   // clean up????
}

void docPageLabel::placeImage(docPageLabel::docTransition thisTransition, QString color)
{
    QImage* blankImage = new QImage(this->width(), this->height(), QImage::Format_RGB32);
    blankImage->fill(color);
    qDebug() << "Switching placement request for blank image as it is outside of page range";
    newImageIsBlank = true;  // This will remember that the image was blank (too hard to check the pixmap itself
    placeImage(thisTransition, blankImage, color);
}

void docPageLabel::placeImage(docPageLabel::docTransition thisTransition, QImage* newImageBuffer, QString color)
{
    // This handles (some) transitions by putting an overlay with the OLD image
    // on top, where it can be left for a period of time, but the NEW is the
    // permanent, underlying image.  If there is no transition then no overlay is created.
    //
    // Possible transitions:
    //
    //   noTransition       Immediately display new (but there is a border transition briefly)
    //   halfPage           The BOTTOM half of the old page remains shown for transition time
    //                      Note this means the overlay is a half screen down
    //   fullPage           Page will remain unchanged for transition time, then brief highlight
    //   fullPageNow        Page is displayed immediately, with brief highlight

    assert(newImageBuffer);
    qDebug() << "Entered with transition type = " << thisTransition
             << ", and image of size " << newImageBuffer->width() << "x" << newImageBuffer->height()
             << ", container is " << this->width() << "x" << this->height();

    // Just kill any timers and hide any overlays we have now as we will make new
    HideAnyInProgressTransitions();

    // We form an image here of what we need to display with suitable background and centered, the image in pageImages
    // does not have these borders.  The new image is formed first so we can copy it into the overlay if needed
    QImage newImage(this->width(), this->height(),QImage::Format_ARGB32_Premultiplied);
    newImage.fill(QColor(color));   // We have to paint this not transparent since pages are different sizes
    // We should only be scaling down never up(pdfDocument takes care of that),
    // We do need need to recalculate scale with possibility we are scalling down
    float scale = std::min((float)(this->width())  / (float)(newImageBuffer->width()),
                           (float)(this->height()) / (float)(newImageBuffer->height()));

    // In the following if we are forcing the scale (in the render thread) to an integer, we may see
    // a need to scale up, so force it to one if we did that; otherwise it indicates a bug if
    // we see a need to scale up.  Not scaling up (when it thinks we need to) causes the image to be
    // a pixel or two too small, but isn't very noticable due to the border colors.

#ifdef MUSICALPI_FORCE_SCALE_TO_INTEGER
    scale = std::min((float)1.0,scale);  // Never scale up
#else
    assert(scale<=1.0);
#endif

    qDebug() << "Scale of drawImage for new image (should be <= 1, ideally == 1 in play mode for quality)  = " << scale;
    int newW = newImageBuffer->width() * scale;
    int newH = newImageBuffer->height() * scale;
    int newX = (this->width() - newW)/2;
    int newY = (this->height() - newH)/2;
    QPainter pNew(&newImage);
    pNew.setRenderHint(QPainter::SmoothPixmapTransform);
    pNew.drawImage( QRectF (newX, newY, newW, newH), *newImageBuffer);  // This implicitly draws the whole from image, scaling if needed

    // While we have all the new image info, go ahead and build the highlight overlay(s)
    // according to the transition type.  This is just the highlight overlay, not the
    // page transition overlay which comes next.
    if(!newImageIsBlank || thisTransition == noTransition)  // if the new page is blank there is no highlight regardless, also if no transition
    {
        QImage frameImage(QImage(this->width(), this->height(), QImage::Format_ARGB32));
        frameImage.fill(Qt::transparent);
        QPainter hp(&frameImage);
        hp.setBrush(QBrush(Qt::green,Qt::Dense4Pattern));
        hp.setPen(Qt::NoPen);
        if(thisTransition == fullPageNow || thisTransition == fullPage)
        {
            hp.drawRect(newX,newY,newW,pageHighlightHeight);
            hp.drawRect(newX,newY,pageHighlightHeight,newH);
            hp.drawRect(newX,newY + newH-pageHighlightHeight,newW,pageHighlightHeight);
            hp.drawRect(newX + newW-pageHighlightHeight,newY,pageHighlightHeight,newH);
            if(thisTransition == fullPageNow)
            {
                ourHighlightShowTimer.setInterval(1);
                ourHighlightHideTimer.setInterval(1 + pageHighlightDelay);
            }
            else // fullPage
            {
                ourHighlightShowTimer.setInterval(pageTurnDelay);
                ourHighlightHideTimer.setInterval(pageTurnDelay + pageHighlightDelay);
            }
            ourHighlightOverlay.setGeometry(0,0,this->geometry().width(),this->geometry().height());   // lay over ourself
            ourHighlightOverlay.setPixmap(QPixmap::fromImage(frameImage));
            ourHighlightShowTimer.start();
            ourHighlightHideTimer.start();
        }
        else if (thisTransition == halfPage)
        {
            // First draw top half which appears immediately
            hp.drawRect(newX,newY,newW,pageHighlightHeight);   // top left across
            hp.drawRect(newX,newY,pageHighlightHeight,newH/2); // top left down
            hp.drawRect(newX,newY + newH/2 -pageHighlightHeight,newW,pageHighlightHeight);  // bottom left across
            hp.drawRect(newX + newW - pageHighlightHeight,newY,pageHighlightHeight,newH/2);  // top right down
            ourHighlightShowTimer.setInterval(1);
            ourHighlightHideTimer.setInterval(1 + pageHighlightDelay);
            ourHighlightOverlay.setGeometry(0,0,this->geometry().width(),this->geometry().height());   // lay over ourself
            ourHighlightOverlay.setPixmap(QPixmap::fromImage(frameImage));

            // now bottom half which appears later
            QImage frame2ndImage(QImage(this->width(), this->height(), QImage::Format_ARGB32));
            frame2ndImage.fill(Qt::transparent);
            QPainter hp2(&frame2ndImage);
            hp2.setBrush(QBrush(Qt::green,Qt::Dense4Pattern));
            hp2.setPen(Qt::NoPen);
            hp2.drawRect(newX,newY + newH/2,newW,pageHighlightHeight);
            hp2.drawRect(newX,newY + newH/2,pageHighlightHeight,newH/2);
            hp2.drawRect(newX,newY + newH-pageHighlightHeight,newW,pageHighlightHeight);
            hp2.drawRect(newX + newW-pageHighlightHeight,newY + newH/2,pageHighlightHeight,newH/2);
            our2ndHighlightShowTimer.setInterval(pageTurnDelay);
            our2ndHighlightHideTimer.setInterval(pageTurnDelay + pageHighlightDelay);
            our2ndHighlightOverlay.setGeometry(0,0,this->geometry().width(),this->geometry().height());   // lay over ourself
            our2ndHighlightOverlay.setPixmap(QPixmap::fromImage(frame2ndImage));

            ourHighlightShowTimer.start();
            ourHighlightHideTimer.start();
            our2ndHighlightShowTimer.start();
            our2ndHighlightHideTimer.start();
        }
    }

    // This is the page image transition overlay, if needed.
    // This is the OLD image, the new image is always placed inside "this", so to show it
    // we just remove the ourOverlay at the right time.

    if((thisTransition != noTransition && thisTransition != fullPageNow) && this->pixmap() && !oldImageIsBlank) // this latter is still the old image; if none no transition regardless
    {
        ourOverlay.setGeometry(0,0,this->geometry().width(),this->geometry().height());  // Position directly over
        QPixmap tmpPixmap = this->pixmap()->copy();  // grab the old image -- the overlay is what is OLD not new, new will be underneath
        if(thisTransition == halfPage)  // we have to replace the top half
        {
            // For overlay just divide it in half do not rescale, and paint new over old
            QPainter pOld(&tmpPixmap);
            QRectF sameFromTo(0,0,tmpPixmap.width(), tmpPixmap.height()/2);
            pOld.drawImage(sameFromTo,newImage,sameFromTo);
        }
        //else thisTransition == fullPage (but not fullPageNow)
        ourOverlay.setPixmap(tmpPixmap);
        ourOverlay.show();
        qDebug() << "Starting timer to hide overlay";
        ourOverlayTimer.start();
    }
    this->setPixmap(QPixmap::fromImage(newImage));

    // This awkward technique records in this instance whether the image now displayed is blank
    oldImageIsBlank = newImageIsBlank;  // remember what we just loaded
    newImageIsBlank = false;  // reseet for next call, since in this variant we can't tell directly
}
void docPageLabel::HideAnyInProgressTransitions()
{
    // Used to interrupt transitions if a sudden change occurs (e.g. a subsequent page turn before this finished)
    ourOverlay.hide();
    ourHighlightOverlay.hide();
    our2ndHighlightOverlay.hide();
    ourOverlayTimer.stop();
    ourHighlightShowTimer.stop();
    ourHighlightHideTimer.stop();
    our2ndHighlightShowTimer.stop();
    our2ndHighlightHideTimer.stop();
}
