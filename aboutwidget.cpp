
// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include "aboutwidget.h"

aboutWidget::aboutWidget(QWidget* parent): QLabel(parent)
{
    this->setText(
        "<h1>MusicalPi</h1>"
        "<p>MusicalPi is a QT programming project partly to learn QT, and partly "
           "to solve a problem of clutter atop our piano, and difficulty in turning pages "
           "in thick books. It is designed to run on a small computer (e.g. a Raspberry "
           "Pi) with a touch screen monitor (which from the OS will have a touch keyboard). "
           "The design concept has a screen large enough to show two sheets of music clearly, "
           "but the program is written so any number can show including 1 page.</p>"
        "<p>The program is based on maintaining a library of PDF's in Calibre, the open "
           "source eBook management program. Calibre is a convenient and extensible front end "
           "to hold your library and there is no reason to re-invent. In my case Calibre is "
           "running on a different computer, but it could also reside on the computer at the "
           "piano.</p>"
        "<p>This software is provided under the Apache 2.0 license, and is Copyright(C) 2017 by "
        "LE Ferguson, LLC and all rights are reserved except as provided therein.</p>"
      );
    this->setAlignment(Qt::AlignLeft);
    this->setAlignment(Qt::AlignTop);
    this->setWordWrap(true);
    this->setContentsMargins(10,10,10,10);
#ifndef MUSICALPI_DEBUG_WIDGET_BORDERS
    this->setStyleSheet("background-color: rgb(240,240,200);");
#endif
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
    this->setFont(QFont("Arial",14));
}

aboutWidget::~aboutWidget()
{
    qDebug() << "In destructor";
}
