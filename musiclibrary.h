#ifndef DBCALIBRE_H
#define DBCALIBRE_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include <QString>
#include <QWidget>
#include <QTime>

// This is a container widget that holds both a label, a search text box, the playlist drop down and a table widget with the library

class MainWindow;
class QSqlDatabase;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QComboBox;
class QTableWidget;
class QSqlError;
class QListView;

class musicLibrary : public QWidget
{
    Q_OBJECT

public:
    musicLibrary(QWidget *parent, MainWindow* mp);
    ~musicLibrary();

    void showKeyboard();         // If needed show the OnBoard keyboard
    void hideKeyboard();         //      or hide it
    bool bookInList(int);        // utility routine for other callers to find if current book is in a given list (by index into the dropdown)
    QString addBookToList(int);      // utility routine to link a book to a list
    QString removeBookFromList(int); // .... or remove one
    QString addNewList(QString);     // or add a completely new list (and reload the dropdown)

    QString ActiveList;     // Persist the (full) name of the tag for the currently active list
    int ActiveListIndex;    // index inside of dropdown
    int bookIDSelected;           // Save the ID in case we need to update playlists
    QString calibreListPrefix;    // Prefix for all play lists

private:
    QSqlDatabase* m_db;
    QWidget* ourParent;
    MainWindow* mParent;

    int columnForPath;     // Stash column in libTable where we store these items in case we need them
    int columnForID;
    int columnForTitle;


    // widget structure as indicated below
    // this (widget) contains...
       QVBoxLayout* baseLayout;
          QWidget* search;
              QHBoxLayout* searchLayout;
                  QLabel* prompt;               // Prompt for search box
                  QLineEdit* searchBox;         // Search box for data entry
public:           QComboBox* dropdown;          // dropdown combo to select the active play list
private:          QListView* listDropdown;      // Used to view the combo
          QTableWidget* libTable;               // The music library display (filtered by list doesn't retrieve items, by search just hides rows)

    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *);

    QString calibreMusicTag;    // Stash settings values
    QString calibrePath;
    QString calibreDatabase;
    bool forceOnboardKeyboard;

    int lastRowSelected;       // Used to prevent mouse-bounce from doing a select twice while processing (reported by Stevend on raspberrypi.org 2/5/17 - thanks)
    QTime lastTimeSelected;    // Ignore the last row selected if it's been over a few seconds (not bounch, just reselect)

    QString pathSelected;
    QString titleSelected;

    void checkSqlError(QString stage, QSqlError err); // utility routine after any sql call

    void loadPlayLists();        // Load the dropdown
    void loadBooks();            // Load the library

    void showEvent(QShowEvent *e);  // Overriden so we know when to load or release our data
    void hideEvent(QHideEvent *e);
    bool screenLoaded;    // We will only load it once, so keep track (this means we won't see library changes without exit)


signals:
    void songSelected(QString, QString);

private slots:
    void onChosen(int, int);
    void filterTable(QString);
    void changeList(int);
};


#endif // DBCALIBRE_H
