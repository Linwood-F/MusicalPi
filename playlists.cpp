#include "playlists.h"
#include "musiclibrary.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListView>
#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QFrame>

#include "piconstants.h"

#include <cassert>

playLists::playLists(MainWindow* parent, musicLibrary* lib, QString thisTitle, int BookId)
{
    qDebug() << "Entered";
    mParent = parent;
    mMusicLibrary = lib;
    mTitle = thisTitle;
    mBookID = BookId;
    setWindowTitle("Playlists for - " + mTitle);
    this->setWindowFlags(Qt::Window|Qt::Dialog);
    this->setObjectName("playLists");

    this->setProperty("Popup",true);  // Can I style all these from this???   All children? ??

    this->setStyleSheet("QLabel    {background-color: " MUSICALPI_POPUP_BACKGROUND_COLOR "}"
                        "QFrame    {background-color: " MUSICALPI_POPUP_BACKGROUND_COLOR "}"
                        "playLists {background-color: " MUSICALPI_POPUP_BACKGROUND_COLOR "}"
                        );

    gl = new QGridLayout(this);

    titlePrompt= new QLabel(this);
    changePrompt = new QLabel(this);
    dropdown = new QComboBox(this);
    listDropdown = new QListView(dropdown);
    dropdown->setObjectName("dropdown");
    dropdown->setView(listDropdown);
    changeState = new QPushButton(this);

    // Show our title
    gl->addWidget(titlePrompt,0,0,1,2,Qt::AlignCenter);

    // to change lists' contents
    gl->addWidget(changePrompt,1,0,1,2,Qt::AlignLeft);
    gl->addWidget(dropdown,    2,0,1,1,Qt::AlignLeft); // Note we don't put the qlistview here or for reasons unclear it screws up.
    gl->addWidget(changeState, 2,1,1,1,Qt::AlignRight);

    // Separator line
    hline = new QFrame(this);
    hline->setFrameShape(QFrame::HLine);
    hline->setFrameShadow(QFrame::Sunken);
    gl->addWidget(hline,    3,0,1,2);
    hline->setFixedHeight(30);

    // To create a new list
    addPrompt = new QLabel(this);
    newList = new QLineEdit(this);
    saveNew = new QPushButton(this);
    gl->addWidget(addPrompt,4,0,1,2,Qt::AlignLeft);
    gl->addWidget(newList,  5,0,1,1,Qt::AlignLeft);
    gl->addWidget(saveNew,  5,1,1,1,Qt::AlignRight);

    errorMsg = new QLabel(this);
    gl->addWidget(errorMsg, 6,0,1,2,Qt::AlignLeft);

    titlePrompt->setText("Song: " + mTitle);
    titlePrompt->setProperty("Heading",true);

    changePrompt->setText("Use dropdown to pick a list to change as indicated");
    changeState->setText("Save");

    addPrompt->setText("Use this to add entirely new list (title is added automatically):");
    saveNew->setText("Add New");
    newList->setText("");
    newList->setMinimumWidth(300);

    errorMsg->setProperty("ErrorMessage",true);

    loadDropdown();

    connect(dropdown,SIGNAL(currentIndexChanged(int)),this,SLOT(changeList(int)));
    connect(newList,SIGNAL(textChanged(QString)),this,SLOT(newListChanged(QString)));
    connect(changeState,SIGNAL(pressed()),this,SLOT(doChangeState()));
    connect(saveNew,SIGNAL(pressed()),this,SLOT(doSaveNew()));

    this->setLayout(gl);
    this->show();
    // Do this after show so it sizes appropriately
    saveNew->hide();
    changeList(mMusicLibrary->ActiveListIndex);
}

void playLists::loadDropdown()
{
    // The musicLibrary knows this info already, for loading we just load its dropdown data (which it persists)
    dropdown->clear();
    dropdown->addItem("Pull down to select list/action",0);
    for(int i=1; i < mMusicLibrary->dropdown->count(); i++)     // Skip zero which is the "All lists"; if we have no others this is empty
    {
        if(mMusicLibrary->bookInList(mMusicLibrary->dropdown->itemData((i)).toInt()))
            dropdown->addItem("Remove: " + mMusicLibrary->dropdown->itemText(i) + " (already present)",mMusicLibrary->dropdown->itemData(i));
        else
            dropdown->addItem("Add into: " + mMusicLibrary->dropdown->itemText(i),mMusicLibrary->dropdown->itemData(i));
    }
    dropdown->setCurrentIndex(mMusicLibrary->ActiveListIndex);
}

void playLists::changeList(int newIndex)
{
    qDebug() << "new index = " << newIndex << ", which is " << dropdown->itemText(newIndex) << " with value " << dropdown->itemData(newIndex).toInt();
    if(newIndex == 0) changeState->hide();
    else changeState->show();
}

void playLists::newListChanged(QString val)
{
    if(val == "") saveNew->hide();
    else saveNew->show();
}

void playLists::doChangeState()
{
    assert(dropdown->currentIndex()); // Shouldn't be able to get here with the zero index as button shoul dbe hidden
    QString result;
    if(dropdown->currentText().left(3)=="Add") result = mMusicLibrary->addBookToList(dropdown->currentIndex());
    else result = mMusicLibrary->removeBookFromList(dropdown->currentIndex());
    errorMsg->setText(result);
    if(result=="")
    {
        loadDropdown();
    }
}
void playLists::doSaveNew()
{
    QString result;
    result = mMusicLibrary->addNewList(mMusicLibrary->calibreListPrefix + newList->text());
    errorMsg->setText(result);
    if(result=="")
    {
        loadDropdown();
        newList->setText("");
    }
}
