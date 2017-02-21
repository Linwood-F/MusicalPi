#ifndef DBCALIBRE_H
#define DBCALIBRE_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QtSql/qtsqlglobal.h>
#include <QSqlDatabase>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <QTableWidget>


// This is a container widget that holds both a label, a search text box, and a table widget

class MainWindow;

class musicLibrary : public QWidget
{
    Q_OBJECT
public:
    musicLibrary(QWidget *parent, MainWindow* mp);
    ~musicLibrary();
    void loadData();
    void showKeyboard();
    void hideKeyboard();

private:
    QSqlDatabase m_db;
    QWidget* ourParent;
    MainWindow* mParent;
    int columnForPath;
    int columnForID;
    int columnForTitle;
    QTableWidget libTable;
    QWidget search;
    QHBoxLayout searchLayout;
    QVBoxLayout baseLayout;
    QLabel prompt;
    QLineEdit searchBox;
    QTimer keepKeyboardUp;
    bool eventFilter(QObject *object, QEvent *event);
    void paintEvent(QPaintEvent *);


signals:
    void songSelected(QString, QString);

private slots:
    void onChosen(int, int);
    void filterTable(QString);


};


#endif // DBCALIBRE_H
