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
        "#settingsUI             {background-color: magenta; } "
        "#generalLayoutWidget    {background-color: red; } "
        "#mainMenuLayoutWidget   {background-color: green; } "
        "#menuLayoutWidget       {background-color: blue; } "
        "#outerLayoutWidget      {background-color: darkRed; } "
        "#menuLayoutWidget       {background-color: darkGreen; } "
        "#playerMenuLayoutWidget {background-color: darkBlue; } "
        "#libraryTable           {background-color: cyan; } "
        "#logoLabel              {background-color: darkCyan; } "
#endif
       ));
    MainWindow w;
    w.show();
    return a.exec();
}
