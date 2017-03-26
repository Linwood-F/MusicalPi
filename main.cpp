// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QApplication>
#include <debugmessages.h>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    a.setStyleSheet(QString(
        "QWidget                         {                background-color:" MUSICALPI_BACKGROUND_COLOR_NORMAL "; margin: 0px; padding: 0px; } "
        "QPushButton                     {color: black;   background-color: gray; font-size: 16px ; border: 1px solid black; border-radius: 4px; height: 35px; padding: 5px; min-width: 60px; text-align: center;} "
        "QComboBox                       {color: black;   font-size: 20px; } "
        "QListView                       {color: blue;    font-size: 20px; } "
        "QLineEdit                       {color: black;   font-size: 20px; } "
        "QLineEdit[Number=\"true\"]      {color: black;   font-size: 20px; min-width: 90px; max-width: 90px; } "
        "QLineEdit[Path=\"true\"]        {color: black;   font-size: 20px; min-width:500px; max-width:500px; } "
        "QLineEdit[SettingMsg=\"true\"]  {color: black;   font-size: 20px; min-width: 90px; max-width: 90px; } "
        "QLineEdit[SettingStr=\"true\"]  {color: black;   font-size: 20px; min-width:200px; max-width:200px; } "
        "QLabel                          {color: black;   font-size: 16px; } "
        "QLabel[ErrorMessage=\"true\"]   {color: darkred; font-size: 20px; } "
        "QLabel[Heading=\"true\"]        {color: blue;    font-size: 30px; } "
        "QLabel[SubHeading=\"true\"]     {color: green;   font-size: 20px; } "
        "QSlider                         {min-height: 50px; } "
        "QSlider::groove:horizontal      {border: 2px solid black; height: 10px; }  "
        "QSlider::handle:horizontal      {background: darkGrey; border: 3px solid black; width: 40px; margin-top: -15px; margin-bottom: -15px; border-radius: 6px; } "
        "QSlider:sub-page:horizontal     {background: darkGreen; } "
        "QSlider:add-page:horizontal     {background: lightGrey; } "
        "QSlider:sub-page:horizontal:disabled {background: lightGreen; } "
        "QSlider:add-page:horizontal:disabled {background: lightGrey; } "
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
