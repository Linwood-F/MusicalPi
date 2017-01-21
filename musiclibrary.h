#ifndef DBCALIBRE_H
#define DBCALIBRE_H

// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QtSql/qtsqlglobal.h>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTimer>
#include <Qt>
#include <QScroller>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>


#include "debugmessages.h"
#include "piconstants.h"
#include <cassert>

// This is a container widget that holds both a label, a search text box, and a table widget

class musicLibrary : public QWidget
{
    Q_OBJECT
public:
    musicLibrary(QWidget *parent = 0);
    ~musicLibrary();
    void loadData();
    void showKeyboard();
    void hideKeyboard();

private:
    QSqlDatabase m_db;
    QWidget* mParent;
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


signals:
    void songSelected(QString, QString);

private slots:
    void onChosen(int, int);
    void filterTable(QString);


};


#endif // DBCALIBRE_H
