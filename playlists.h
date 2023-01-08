#ifndef PLAYLISTS_H
#define PLAYLISTS_H

// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

// Responsible for modifying playlists contents and/or creating a playlist
// based on the now-current title (only).


#include <QWidget>

class MainWindow;
class musicLibrary;
class QVBoxLayout;
class QGridLayout;
class QLineEdit;
class QPushButton;
class QListView;
class QComboBox;
class QLabel;
class QFrame;

class playLists : public QWidget
{
    Q_OBJECT

public:
    playLists(MainWindow* parent, musicLibrary* lib, QString thisTitle, int BookId);

private:
    MainWindow* mParent;
    musicLibrary* mMusicLibrary;
    QString mTitle;
    int mBookID;

    // this - layout of widgets on this widget
        QGridLayout* gl;
            QLabel* titlePrompt;
            QLabel* changePrompt;
            QComboBox*   dropdown; QListView*   listDropdown; /*view of dropdown */   QPushButton* changeState;
            QFrame*      hline;
            QLabel*      addPrompt;
            QLineEdit*   newList;    QPushButton* saveNew;
            QLabel*  errorMsg;

    void loadDropdown();

public slots:
    void changeList(int);
    void newListChanged(QString);
    void doChangeState();
    void doSaveNew();

};

#endif // PLAYLISTS_H
