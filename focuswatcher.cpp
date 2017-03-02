#include "focuswatcher.h"
#include <QEvent>

// Courtesy Emmanuel Lepage @  http://stackoverflow.com/questions/17818059/what-is-the-signal-for-when-a-widget-loses-focus/17819214

// Sends signal if focus-out event occurs

FocusWatcher::FocusWatcher(QObject* parent) : QObject(parent)
{
    parent->installEventFilter(this);
}
bool FocusWatcher::eventFilter(QObject *obj, QEvent *event)
{
    (void)obj;
    if (event->type() == QEvent::FocusOut) emit focusChanged(false);
    return false;
}

