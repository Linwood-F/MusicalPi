// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include "settingswidget.h"


settingsWidget::settingsWidget(QWidget* parent)
{
    this->setText(
        "<h1>MusicalPi</h1>"
        "<p>This feature is not yet implemented, and all settings are implemented inside of the code in piconstants.h.</p>"
      );
    this->setAlignment(Qt::AlignLeft);
    this->setAlignment(Qt::AlignTop);
    this->setWordWrap(true);
    this->setContentsMargins(10,10,10,10);
    this->setStyleSheet("background-color: rgb(240,240,200);");
    this->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);
    this->setFont(QFont("Arial",14));
}


