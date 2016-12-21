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
            "}"
       ));
    MainWindow w;
    w.show();
    return a.exec();
}
