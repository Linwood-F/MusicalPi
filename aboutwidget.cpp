
// Copyright 2023 by Linwood Ferguson, licensed under GNU GPLv3

#include "aboutwidget.h"
#include <QFont>
#include <QFile>
#include <QDebug>
#include "mainwindow.h"
#include "piconstants.h"
#include <algorithm>

aboutWidget::aboutWidget(QWidget* parent): QTextEdit(parent)
{
    QString tmpstr;
    tmpstr =
        "<h1>MusicalPi</h1>"
        "<p>MusicalPi is a QT programming project partly to learn QT, and partly "
           "to solve a problem of clutter atop our piano, and difficulty in turning pages "
           "in thick books. It is designed to run on a small computer (originally a Raspberry "
           "Pi, hence the name, but lately a Mini-PC for better performance) "
           "with a touch screen monitor (which from the OS will have a touch keyboard). "
           "The design concept has a screen large enough to show two sheets of music clearly, "
           "but the program is written so any number can show including 1 page.</p>"
        "<p>The program is based on maintaining a library of PDF's in Calibre, the open "
           "source eBook management program. Calibre is a convenient and extensible front end "
           "to hold your library and there is no reason to re-invent. In my case Calibre is "
           "running on a different computer, but it could also reside on the computer at the "
           "piano.</p>"
        "<p>An embedded midi player was included as our piano had a player system, and so "
           "I can embed a .mid file in calibre with the .pdf, and the piano will actually "
           "play it, allowing one to hear the proper timing (and slow it down if desired)."
           "As of this writing I have not tried to hook (e.g. via JACK) a synthesizer to the "
           "embedded midi player, but it quite likely would work as under the covers it uses "
           "ALSA.  It has been tested with qmidinet (though not on the Z83)."
        "<p>This software comes with ABSOLUTELY NO WARRANTY; for details on the license "
            "refer to the LICENSE document that accompanied the source code.</p>"
        "<p> Copyright Linwood Ferguson 2023 licensed under the GNU General Public License V3.0</p>"
                "<p><hr/></p>"
                "<pre>";


    QFile file("./LICENSE");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) {
            tmpstr = tmpstr + in.readLine() + "<br/>";
        }
    }
    tmpstr += "</pre>";

    this->setText(tmpstr);
    this->setReadOnly(true);
    this->setContentsMargins(10,10,10,10);
    // We stased the size of the main window, and can use this for a width setting
    MainWindow* mw = (MainWindow*)this->parent()->parent()->parent();
    this->setFixedSize(std::max((int)(mw->screenWidth*.5),800),mw->screenHeight*0.9);
    this->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

#ifndef MUSICALPI_DEBUG_WIDGET_BORDERS
    this->setStyleSheet("background-color:" MUSICALPI_POPUP_BACKGROUND_COLOR ";");
#endif
}

aboutWidget::~aboutWidget()
{
    qDebug() << "In destructor";
}
