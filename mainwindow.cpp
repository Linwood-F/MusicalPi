// Copyright 2018 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QColor>
#include <QApplication>
#include <QPushButton>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <cmath>

#include "mainwindow.h"
#include "tipoverlay.h"
#include "pdfdocument.h"
#include "musiclibrary.h"
#include "aboutwidget.h"
#include "settingswidget.h"
#include "midiplayerV2.h"
#include "oursettings.h"
#include "playlists.h"


MainWindow::MainWindow() : QMainWindow()
{
    qDebug() << "MainWindow::MainWindow() in constructor";
    setWindowTitle(tr("MusicalPi"));
    ourSettingsPtr = new ourSettings(this);  // Get all our defaults
    PDF = NULL;
    mp = NULL;
    pl = NULL;
    pagesNowDown = 0;
    pagesNowAcross = 0;
    overlay = NULL;
    for(int i=0; i<MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS; i++) loadPagePendingNumber[i]=0; // flag as nothing yet to load

    // Because this is a QMainWindow we have to put the layout inside a
    // widget which is the central widget, the rest works as normal in the layout.

    setupCoreWidgets(); // These are widgets that are always there and change visibility
    setLibraryMode();

    showFullScreen(); // Since this fires resize it needs to be after things are initialized
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow in destructor";
    deletePDF();
    DELETE_LOG(libraryTable);
}

void MainWindow::setupCoreWidgets()
{
    // These are the widgets that persist, may become hidden but are not destroyed
    // Refer to indented declaration in header showing nesting

    qDebug()<< "Starting widget setup";
    outerLayoutWidget = new QWidget();
    outerLayoutWidget->setObjectName("outerLayoutWidget");
    outerLayoutWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    outerLayout = new QVBoxLayout(outerLayoutWidget);
    this->setCentralWidget(outerLayoutWidget);
    outerLayout->setAlignment(Qt::AlignTop);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(0,0,0,0);

    menuLayoutWidget = new QWidget();
    menuLayoutWidget->setObjectName("menuLayoutWidget");
    menuLayout = new QHBoxLayout(menuLayoutWidget);
    outerLayout->addWidget(menuLayoutWidget);
    menuLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    menuLayout->setSpacing(0);
    menuLayout->setContentsMargins(0,0,0,0);
    menuLayoutWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    mainMenuLayoutWidget = new QWidget();
    mainMenuLayoutWidget->setObjectName("mainMenuLayoutWidget");
    mainMenuLayout = new QHBoxLayout(mainMenuLayoutWidget);
    menuLayout->addWidget(mainMenuLayoutWidget);
    mainMenuLayout->setSpacing(10);
    mainMenuLayout->setAlignment(Qt::AlignLeft);
    mainMenuLayout->setContentsMargins(5,5,5,5);

    playerMenuLayoutWidget = new QWidget();
    playerMenuLayout = new QHBoxLayout(playerMenuLayoutWidget);
    menuLayout->addWidget(playerMenuLayoutWidget);
    playerMenuLayout->setSpacing(10);
    playerMenuLayout->setAlignment(Qt::AlignLeft);
    playerMenuLayout->setContentsMargins(5,5,5,5);

    libraryButton  = new QPushButton("Library");
    mainMenuLayout->addWidget(libraryButton);
    connect(libraryButton,&QPushButton::clicked, this, &MainWindow::setLibraryMode);

    settingsButton = new QPushButton("Settings");
    mainMenuLayout->addWidget(settingsButton);
    connect(settingsButton,&QPushButton::clicked, this, &MainWindow::setSettingsMode);

    aboutButton    = new QPushButton("About");
    mainMenuLayout->addWidget(aboutButton);
    connect(aboutButton,&QPushButton::clicked, this, &MainWindow::setAboutMode);

    exitButton     = new QPushButton("Exit");
    mainMenuLayout->addWidget(exitButton);
    connect(exitButton,&QPushButton::clicked, this,QApplication::quit);

    QString gapStyle = "margin-left: 30px;"; // Some extra spacing between groups

    playButton      = new QPushButton("Play");
    playerMenuLayout->addWidget(playButton);
    playButton->setStyleSheet(gapStyle);
    connect(playButton,&QPushButton::clicked, this, [this] {this->setPlayMode(true,pagesNowAcross,pagesNowDown);});

    firstButton     = new QPushButton("First");
    playerMenuLayout->addWidget(firstButton);
    firstButton->setStyleSheet(gapStyle);
    connect(firstButton,&QPushButton::clicked, [this]{navigateTo(1);});

    prevButton      = new QPushButton("Previous");
    playerMenuLayout->addWidget(prevButton);
    connect(prevButton,&QPushButton::clicked, [this]{int nextPage = leftmostPage - (pagesNowAcross * pagesNowDown); navigateTo(nextPage);});

    nextButton      = new QPushButton("Next");
    playerMenuLayout->addWidget(nextButton);
    connect(nextButton,&QPushButton::clicked, [this]{int nextPage = leftmostPage + (pagesNowAcross * pagesNowDown); navigateTo(nextPage);});

    lastButton      = new QPushButton("Last");
    playerMenuLayout->addWidget(lastButton);
    connect(lastButton,&QPushButton::clicked, [this]{navigateTo(MUSICALPI_MAXPAGES);});

    oneUpButton     = new QPushButton("One Up");
    playerMenuLayout->addWidget(oneUpButton);
    oneUpButton->setStyleSheet(gapStyle);
    connect(oneUpButton,&QPushButton::clicked, this, [this](){this->setPlayMode(false,1,1);});

    twoUpButton     = new QPushButton("Two Up");
    playerMenuLayout->addWidget(twoUpButton);
    connect(twoUpButton,&QPushButton::clicked, this, [this](){this->setPlayMode(false,2,1);});

    fourByTwoButton = new QPushButton("4 x 2");
    playerMenuLayout->addWidget(fourByTwoButton);
    connect(fourByTwoButton,&QPushButton::clicked, this, [this](){this->setPlayMode(false,4,2);});

    playMidiButton = new QPushButton("Play Midi");
    playerMenuLayout->addWidget(playMidiButton);
    connect(playMidiButton,&QPushButton::clicked, this, &MainWindow::doMidiPlayer);

    playListButton = new QPushButton("PlayLists");
    playerMenuLayout->addWidget(playListButton);
    connect(playListButton,&QPushButton::clicked, this, &MainWindow::doPlayLists);

    // ALl details of this get filled in during use in play section; not all may be use, this is max.
    for (int i=0; i < (MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS); i++)
    {
        visiblePages[i] = new docPageLabel(outerLayoutWidget, this);
    }

    generalLayoutWidget = new QWidget(outerLayoutWidget);
    generalLayoutWidget->setAccessibleDescription("generalLayoutWidget");
    generalLayoutWidget->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    outerLayout->addWidget(generalLayoutWidget);

    generalLayout = new QHBoxLayout(generalLayoutWidget);

    logoLabel = new QLabel("logo goes here");
    logoLabel->setObjectName("logoLabel");
    generalLayout->addWidget(logoLabel);
    sizeLogo();

    libraryTable = new musicLibrary(generalLayoutWidget,this);
    libraryTable->setObjectName("libraryTable");
    generalLayout->addWidget(libraryTable);
    generalLayout->setAlignment(generalLayout,Qt::AlignTop | Qt::AlignLeft);

    aboutLabel = new aboutWidget(generalLayoutWidget);
    aboutLabel->setObjectName("aboutLabel");
    generalLayout->addWidget(aboutLabel);
    generalLayout->setAlignment(aboutLabel,Qt::AlignTop | Qt::AlignLeft);

    settingsUI = new settingsWidget(generalLayoutWidget, this);
    settingsUI->setObjectName("settingsUI");
    generalLayout->addWidget(settingsUI);
    generalLayout->setAlignment(settingsUI,Qt::AlignTop | Qt::AlignLeft);

    overlay = new TipOverlay(outerLayoutWidget, this);
    connect(libraryTable, SIGNAL(songSelected(QString,QString)), this, SLOT(startPlayMode(QString,QString)));
}

void MainWindow::setLibraryMode()
{
    qDebug() << "called";
    nowMode = libraryMode;
    HideEverything();
    deletePDF(); // we'll create a new one if needed
    menuLayoutWidget->show();
    mainMenuLayoutWidget->show();
    generalLayoutWidget->show();
    libraryTable->show();  // Implicitly loads data
}

void MainWindow::setAboutMode()
{
    qDebug() << "Entered";
    nowMode = aboutMode;
    HideEverything();
    deletePDF(); // we'll create a new one if needed
    menuLayoutWidget->show();
    mainMenuLayoutWidget->show();
    generalLayoutWidget->show();
    aboutLabel->show();
}

void MainWindow::setSettingsMode()
{
    qDebug() << "Entered";
    nowMode = settingsMode;
    HideEverything();
    deletePDF(); // we'll create a new one if needed
    menuLayoutWidget->show();
    mainMenuLayoutWidget->show();
    generalLayoutWidget->show();
    settingsUI->loadData();
    settingsUI->show();
}

void MainWindow::startPlayMode(QString path, QString _titlePlaying)
{
    qDebug() << "Calling with " << path << ", " << _titlePlaying;
    if(PDF==NULL || PDF->filepath != path)
    {
        deletePDF();
        PDF = new PDFDocument(this, path, _titlePlaying);
        leftmostPage = 1;   // Always start new document from zero
        connect(PDF,&PDFDocument::newImageReady,this, [this]{this->checkQueueVsCache();});
        //Hide or show button if midi there
        playMidiButton->setVisible(PDF->midiFilePath != "");
    }
    setPlayMode(false,2,1);
}

void MainWindow::setPlayMode(bool _playing, int pagesToShowAcross, int pagesToShowDown)
{
    qDebug() << "setPlayMode(playing=" << _playing << "," << pagesToShowAcross << "," << pagesToShowDown << ") called";
    nowMode=playMode;
    HideEverything();  // This gets called with play already there when changing modes
    playing = _playing;
    pageBorderWidth = ourSettingsPtr->getSetting("pageBorderWidth").toInt();
    if(!playing) // if we are in player review, not playing mode
    {
        menuLayoutWidget->show();
        mainMenuLayoutWidget->show();
        playerMenuLayoutWidget->show();
    }
    pagesNowAcross = pagesToShowAcross;
    pagesNowDown = pagesToShowDown;
    // Change all widget backgrounds to the playing color, or normal color, as appropriate
#ifndef MUSICALPI_DEBUG_WIDGET_BORDERS
    QString playBackground = "background-color: " + QString((playing ?  MUSICALPI_BACKGROUND_COLOR_PLAYING : MUSICALPI_BACKGROUND_COLOR_NORMAL));
    outerLayoutWidget->setStyleSheet("docPageLabel {" + playBackground + " } "
                                     "#outerLayoutWidget {" + playBackground + "}" );
#endif
    // These are placed in outerLayoutWidget reduced in height but the size of menuLayoutWidget if not playing
    QSize outerLayoutWidgetSize = outerLayoutWidget->size();
    QSize menuLayoutWidgetSize = menuLayoutWidget->size();
    // These are packed as tight as they can go, except between rows and between columns put in a small border
    int roomForMenu = playing ? 0 : menuLayoutWidgetSize.height();
    int maxPageWidth = std::floor((outerLayoutWidgetSize.width() - pageBorderWidth * (pagesToShowAcross - 1)) / pagesToShowAcross);
    int maxPageHeight = std::floor((outerLayoutWidgetSize.height() - roomForMenu - pageBorderWidth * (pagesToShowDown - 1)) / pagesToShowDown);
    PDF->checkResetImageSize(maxPageWidth, maxPageHeight + roomForMenu);  // add menu back in so we get larger image in cache so we don't re-cache for playing mode
    for (int r=0; r<pagesToShowDown; r++)
    {
        for (int c=0; c<pagesToShowAcross; c++)
        {
            int indx = r * pagesToShowAcross + c;
            visiblePages[indx]->setGeometry(c * (pageBorderWidth + maxPageWidth), roomForMenu + r * (pageBorderWidth + maxPageHeight), maxPageWidth, maxPageHeight );
            loadPagePendingNumber[indx] = leftmostPage + indx; // This is a request to display
            loadPagePendingTransition[indx] = docPageLabel::noTransition;
            visiblePages[indx]->setAttribute(Qt::WA_TransparentForMouseEvents, playing);  // if we are playing, pass mouse events through to main window
            visiblePages[indx]->show();
            qDebug() << "visiblePages[" << indx << "] sized and shown";
        }
    }
    checkQueueVsCache();
    PDF->checkCaching();
    if(playing && ourSettingsPtr->getSetting("pageTurnTipOverlay").toBool()) overlay->show();
}

void MainWindow::checkQueueVsCache()
{
    PDF->adjustCache(leftmostPage);
    PDF->lockOrUnlockMutex(true); // Shouldn't need this but just make sure the ones we find are fully formed - this may be too broad, and we might want to put this just around the inner loop so it releases each time
    int skipped = 0;
    for(int i = 0; i < MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS ; i++)
    {
        if(loadPagePendingNumber[i] > PDF->numPages)
        {
            // We are out of the range of the book and need to clear the widget
            visiblePages[i]->placeImage(loadPagePendingTransition[i], playing ? MUSICALPI_BACKGROUND_COLOR_PLAYING : MUSICALPI_BACKGROUND_COLOR_NORMAL );  // special call which asks for empty page (but allows transitions)
        }
        else if(loadPagePendingNumber[i] && PDF->pageImagesAvailable[loadPagePendingNumber[i]-1])  // If it's needed and present
        {
            // qDebug() << "Found we should display page " << loadPagePendingNumber[i] << " for position " << i;
            visiblePages[i]->placeImage(loadPagePendingTransition[i], PDF->pageImages[loadPagePendingNumber[i]-1], playing ? MUSICALPI_BACKGROUND_COLOR_PLAYING : MUSICALPI_BACKGROUND_COLOR_NORMAL);
            loadPagePendingNumber[i]=0;
        }
        else if(loadPagePendingNumber[i])
        {
            // qDebug() << "Found we need a page but it isn't available, skipped for now page " << loadPagePendingNumber[i] << " for position " << i;
            skipped++;
        }
        // else we just don't need it (yet)
    }
    PDF->lockOrUnlockMutex(false);
}

void MainWindow::playingNextPage()
{
    // Principle: Going foward, we assume the eyes are on the last page and so we do any prior pages
    //            immediately (if any), and delay the last page.  On the last page if it is the only
    //            page we do it half at a time.

    qDebug()<< "started";
    if(nowMode!=playMode || !playing) return;
    int nextPage = leftmostPage + pagesNowAcross * pagesNowDown;
    if(nextPage> PDF->numPages)
    {
        qDebug() << "play forward no action - exiting as already at end";
        return;
    }
    leftmostPage = nextPage; // new leftmost
    for(int i=0; i<pagesNowDown * pagesNowAcross; i++)
    {
        loadPagePendingNumber[i] = leftmostPage + i;
        if(i == pagesNowDown * pagesNowAcross - 1 && pagesNowDown == 1 && pagesNowAcross == 1)  // only screen
            loadPagePendingTransition[i] = docPageLabel::halfPage;
        else if (i == pagesNowDown * pagesNowAcross - 1) // last screen but not only
            loadPagePendingTransition[i] = docPageLabel::fullPage;
        else if (i==0)    // if leftmost page flash it
            loadPagePendingTransition[i] = docPageLabel::fullPageNow;
        else
            loadPagePendingTransition[i] = docPageLabel::noTransition;
    }
    checkQueueVsCache();
}

void MainWindow::playingPrevPage()
{
    // Principle: Going backwards, we only want to go back a single page regardless of "up" since the human would not
    //            have hit "back" for any displayed page, but going back more than 1 page (when we mostly have 2
    //            pages displayed) will mean transitioning the last page (which they are reading) to get back.
    //            this would be fine if they are at the very end of the page, but "back" could be anywhere on the page.
    //            Going back on a single page display just stinks, as there's no real choice.
    qDebug() << "started";
    if(nowMode!=playMode || !playing) return;
    if(leftmostPage == 1)
    {
        qDebug() << "exiting as already at start";
        return;
    }
    leftmostPage--;
    for(int i=0; i<pagesNowDown * pagesNowAcross; i++)
    {
        loadPagePendingNumber[i] = leftmostPage + i;
        if(i == pagesNowDown * pagesNowAcross - 1 && pagesNowDown == 1 && pagesNowAcross == 1)  // only screen
            loadPagePendingTransition[i] = docPageLabel::halfPage;
        else if (i == pagesNowDown * pagesNowAcross - 1) // last screen but not only
            loadPagePendingTransition[i] = docPageLabel::fullPage;
        else if(i==0)  // we want a highlight
            loadPagePendingTransition[i] = docPageLabel::fullPageNow;
        else
            loadPagePendingTransition[i] = docPageLabel::noTransition;
    }
    checkQueueVsCache();
}

void MainWindow::navigateTo(int nextPage)
{
    // Request, in playMode but not while playing=true (so no transitions)
    qDebug() << "Nevigate to " << nextPage;
    assert(!playing);   // We shouldn't get here while actually playing, that's different
    // This program has the responsibility to make the parameter sane
    // Start with forcing the next page to be within the document
    nextPage = std::min(PDF->numPages,std::max((int)1,nextPage));
    if(nextPage == leftmostPage)
    {
        qDebug() << "corrected request=" << nextPage << " and exiting as we are already there.";
        return;
    }
    if((nextPage > leftmostPage) && (nextPage <= leftmostPage + pagesNowDown * pagesNowAcross - 1))
    {
        qDebug() << "determined corrected nextPage (" << nextPage << ") is already shown relative to leftmostPage = " << leftmostPage;
        return;
    }
    // It appears we need to move, the getOnePage will be responsible if we are beyond the last page
    // But if we are jumping beyond "next" we want the last to be on the last window (but not when moving a window's worth)
    if(nextPage > leftmostPage + pagesNowDown * pagesNowAcross)
        leftmostPage = PDF->numPages - pagesNowDown * pagesNowAcross + 1;
    else
        leftmostPage = nextPage;
    for(int i=0; i< pagesNowDown * pagesNowAcross; i++)
    {
        loadPagePendingNumber[i] = leftmostPage + i;
        loadPagePendingTransition[i] = docPageLabel::noTransition;
    }
    checkQueueVsCache();
}

void MainWindow::HideEverything()
{
    qDebug() << "Hiding most widgets so we can display what we specifically need";
    // Hide every outer widget (except outerLayoutWidget) and display panes and library/about/etc panes
    menuLayoutWidget->hide();
    mainMenuLayoutWidget->hide();
    playerMenuLayoutWidget->hide();
    generalLayoutWidget->hide();
    libraryTable->hide();
    aboutLabel->hide();
    settingsUI->hide();
    DELETE_LOG(pl);  // if Playlists is there just get rid of it.
    DELETE_LOG(mp);  // same with midi player
    for(int row=0; row < MUSICALPI_MAXROWS; row++)
        for(int column=0; column < MUSICALPI_MAXCOLUMNS; column++)
        {
            visiblePages[row * MUSICALPI_MAXCOLUMNS + column]->hide();
            visiblePages[row * MUSICALPI_MAXCOLUMNS + column]->clear();  // we need to re-do these
        }
    overlay->hide();
    libraryTable->hideKeyboard();
}

void MainWindow::deletePDF()
{
    // THis is to keep the pointer NULL if not valid so we can reliably clean up
    if(PDF) delete PDF;
    PDF=NULL;
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // This is only used at present if we are in play mode, and actually playing
    qDebug() << "event " << ((event->button() == Qt::LeftButton) ? "left" : "other")
             << ", location=(" << event->x() << "," << event->y() << ")";
    if(nowMode == playMode && playing)
    {
        if(overlay)
        {
            qDebug() << "We still have an overlay - hide it";
            overlay->hide();
        }
        if(event->y()< ourSettingsPtr->getSetting("overlayTopPortion").toInt() * this->height() / 100)  // How will I know where with it a class ???
        {
            qDebug() << "MainWindow::mouseReleaseEvent ending play mode";
            setPlayMode(false,pagesNowAcross,pagesNowDown);
        }
        else if (event->x() < ourSettingsPtr->getSetting("overlaySidePortion").toInt() * this->width() / 100)
        {
            qDebug() << "MainWindow::mouseReleaseEvent doing previous page";
            playingPrevPage();
        }
        else if (event->x() > this->width() - ourSettingsPtr->getSetting("overlaySidePortion").toInt() * this->width() / 100)
        {
            qDebug() << "MainWindow::mouseReleaseEvent doing next page";
            playingNextPage();
        }
        else
        {
            qDebug() << "MainWindow::mouseReleaseEvent touch in not-effective space";
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent*e)
{
    // Note that this will RE-CALL play mode if it is already running but not others

    // Some brief experimentation showed that this isn't firing until the move is
    // reasonably done, so there's no delay in the calculations.

    // General resizing stuff goes here.  Regular widget resizing will handle itself.

    qDebug() << "Resize event, new size = " << e->size().width() << "x" << e->size().height();
    if(nowMode == playMode && pagesNowAcross && pagesNowDown ) setPlayMode(playing, pagesNowAcross, pagesNowDown);
    else sizeLogo();
}

void MainWindow::sizeLogo()
{
    qDebug() << "Sizing logo";
    QPixmap pm;
    pm.load("/home/ferguson/MusicalPi/MusicalPi.gif");
    logoLabel->setPixmap(pm.scaledToWidth(outerLayoutWidget->width() * ourSettingsPtr->getSetting("logoPct").toInt() / 100)); // ?? move to resize
    logoLabel->setScaledContents(false);
    logoLabel->setAlignment(Qt::AlignTop);
    logoLabel->setContentsMargins(20,20,20,20);
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
    // Respond to general keypress events (at this point just up/down in playmode-playing)

    if(nowMode!=playMode || !playing) return;  // Only pay attention in playmode-playing
    switch (e->key())
    {
        case Qt::Key_PageDown: qDebug() << "Received PageDown"; playingNextPage(); break;
        case Qt::Key_PageUp:   qDebug() << "Received PageUp";   playingPrevPage(); break;
    }
}

void MainWindow::doPlayLists()
{
    qDebug() << "Entered, bookID = " << libraryTable->bookIDSelected << ", title" << PDF->titleName << " Active list = " << libraryTable->ActiveList;
    DELETE_LOG(pl); // Don't use the same one if they ask for another, delete it and create a new one
    pl = new playLists(this, libraryTable,PDF->titleName, libraryTable->bookIDSelected);
    pl->move(QWidget::mapToGlobal(this->pos()));  // Put this somewhere interesting -- ??
}

void MainWindow::doMidiPlayer()
{
    DELETE_LOG(mp); // Don't run same one, create a new one if it exists
    mp = new midiPlayerV2(this,PDF->midiFilePath, PDF->titleName);
    mp->show();
    qDebug() << "Position of this = (" << this->x() << "," << this->y() << "), maptoglobal=(" << mapToGlobal(this->pos()).x() << "," << mapToGlobal(this->pos()).y() << ")";
    mp->move(QWidget::mapToGlobal(this->pos()));  // Put this somewhere interesting -- ??
}

