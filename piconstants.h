#ifndef PICONSTANTS
#define PICONSTANTS

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#define MUSICALPI_MAXPAGES 1000   // Maximum pages in any PDF -- minimal impact to make bigger

// review/play screen maximum "up" sizes - moderate memory impact if larger (but indirect large impact on cache as below)

#define MUSICALPI_MAXROWS 2
#define MUSICALPI_MAXCOLUMNS 4

// A Good rule of thumb is cores - 1

#define MUSICALPI_THREADS 3

#define MUSICALPI_BACKGROUND_COLOR_NORMAL "white"
#define MUSICALPI_BACKGROUND_COLOR_PLAYING "black"
#define MUSICALPI_POPUP_BACKGROUND_COLOR "rgb(240,240,200)"

#define MUSICALPI_SETTINGS_TIPOVERLAY_FONT_SIZE  "36px"
#define MUSICALPI_SETTINGS_PAGENUMBER_FONT_SIZE  "16px"

// Normally 1, but if you have notational scores (with lots of little elements) that
// are not playing properly, try 2.  This doubles the image from poppler then
// downscales to display.

#define MUSICALPI_TARGET_SCALE 2

// Define this to get colored borders on key widgets (from stylesheet in main)
//#define MUSICALPI_DEBUG_WIDGET_BORDERS

// SplashBackend seems to render better quality and only slightly slower.

#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::SplashBackend
//#define MUSICALPI_POPPLER_BACKEND Poppler::Document::RenderBackend::ArthurBackend

// Midi Player queue and debug control information

#define MUSICALPI_ALSALOWWATER 70
#define MUSICALPI_ALSAHIGHWATER 170
#define MUSICALPI_ALSAMAXOUTPUTBUFFER 512
#define MUSICALPI_ALSAQUEUECHUNKSIZE 50
#define MUSICALPI_ALSAPACINGINTERVAL 10
#define MUSICALPI_MIDIPLAYER_STATUSUPDATERATE 500
#define MUSICALPI_MEAURE_MARKER_TAG "Measure "

//#define MUSICALPI_OPEN_DOC_IN_THREAD

// Utility macro for managing pointers and logging

#define DELETE_LOG(X) if(X != NULL) { qDebug() << "Freeing " #X; delete X; X = NULL; }

#endif // PICONSTANTS
