// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QApplication>
#include <debugmessages.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qInstallMessageHandler(myMessageOutput);
    a.setStyleSheet(QString(
        "QWidget"
           "{margin: 0px; "
            "padding: 0px; "
            "background-color: " MUSICALPI_BACKGROUND_COLOR_NORMAL
           "}"
        "QPushButton "
           "{background-color: gray; "
            "color: black; "
            "border: 1px solid black; "
            "border-radius: 4px; "
            "height: 35px; "
            "min-width: 60px; "
            "max-width: 60px; "
            "text-align: center;"
            "} "
#ifdef MUSICALPI_DEBUG_WIDGET_BORDERS
        "[accessibleName=\"generalLayoutWidget\"]    {border:5px solid red;} "
        "[accessibleName=\"mainMenuLayoutWidget\"]   {border:5px solid green;} "
        "[accessibleName=\"menuLayoutWidget\"]       {border:5px solid blue;} "
        "[accessibleName=\"outerLayoutWidget\"]      {border:5px solid darkRed;} "
        "[accessibleName=\"menuLayoutWidget\"]       {border:5px solid darkGreen;} "
        "[accessibleName=\"playerMenuLayoutWidget\"] {border:5px solid darkBlue;} "
        "[accessibleName=\"libraryTable\"]           {border:5px solid cyan;} "
        "[accessibleName=\"settingsLabel\"]          {border:5px solid magenta;} "
        "[accessibleName=\"playerMenuLayoutWidget\"] {border:5px solid yellow;} "
        "[accessibleName=\"logoLabel\"]              {border:5px solid darkCyan;} "
#endif
       ));
    MainWindow w;
    w.show();
    return a.exec();
}
