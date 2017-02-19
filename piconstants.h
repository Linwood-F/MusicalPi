#ifndef PICONSTANTS
#define PICONSTANTS

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#define MUSICALPI_MAXPAGES 1000   // Maximum pages in any PDF -- minimal impact to make bigger

// review/play screen maximum "up" sizes - moderate memory impact if larger (but indirect large impact on cache as below)

#define MUSICALPI_MAXROWS 2
#define MUSICALPI_MAXCOLUMNS 4

// A Good rule of thumb is cores - 1

#define MUSICALPI_THREADS 3

// Normal and playing styles ?? need more styling here I think ??
// These are not in settings file for need to coordinate the QT and English words.

#define MUSICALPI_BACKGROUND_COLOR_NORMAL "white"
#define MUSICALPI_BACKGROUND_COLOR_NORMAL_QT Qt::white
#define MUSICALPI_BACKGROUND_COLOR_PLAYING "black"
#define MUSICALPI_BACKGROUND_COLOR_PLAYING_QT Qt::black

// SplashBackend seems to render better quality and only slightly slower.

#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::SplashBackend
//#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::ArthurBackend


// Utility macro for managing pointers and logging

#define DELETE_LOG(X) if(X != NULL) { qDebug() << "Freeing " #X; delete X; X = NULL; }

#endif // PICONSTANTS
