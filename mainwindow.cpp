// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qDebug() << "MainWindow::MainWindow() in constructor";
    setWindowTitle(tr("MusicalPi"));
    PDF = NULL;
    mp = NULL;
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
    delete libraryTable;
}

void MainWindow::setupCoreWidgets()
{
    // These are the widgets that persist, may become hidden but are not destroyed
    // Refer to indented declaration in header showing nesting

    qDebug()<< "Starting widget setup";
    outerLayoutWidget = new QWidget();
    outerLayout = new QVBoxLayout(outerLayoutWidget);
    this->setCentralWidget(outerLayoutWidget);
    outerLayout->setAlignment(Qt::AlignTop);
    outerLayout->setSpacing(0);
    outerLayout->setContentsMargins(0,0,0,0);

    menuLayoutWidget = new QWidget();
    menuLayout = new QHBoxLayout(menuLayoutWidget);
    outerLayout->addWidget(menuLayoutWidget);
    menuLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    menuLayout->setSpacing(0);
    menuLayout->setContentsMargins(0,0,0,0);
    menuLayoutWidget->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);

    mainMenuLayoutWidget = new QWidget();
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

    // ALl details of this get filled in during use in play section; not all may be use, this is max.
    for (int i=0; i < (MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS); i++)
    {
        visiblePages[i] = new docPageLabel(outerLayoutWidget);
    }

    generalLayoutWidget = new QWidget(outerLayoutWidget);
    outerLayout->addWidget(generalLayoutWidget);

    generalLayout = new QHBoxLayout(generalLayoutWidget);
    logoLabel = new QLabel("logo goes here");
    generalLayout->addWidget(logoLabel);
    sizeLogo();

    libraryTable = new musicLibrary(generalLayoutWidget);
    generalLayout->addWidget(libraryTable);

    aboutLabel = new aboutWidget(generalLayoutWidget);
    generalLayout->addWidget(aboutLabel);

    settingsLabel = new settingsWidget(generalLayoutWidget);
    generalLayout->addWidget(settingsLabel);

    overlay = new TipOverlay(outerLayoutWidget);
    connect(libraryTable, SIGNAL(songSelected(QString,QString)), this, SLOT(startPlayMode(QString,QString)));
}

void MainWindow::setLibraryMode()
{
    qDebug() << "called";
    nowMode = libraryMode;
    HideEverything();
    deletePDF(); // we'll create a new one if needed
    libraryTable->loadData();
    menuLayoutWidget->show();
    mainMenuLayoutWidget->show();
    generalLayoutWidget->show();
    libraryTable->show();
}

void MainWindow::setAboutMode()
{
    qDebug() << "About mode -- needs stuff ????";
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
    qDebug() << "Settings mode -- need stuff ?????";
    nowMode = settingsMode;
    HideEverything();
    deletePDF(); // we'll create a new one if needed
    menuLayoutWidget->show();
    mainMenuLayoutWidget->show();
    generalLayoutWidget->show();
    settingsLabel->show();
}

void MainWindow::startPlayMode(QString path, QString _titlePlaying)
{
    if(PDF==NULL || PDF->filepath != path)
    {
        deletePDF();
        PDF = new PDFDocument(path, _titlePlaying);
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
    if(!playing) // if we are in player review, not playing mode
    {
        menuLayoutWidget->show();
        mainMenuLayoutWidget->show();
        playerMenuLayoutWidget->show();
    }
    pagesNowAcross = pagesToShowAcross;
    pagesNowDown = pagesToShowDown;
    // Change all widget backgrouns to the playing color, or normal color, as appropriate
    this->setStyleSheet(QString("QWidget { background-color: ") + (playing ?  QString(MUSICALPI_BACKGROUND_COLOR_PLAYING) : QString(MUSICALPI_BACKGROUND_COLOR_NORMAL)) + "}  QPushButton { background-color: grey }");
    // These are placed in outerLayoutWidget reduced in height but the size of menuLayoutWidget if not playing
    QSize outerLayoutWidgetSize = outerLayoutWidget->size();
    QSize menuLayoutWidgetSize = menuLayoutWidget->size();
    qDebug() << "outerLayoutWidget->size(" << outerLayoutWidgetSize.width() << "," << outerLayoutWidgetSize.height() << ")";
    qDebug() << "menuLayoutWidgetSize->size(" << menuLayoutWidgetSize.width() << "," << menuLayoutWidgetSize.height() << ")";

    // These are packed as tight as they can go, except between rows and between columns put in a small border
    int borderPixels = MUSICALPI_BORDERS;
    int roomForMenu = playing ? 0 : menuLayoutWidgetSize.height();
    int maxPageWidth = std::floor((outerLayoutWidgetSize.width() - borderPixels * (pagesToShowAcross - 1)) / pagesToShowAcross);
    int maxPageHeight = std::floor((outerLayoutWidgetSize.height() - roomForMenu - borderPixels * (pagesToShowDown - 1)) / pagesToShowDown);
    PDF->checkResetImageSize(maxPageWidth, maxPageHeight + roomForMenu);  // add menu back in so we get larger image in cache so we don't re-cache for playing mode
    for (int r=0; r<pagesToShowDown; r++)
    {
        for (int c=0; c<pagesToShowAcross; c++)
        {
            int indx = r * pagesToShowAcross + c;
            visiblePages[indx]->setGeometry(c * (borderPixels + maxPageWidth), roomForMenu + r * (borderPixels + maxPageHeight), maxPageWidth, maxPageHeight );
            loadPagePendingNumber[indx] = leftmostPage + indx; // This is a request to display
            loadPagePendingTransition[indx] = docPageLabel::noTransition;
            visiblePages[indx]->setAttribute(Qt::WA_TransparentForMouseEvents, playing);  // if we are playing, pass mouse events through to main window
            visiblePages[indx]->show();
            qDebug() << "visiblePages[" << indx << "] sized and shown";
        }
    }
    checkQueueVsCache();
    PDF->checkCaching();
    if(playing) overlay->show();
}

void MainWindow::checkQueueVsCache()
{
    PDF->adjustCache(leftmostPage);
    qDebug() << "Checking for anything to display";
    PDF->PDFMutex.lock(); // Shouldn't need this but just make sure the ones we find are fully formed - this may be too broad, and we might want to put this just around the inner loop so it releases each time
    int skipped = 0;
    for(int i = 0; i < MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS ; i++)
    {
        if(loadPagePendingNumber[i] > PDF->numPages)
        {
            // We are out of the range of the book and need to clear the widget
            visiblePages[i]->placeImage(loadPagePendingTransition[i], playing ? MUSICALPI_BACKGROUND_COLOR_PLAYING_QT : MUSICALPI_BACKGROUND_COLOR_NORMAL_QT );  // special call which asks for empty page (but allows transitions)
        }
        else if(loadPagePendingNumber[i] && PDF->pageImagesAvailable[loadPagePendingNumber[i]-1])  // If it's needed and present
        {
            qDebug() << "Found we should display page " << loadPagePendingNumber[i] << " for position " << i;
            visiblePages[i]->placeImage(loadPagePendingTransition[i], PDF->pageImages[loadPagePendingNumber[i]-1], playing ? MUSICALPI_BACKGROUND_COLOR_PLAYING_QT : MUSICALPI_BACKGROUND_COLOR_NORMAL_QT);
            loadPagePendingNumber[i]=0;
        }
        else if(loadPagePendingNumber[i]) skipped++;
        // else we just don't need it (yet)
    }
    if(skipped) qDebug() << "Leaving the check with "<< skipped << "pages not displayed because not (yet) available.";
    PDF->PDFMutex.unlock();
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
    nextPage = std::min(PDF->numPages,std::max(1,nextPage));
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
    settingsLabel->hide();
    if(mp != NULL) closeMidiPlayer();  // if it's open
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
        if(event->y()<MUSICALPI_OVERLAY_TOP_PORTION * this->height())  // How will I know where with it a class ???
        {
            qDebug() << "MainWindow::mouseReleaseEvent ending play mode";
            setPlayMode(false,pagesNowAcross,pagesNowDown);
        }
        else if (event->x() < MUSICALPI_OVERLAY_SIDE_PORTION * this->width())
        {
            qDebug() << "MainWindow::mouseReleaseEvent doing previous page";
            playingPrevPage();
        }
        else if (event->x() > this->width() - MUSICALPI_OVERLAY_SIDE_PORTION * this->width())
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
    logoLabel->setPixmap(pm.scaledToWidth(outerLayoutWidget->width()*MUSICALPI_LOGO_PCT)); // ?? move to resize
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

void MainWindow::doMidiPlayer()
{
    mp = new midiPlayerV2(this,PDF->midiFilePath, PDF->titleName);
    mp->show();
    qDebug() << "Position of this = (" << this->x() << "," << this->y() << "), maptoglobal=(" << mapToGlobal(this->pos()).x() << "," << mapToGlobal(this->pos()).y() << ")";
    mp->move(QWidget::mapToGlobal(this->pos()));  // Put this somewhere interesting -- ??
    connect(mp,&midiPlayerV2::requestToClose,this,[this]{this->closeMidiPlayer();});
}

void MainWindow::closeMidiPlayer()
{
    qDebug() << "Request to close midi";
    if(mp!=NULL) delete mp;
    mp = NULL;
}

