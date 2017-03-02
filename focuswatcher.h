#ifndef FOCUSWATCHER_H
#define FOCUSWATCHER_H
// Courtesy Emmanuel Lepage @  http://stackoverflow.com/questions/17818059/what-is-the-signal-for-when-a-widget-loses-focus/17819214

#include <QObject>

class QEvent;

class FocusWatcher : public QObject
{
   Q_OBJECT
public:
   explicit FocusWatcher(QObject*);
   virtual bool eventFilter(QObject *obj, QEvent *event) override;
Q_SIGNALS:
   void focusChanged(bool in);
};

#endif // FOCUSWATCHER_H
