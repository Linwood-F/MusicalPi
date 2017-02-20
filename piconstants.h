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
#define MUSICALPI_BACKGROUND_COLOR_PLAYING "black"

#define MUSICALPI_SETTINGS_HEADING_FONT_SIZE 30
#define MUSICALPI_SETTINGS_SUBHEADING_FONT_SIZE 20
#define MUSICALPI_SETTINGS_TIPOVERLAY_FONT_SIZE 36
#define MUSICALPI_SETTINGS_PAGENUMBER_FONT_SIZE 16
#define MUSICALPI_SETTINGS_FORM_DATA_FONT_SIZE 20
#define MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH 90
#define MUSICALPI_SETTINGS_PATH_WIDTH 500
#define MUSICALPI_SETTINGS_STRING_LEN 200
#define MUSICALPI_SETTINGS_MSG_MIN_WIDTH 400

// Define this to get colored borders on key widgets (from stylesheet in main)
#define MUSICALPI_DEBUG_WIDGET_BORDERS

// SplashBackend seems to render better quality and only slightly slower.

#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::SplashBackend
//#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::ArthurBackend


// Utility macro for managing pointers and logging

#define DELETE_LOG(X) if(X != NULL) { qDebug() << "Freeing " #X; delete X; X = NULL; }

#endif // PICONSTANTS
