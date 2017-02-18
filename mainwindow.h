#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QLayout>
#include <QPushButton>
#include <QMouseEvent>
#include <QColor>

#include "tipoverlay.h"
#include "docpagelabel.h"
#include "pdfdocument.h"
#include "musiclibrary.h"
#include "aboutwidget.h"
#include "settingswidget.h"
#include "midiplayerV2.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();
    // Dynamic layouts and labels for visible pages, defined at max, but vary
    int pagesNowAcross;
    int pagesNowDown;

private:
    PDFDocument* PDF;
    enum runningModes {libraryMode, playMode, aboutMode, settingsMode};
    runningModes nowMode;  // Note playMode is either playing=true or playing=false as submode
    bool playing;          // Submode for playMode, indicates if we are reviewing or playing
    int leftmostPage;      // Page number (reference 1) of leftmost page

    // Window contents persistent widgets
    QWidget* outerLayoutWidget;  // outer widget set as central widget in window to hold layout
      QVBoxLayout* outerLayout;
        QWidget* menuLayoutWidget;   // Used if any menu at all present
          QHBoxLayout* menuLayout;
            QWidget* mainMenuLayoutWidget;   // Used for menu options that are always present (if any menu present)
              QHBoxLayout* mainMenuLayout;
                QPushButton* libraryButton;
                QPushButton* settingsButton;
                QPushButton* aboutButton;
                QPushButton* exitButton;
            QWidget* playerMenuLayoutWidget;  // Used for manu options shown during play-review mode
              QHBoxLayout* playerMenuLayout;
                QPushButton* playButton;
                QPushButton* firstButton;
                QPushButton* nextButton;
                QPushButton* prevButton;
                QPushButton* lastButton;
                QPushButton* oneUpButton;
                QPushButton* twoUpButton;
                QPushButton* fourByTwoButton;
                QPushButton* playMidiButton;
        QWidget* generalLayoutWidget;   // This is the main body under the menu and holds everything except play mode items
          QHBoxLayout* generalLayout;
            QLabel* logoLabel;
            musicLibrary* libraryTable;
            aboutWidget* aboutLabel;
            settingsWidget* settingsLabel;
       docPageLabel* visiblePages[MUSICALPI_MAXROWS * MUSICALPI_MAXCOLUMNS];  // used for play mode panes; layout is ignored for these and explicitly positioned
       TipOverlay* overlay;                                                   // fits (briefly) over the whole window when play-playing mode starts.

    midiPlayerV2* mp; // This widget is a free floating window

    int loadPagePendingNumber[MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS]; // 0 indicates none pending
    docPageLabel::docTransition loadPagePendingTransition[MUSICALPI_MAXCOLUMNS * MUSICALPI_MAXROWS];

    void setLibraryMode();
    void setPlayMode(bool playing, int pagesToShowAcross, int pagesToShowDown);
    void navigateTo(int leftPage);
    void setAboutMode();
    void setSettingsMode();
    void setupCoreWidgets();
    void HideEverything();
    void playingNextPage();
    void playingPrevPage();
    void deletePDF();
    void checkQueueVsCache();
    void mouseReleaseEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void sizeLogo();
    void keyPressEvent(QKeyEvent* e);
    void doMidiPlayer();
    void closeMidiPlayer();

private slots:
    void startPlayMode(QString,QString);

};

#endif // MAINWINDOW_H
