// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "musiclibrary.h"
#include "mainwindow.h"

// Isolate specific librarydetails in this class so we could switch to different
// calling process, also isolate formatting and population of the widget here.
// Basically the caller can only load it, and gets a signal from selection.

musicLibrary::musicLibrary(QWidget *parent, MainWindow* mp) : QWidget(parent)
{
    ourParent = parent;
    mParent = mp;
    qDebug() << "in constructor with path " << mParent->ourSettingsPtr->calibrePath << " and file " << mParent->ourSettingsPtr->calibreDatabase;

    // Arrange the widgets

    this->setLayout(&baseLayout);
    baseLayout.addWidget(&search);
    baseLayout.addWidget(&libTable);
    search.setLayout(&searchLayout);
    searchLayout.addWidget(&prompt);
    searchLayout.addWidget(&searchBox);
    prompt.setFont(QFont("Arial",16,1));
    prompt.setText("Search for:");
    searchBox.setMinimumWidth(300);  // Looks dumb if too short, but also if too long
    searchBox.setMaximumWidth(300);
    searchBox.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    searchBox.setAlignment(Qt::AlignLeft);
    searchLayout.setAlignment(Qt::AlignLeft);
    searchBox.installEventFilter(this);  // So we can catch keystrokes and do as-you-type filter

    // Get the database ready
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(mParent->ourSettingsPtr->calibrePath + "/" + mParent->ourSettingsPtr->calibreDatabase);
    if(m_db.open())
    {
        qDebug() << "Successfully opened database, lasterror=" << m_db.lastError();
    }
    else
    {
        qDebug() << "Failed to open database (where is error ??? )";
    }
}

musicLibrary::~musicLibrary()
{
    qDebug() << "in destructor";
    if(m_db.isOpen()) m_db.close();
}

void musicLibrary::loadData()
{
    qDebug()<<"Starting to load data";
    searchBox.setText("");  // Start fresh search with new library
    QSqlQuery queryBooks;
    QString sql =
        "select b.id as BookID, b.sort as Title, coalesce(max(s.name),'') as Collection, b.author_sort as Author, b.path || '/' ||  d.name || '.' || d.format as Path "
        "from books b "
        "inner join books_tags_link btl on btl.book = b.id "
        "inner join tags t on t.id = btl.tag "
        "inner join data d on d.book = b.id and d.format = 'PDF' "
        "left join books_series_link bsl on bsl.book = b.id "
        "left join series s on s.id=bsl.series "
        "where t.name = '" + mParent->ourSettingsPtr->calibreMusicTag + "' "
        "group  by b.id, b.sort, b.author_sort, b.path, d.name "
        "order by b.sort;";
    qDebug() << "Preparing main query " << sql ;
    queryBooks.prepare(sql);
    qDebug() << "Prepared with lasterror=" << queryBooks.lastError();
    queryBooks.exec();
    qDebug() << "Executed with lasterror=" << queryBooks.lastError();
    QSqlRecord rec = queryBooks.record();
    libTable.setFont(QFont("Arial",10,0,false));
    libTable.setColumnCount(rec.count()+1);
    for(int field = 0; field < rec.count(); field++)
    {
        libTable.setHorizontalHeaderItem(field, new QTableWidgetItem(rec.fieldName(field)));
        if(rec.fieldName(field)=="BookID")
        {
            libTable.setColumnHidden(field,true);
            columnForID=field;
        }
        if(rec.fieldName(field)=="Path")  // Remember where we put this
        {
            libTable.setColumnHidden(field,true);
            columnForPath=field;
        }
        if(rec.fieldName(field)=="Title")
        {
            columnForTitle=field;  // And remember this one
        }
    }
    libTable.setHorizontalHeaderItem(libTable.columnCount()-1, new QTableWidgetItem("Tags"));
    int row=0;
    while(queryBooks.next())
    {
        libTable.setRowCount(row+1);
        for(int field=0; field < rec.count(); field++)
        {
            libTable.setItem(row,field,new QTableWidgetItem(queryBooks.record().field(field).value().toString()));
            if(rec.fieldName(field)=="Title") libTable.item(row,field)->setFont(QFont("Arial",14,1,false));
        }
        QSqlQuery queryTags;
        queryTags.prepare(
          tr("select t.name "
             "from books b "
             "inner join books_tags_link btl on btl.book = b.id "
             "inner join tags t on t.id = btl.tag "
             "where b.id=%1 and t.name <> '%2' ").arg(queryBooks.record().field(rec.indexOf("BookID")).value().toString()).arg(mParent->ourSettingsPtr->calibreMusicTag)
            );
        if(queryTags.lastError().databaseText()!="" || queryTags.lastError().driverText()!="")
        {
            qDebug() << "Tag query Prepared with lasterror=" << queryBooks.lastError();
        }
        queryTags.exec();
        if(queryTags.lastError().databaseText()!="" || queryTags.lastError().driverText()!="")
        {
            qDebug() << "Tag query Executed with lasterror=" << queryBooks.lastError();
        }
        bool anyTags = queryTags.next();
        if(queryTags.lastError().databaseText()!="" || queryTags.lastError().driverText()!="")
        {
            qDebug() << "Tag query next failed lasterror=" << queryBooks.lastError() ;
        }
        libTable.setItem(row,libTable.columnCount() - 1,new QTableWidgetItem(anyTags ? queryTags.record().field(0).value().toString() : "" ));
        row++;
    }
    libTable.setSelectionBehavior(QAbstractItemView::SelectRows);
    libTable.setSelectionMode(QAbstractItemView::SingleSelection);
    libTable.verticalHeader()->hide();
    libTable.resizeColumnsToContents();
    libTable.setAlternatingRowColors(true);
    libTable.setSortingEnabled(true);

    QScroller::grabGesture(&libTable,QScroller::LeftMouseButtonGesture);  // so a touch-drag will work, otherwise need two finger drag
    libTable.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    libTable.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(&libTable, SIGNAL(cellClicked(int,int)), this, SLOT(onChosen(int,int)));  // Maybe this should be double clicked?
    connect(&searchBox, SIGNAL(textChanged(QString)), this, SLOT(filterTable(QString)));
}
void musicLibrary::onChosen(int row, int column)
{
    qDebug() << "doubleclicked on row " << row <<  " column " << column << ", value=" << mParent->ourSettingsPtr->calibrePath <<  "/" << libTable.item(row, columnForPath)->text();
    emit songSelected(tr("%1/%2").arg(mParent->ourSettingsPtr->calibrePath).arg(libTable.item(row,columnForPath)->text()),libTable.item(row,columnForTitle)->text());
}

void musicLibrary::filterTable(QString filter)
{
    bool match;
    qDebug() << tr("Filtering for string '%1'").arg(filter);
    libTable.setUpdatesEnabled(false);
    for(int row = 0; row < libTable.rowCount(); row++)
    {
        if(filter=="")  match=true; // With no filter show everything
        else
        {  // see if any not-hidden field matches and show
            match = false;
            for(int col = 0; col < libTable.columnCount(); col++)
                if (columnForPath != col && columnForID != col &&  libTable.item(row,col)->text().contains(filter,Qt::CaseInsensitive))
                {
                    match = true;
                    break;
                }
        }
        if(match) libTable.showRow(row);
        else libTable.hideRow(row);
    }
    libTable.setUpdatesEnabled(true);
}
void musicLibrary::showKeyboard()
{
    qDebug() << "Firing show keyboard";
    QDBusMessage show = QDBusMessage::createMethodCall("org.onboard.Onboard","/org/onboard/Onboard/Keyboard","org.onboard.Onboard.Keyboard","Show");
    QDBusConnection::sessionBus().send(show);
}
void musicLibrary::hideKeyboard()
{
    qDebug() << "Firing show keyboard";
    QDBusMessage show = QDBusMessage::createMethodCall("org.onboard.Onboard","/org/onboard/Onboard/Keyboard","org.onboard.Onboard.Keyboard","Hide");
    QDBusConnection::sessionBus().send(show);
}

bool musicLibrary::eventFilter(QObject *object, QEvent *event)
{
    // This is a kludge because Onboard does not work in auto-hide mode with QT, it flashes in and out
    // This forces it in when the seaerch is active, and forces it out when not.

    if(event->type() == QEvent::FocusIn && object == &searchBox) showKeyboard();
    else if(event->type() == QEvent::FocusOut && object == &searchBox) hideKeyboard();
    return false;
}

void musicLibrary::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
