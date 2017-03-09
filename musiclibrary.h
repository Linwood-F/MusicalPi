#ifndef DBCALIBRE_H
#define DBCALIBRE_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QString>
#include <QWidget>


// This is a container widget that holds both a label, a search text box, and a table widget

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
    void showKeyboard();
    void hideKeyboard();

private:
    QSqlDatabase* m_db;
    QWidget* ourParent;
    MainWindow* mParent;

    int columnForPath;
    int columnForID;
    int columnForTitle;

    QString ActiveList;

    // widget structure as indicated below
    // this (widget) contains...
       QVBoxLayout* baseLayout;
          QWidget* search;
              QHBoxLayout* searchLayout;
                  QLabel* prompt;
                  QLineEdit* searchBox;
                  QComboBox* dropdown;
                  QListView* listDropdown; // Used to view the combo
          QTableWidget* libTable;

    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *);
    QString calibreMusicTag;
    QString calibrePath;
    QString calibreDatabase;
    QString calibreListPrefix;
    int lastRowSelected;       // Used to prevent mouse-bounce from doing a select twice while processing (reported by Stevend on raspberrypi.org 2/5/17 - thanks)
    bool forceOnboardKeyboard;

    QString pathSelected;
    QString titleSelected;

    void showEvent(QShowEvent *e);
    void hideEvent(QHideEvent *e);
    void checkSqlError(QString stage, QSqlError err);

    void loadPlayLists();
    void loadBooks();

signals:
    void songSelected(QString, QString);

private slots:
    void onChosen(int, int);
    void filterTable(QString);
    void changeList(QString newList);
};


#endif // DBCALIBRE_H
