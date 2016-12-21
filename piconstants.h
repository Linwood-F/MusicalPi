#ifndef PICONSTANTS
#define PICONSTANTS

#define MUSICALPI_MAXPAGES 1000   // Maximum pages in any PDF -- minimal impact to make bigger

// review/play screen maximum "up" sizes - moderate memory impact if larger (but indirect large impact on cache as below)
#define MUSICALPI_MAXROWS 2
#define MUSICALPI_MAXCOLUMNS 4

// A Good rule of thumb is cores - 1
#define MUSICALPI_THREADS 3

// This one can be smaller than calculated below, but is set to allow twice the number of pages
// ahead of, and the same number behind, the max "up" display, plus some slop, so page forward/back works.
// The 2:1 ratio of ahead to behind is hard coded into the cache range calculation routine
// Large impact if larger; if "up" changes and this gets too large it can be smaller and will
// only really impact performance when paging through with very high "up" size.
#define MUSICALPI_MAX_CACHE (4 * (MUSICALPI_MAXROWS * MUSICALPI_MAXCOLUMNS) + 3) // max pages to cache

// The where-to-touch overlay size and duration
#define MUSICALPI_OVERLAY_DURATION 3000 // time in milliseconds total
#define MUSICALPI_OVERLAY_TOP_PORTION 0.20   // portion of screen for top (stop playing) area
#define MUSICALPI_OVERLAY_SIDE_PORTION 0.40  // portion of screen for each side (back, forward) area

// Page turn controls
#define MUSICALPI_PAGE_TURN_DELAY 3200   // time in ms
#define MUSICALPI_PAGE_HIGHLIGHT_DELAY 1500   // time in ms
#define MUSICALPI_PAGE_HIGHLIGHT_HEIGHT 10   // half-page indicator

// Normal and playing styles ?? need more styling here I think ??
#define MUSICALPI_BORDERS 10  // space between pages
#define MUSICALPI_BACKGROUND_COLOR_NORMAL "white"
#define MUSICALPI_BACKGROUND_COLOR_NORMAL_QT Qt::white
#define MUSICALPI_BACKGROUND_COLOR_PLAYING "black"
#define MUSICALPI_BACKGROUND_COLOR_PLAYING_QT Qt::black

#define POPPLER_BACKEND Poppler::Document::RenderBackend::SplashBackend
//#define POPPLER_BACKEND Poppler::Document::RenderBackend::ArthurBackend

// Calibre path must be reachable and writeable (eventually)
// Sample mount command: mount -t cifs //lef.leferguson.com/t /mnt/lef/t -o username=ferguson   (then enter password)
// ??? does this survive a reboot ??? Disconnection ???  -- no, needs to be set up differently
#define MUSICALPI_CALIBRE_PATH "/mnt/lef/t/Calibre Library"
#define MUSICALPI_CALIBRE_DATABASE "metadata.db"
// Tag must be set up in Calibre
#define MUSICALPI_CALIBRE_TAG "music"
// Cosmetic - set fraction of screen width logo takes up
#define MUSICALPI_LOGO_PCT 0.20

#endif // PICONSTANTS

