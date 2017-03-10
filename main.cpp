// Copyright 2016 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QApplication>
#include <debugmessages.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    a.setStyleSheet(QString(
        "QWidget                       {                background-color:" MUSICALPI_BACKGROUND_COLOR_NORMAL "; margin: 0px; padding: 0px; }"
        "QPushButton                   {color: black;   background-color: gray                                ; font-size: 16px ; border: 1px solid black; border-radius: 4px; height: 35px; padding: 5px; min-width: 60px;  text-align: center;} "
        "QComboBox                     {color: black;    font-size: 16px ; } "
        "QListView                     {color: blue;    font-size: 16px ; } "
        "QLineEdit                     {color: black;    font-size: 20px ; } "
        "QLabel                        {color: black;    font-size: 16px ; } "
        "QLabel[ErrorMessage=\"true\"] {color: darkred;                                                         font-size: 20px ; } "
        "QLabel[Heading=\"true\"]      {color: blue;                                                            font-size: 30px ; } "
        "QLabel[SubHeading=\"true\"]   {color: green;                                                           font-size: 20px ; } "
        "QSlider                       {background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 white, stop:1 Grey); } "

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
        "#containingWidget       {background-color: red; } "
        "#midiPort               {background-color: yellow; } "  // example for item on settings page, done by name
#endif
       ));
    MainWindow w;
    w.show();
    return a.exec();
}
