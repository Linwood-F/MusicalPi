#ifndef MIDIPLAYER2_H
#define MIDIPLAYER2_H

// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0


#include <QObject>
#include <QWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QCloseEvent>

#include "piconstants.h"

#include "MidiFile.h"
#include "MidiEvent.h"
#include "MidiEventList.h"
#include "MidiMessage.h"


class midiPlayerV2 : public QWidget
{
    Q_OBJECT
public:
    midiPlayerV2(QWidget *parent, QString midiFile, QString titleName);
    ~midiPlayerV2();

    int lastBar;


private:
    QVBoxLayout *outLayout;
    QGridLayout *gridLayout;
    QLabel      *helpLabel;
    QLabel      *measureInLabel;
    QLineEdit   *measureIn;
    QLabel      *measureNowAt;
    QPushButton *measureGo;
    QSlider     *tempoSlider;
    QSlider     *positionSlider;
    QLabel      *tempoLabel, *tempoValueLabel;
    QLabel      *positionLabel, *positionValueLabel;
    QSlider     *volumeSlider;
    QLabel      *volumeLabel, *volumeValueLabel;
    QLabel      *songLabel;
    QLabel      *errorLabel;
    QString     midiFile;     // Passed in file

    void doPlayingLayout();
    void openAndLoadFile();
    void go();

    bool canPlay;   // Set to indicate if the song is loaded and playable
    QString errorEncountered;  // Blank is no error, otherwise a fatal error
    void closeEvent(QCloseEvent*);

private slots:
    void updateSliders();
    void updateVolume(int);
    void updateTempo(int);

signals:
    void requestToClose();
};

#endif // MIDIPLAYER2_H
