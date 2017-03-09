// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QPainter>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>

#include <QHeaderView>
#include <QScroller>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QEvent>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QListView>

#include "musiclibrary.h"
#include "mainwindow.h"
#include "oursettings.h"
#include "piconstants.h"
#include <cassert>


musicLibrary::musicLibrary(QWidget *parent, MainWindow* mp) : QWidget(parent)
{
    // Isolate specific librarydetails in this class so we could switch to different
    // calling process, also isolate formatting and population of the widget here.

    // This widget (i.e. "this") is persistent and created once.  The inner widgets that
    // represent the data are deleted and rebuilt each time with the hide and show events
    // This overhead is relatively small, but since by design this program stays running
    // all the time, it otherwise will not see new music (i.e. calibre books). Notice the
    // database is opened each time it is read in case the share dropped out and came back
    // at some point.

    ourParent = parent;
    mParent = mp;
    calibreMusicTag   =  mParent->ourSettingsPtr->getSetting("calibreMusicTag").toString();
    calibrePath       =  mParent->ourSettingsPtr->getSetting("calibrePath").toString();
    calibreDatabase   =  mParent->ourSettingsPtr->getSetting("calibreDatabase").toString();
    calibreListPrefix = mParent->ourSettingsPtr->getSetting("calibreListPrefix").toString();
    forceOnboardKeyboard = mParent->ourSettingsPtr->getSetting("forceOnboardKeyboard").toBool();

    ActiveList = "All items";

    m_db = NULL;

    qDebug() << "in constructor with path " << calibrePath << " and file " << calibreDatabase;

    // Create widgets and layouts

    baseLayout = new QVBoxLayout(this);
    search = new QWidget(this);
    searchLayout = new QHBoxLayout(search);
    prompt = new QLabel(search);
    searchBox = new QLineEdit(search);
    dropdown= new QComboBox(search);
    listDropdown = new QListView(dropdown);
    dropdown->setView(listDropdown);
    libTable = new QTableWidget(this);

    // Arrange the widgets

    this->setLayout(baseLayout);
    baseLayout->addWidget(search);
    baseLayout->addWidget(libTable);
    search->setLayout(searchLayout);
    searchLayout->addWidget(prompt);
    searchLayout->addWidget(searchBox);
    searchLayout->addWidget(dropdown);

    prompt->setFont(QFont("Arial",16,1));
    prompt->setText("Search for:");
    searchBox->setMinimumWidth(300);  // Looks dumb if too short, but also if too long
    searchBox->setMaximumWidth(300);
    searchBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    searchBox->setAlignment(Qt::AlignLeft);
    searchLayout->setAlignment(Qt::AlignLeft);
    searchBox->installEventFilter(search);  // So we can catch keystrokes and do as-you-type filter


    libTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    libTable->setSelectionMode(QAbstractItemView::SingleSelection);
    libTable->verticalHeader()->hide();
    libTable->setAlternatingRowColors(true);
    libTable->setSortingEnabled(true);

    QScroller::grabGesture(libTable,QScroller::LeftMouseButtonGesture);  // so a touch-drag will work, otherwise need two finger drag
    libTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    libTable->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(libTable, SIGNAL(cellClicked(int,int)), this, SLOT(onChosen(int,int)));  // Maybe this should be double clicked?
    connect(searchBox, SIGNAL(textChanged(QString)), this, SLOT(filterTable(QString)));

    m_db = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
    m_db->setDatabaseName(calibrePath + "/" + calibreDatabase);

}

musicLibrary::~musicLibrary()
{
    qDebug() << "in destructor";
}


void musicLibrary::loadPlayLists()
{
    // Get a list of playlists and set it to active if we already had chosen it
    // Must have database open to call

    qDebug() << "Entered";
    dropdown->clear();
    QSqlQuery queryLists;
    QString sql = "select id, name from tags where name like '" + calibreListPrefix + "%';";
    queryLists.prepare(sql);
    checkSqlError("Prepared tag query " + sql, queryLists.lastError());
    queryLists.exec();
    checkSqlError("Executed tag query " + sql, queryLists.lastError());
    dropdown->addItem("All items",0);
    while(queryLists.next())
    {
        qDebug()<<"read tags, id=" << queryLists.record().field(0).value().toString() << ", value=" << queryLists.record().field(0).value().toInt();
        dropdown->addItem(queryLists.record().field(1).value().toString(),queryLists.record().field(1).value().toString());
    }
    int index = dropdown->findData(ActiveList);
    if(index == -1) index = 0;  // If the current active disappeareed reset to all
    dropdown->setCurrentIndex(index);
    connect(dropdown,SIGNAL(currentTextChanged(QString)),this,SLOT(changeList(QString)));
}

void musicLibrary::loadBooks()
{
    // Must have database open to call
    lastRowSelected=-1; // None selected (yet)
    libTable->setRowCount(0);
    searchBox->setText("");  // Start fresh search each time
    QSqlQuery queryBooks;
    QString sql =
        "select b.id as BookID, b.sort as Title, coalesce(max(s.name),'') as Collection, b.author_sort as Author, b.path || '/' ||  d.name || '.' || d.format as Path, group_concat(t2.name,',') as tags "
        "from books b "
        "inner join books_tags_link btl on btl.book = b.id "
        "inner join tags t on t.id = btl.tag "
        "inner join data d on d.book = b.id and d.format = 'PDF' " +
        (
           ActiveList == "All items" ? "" :
              ( "inner join books_tags_link btl3 on btl3.book = b.id "
                "inner join tags t3 on t3.id = btl3.tag and t3.name = '" + ActiveList + "' "
              )
        ) +
        "left join books_series_link bsl on bsl.book = b.id "
        "left join series s on s.id=bsl.series "
        "left join books_tags_link btl2 on btl2.book=b.id "
        "left join tags t2 on t2.id=btl2.tag and t2.name not like 'musicList%' and t2.name <> 'music' "
        "where t.name = '" + calibreMusicTag + "' "
        "group  by b.id, b.sort, b.author_sort, b.path, d.name "
        "order by b.sort;";
    queryBooks.prepare(sql);
    checkSqlError("Prepared book query " + sql, queryBooks.lastError());
    queryBooks.exec();
    checkSqlError("Executed book query " + sql, queryBooks.lastError());
    QSqlRecord rec = queryBooks.record();
    libTable->setFont(QFont("Arial",10,0,false));
    libTable->setColumnCount(rec.count());
    // First run through the columns and code appropriately in the table
    for(int field = 0; field < rec.count(); field++)
    {
        libTable->setHorizontalHeaderItem(field, new QTableWidgetItem(rec.fieldName(field)));
        if(rec.fieldName(field)=="BookID")
        {
            libTable->setColumnHidden(field,true);
            columnForID=field;
        }
        if(rec.fieldName(field)=="Path")  // Remember where we put this
        {
            libTable->setColumnHidden(field,true);
            columnForPath=field;
        }
        if(rec.fieldName(field)=="Title")
        {
            columnForTitle=field;  // And remember this one
        }
    }
    int row=0;
    while(queryBooks.next())
    {
        libTable->setRowCount(row+1);
        for(int field=0; field < rec.count(); field++)
        {
            libTable->setItem(row,field,new QTableWidgetItem(queryBooks.record().field(field).value().toString()));
            if(rec.fieldName(field)=="Title") libTable->item(row,field)->setFont(QFont("Arial",14,1,false));
        }
        row++;
    }
    libTable->resizeColumnsToContents();

    qDebug() << "Completed by book retrieval and build of table";

}

void musicLibrary::showEvent(QShowEvent *e)
{
    qDebug() << "Entered";
    m_db->open();
    checkSqlError("Opening SQL database " + m_db->databaseName(), m_db->lastError());
    loadPlayLists();
    loadBooks();
    m_db->close();
}

void musicLibrary::changeList(QString newList)
{
    qDebug() << "Entered with " << newList;
    ActiveList = newList;
    m_db->open();
    checkSqlError("Opening SQL database " + m_db->databaseName(), m_db->lastError());
    loadBooks();
    m_db->close();
}

void musicLibrary::checkSqlError(QString stage, QSqlError err)
{
    if(err.databaseText()!="" || err.driverText()!="")
    {
        qDebug() << stage << " resulted in error " << err;
        assert(err.databaseText()=="" || err.driverText()=="");
    }
}

void musicLibrary::hideEvent(QHideEvent *e)
{
    qDebug() << "Entered";
    // We can't delete this: DELETE_LOG(libTable);
    // as for some reason the slignal/slot segfaults, so just clear it out
    libTable->setRowCount(0);  // THis just saves some memory when we're not using it
    disconnect(dropdown,SIGNAL(currentTextChanged(QString)),this,SLOT(changeList(QString))); // Need this so we don't signal when we reload it
}

void musicLibrary::onChosen(int row, int column)
{
    qDebug() << "Entered";
    if(lastRowSelected == row)
    {
        qDebug() << "doubleclicked on row " << row <<  " which looks like a duplicate so ignoring.";
        return;
    }
    else
    {
        // Move these to local items since the following are going to disappear
        pathSelected = tr("%1/%2").arg(calibrePath).arg(libTable->item(row,columnForPath)->text());
        titleSelected = libTable->item(row,columnForTitle)->text();
        qDebug() << "doubleclicked on row " << row <<  " column " << column << ", value=" << pathSelected;
        lastRowSelected = row;
        emit songSelected(pathSelected, titleSelected);
    }
    qDebug() << "exiting";
}

void musicLibrary::filterTable(QString filter)
{
    bool match;
    qDebug() << tr("Filtering for string '%1'").arg(filter);
    libTable->setUpdatesEnabled(false);
    for(int row = 0; row < libTable->rowCount(); row++)
    {
        if(filter=="")  match=true; // With no filter show everything
        else
        {  // see if any not-hidden field matches and show
            match = false;
            for(int col = 0; col < libTable->columnCount(); col++)
                if (columnForPath != col && columnForID != col &&  libTable->item(row,col)->text().contains(filter,Qt::CaseInsensitive))
                {
                    match = true;
                    break;
                }
        }
        if(match) libTable->showRow(row);
        else libTable->hideRow(row);
    }
    libTable->setUpdatesEnabled(true);
}

void musicLibrary::showKeyboard()
{
    if(!forceOnboardKeyboard) return;
    qDebug() << "Firing show keyboard";
    QDBusMessage show = QDBusMessage::createMethodCall("org.onboard.Onboard","/org/onboard/Onboard/Keyboard","org.onboard.Onboard.Keyboard","Show");
    QDBusConnection::sessionBus().send(show);
}

void musicLibrary::hideKeyboard()
{
    if(!forceOnboardKeyboard) return;
    qDebug() << "Firing hide keyboard";
    QDBusMessage show = QDBusMessage::createMethodCall("org.onboard.Onboard","/org/onboard/Onboard/Keyboard","org.onboard.Onboard.Keyboard","Hide");
    QDBusConnection::sessionBus().send(show);
}

bool musicLibrary::eventFilter(QObject *object, QEvent *event)
{
    // This is a kludge because Onboard does not work in auto-hide mode with QT, it flashes in and out
    // This forces it in when the seaerch is active, and forces it out when not.
    if(event->type() == QEvent::FocusIn && object == searchBox) showKeyboard();
    else if(event->type() == QEvent::FocusOut && object == searchBox) hideKeyboard();
    return false;
}

void musicLibrary::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
