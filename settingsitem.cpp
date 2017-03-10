// Copyright 2017 by LE Ferguson, LLC, licensed under Apache 2.0

#include <QWidget>
#include <QStyleOption>
#include <QPainter>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "settingsitem.h"
#include "piconstants.h"
#include "mainwindow.h"
#include "settingswidget.h"
#include "focuswatcher.h"
#include "oursettings.h"

#include <cassert>

// This provides individual widgets composed of a prompt, a value entry portion, and a message for validation messages

/* int */     settingsItem::settingsItem(settingsWidget *sw, QWidget *parent, QString key, QString prompt, int lower, int higher) : QWidget(parent)
{
    valueWidget = new QLineEdit(this);
    valueWidget->setMinimumWidth(MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH);
    valueWidget->setMaximumWidth(MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH);
    thisType = int_t;
    settingsItemShared(sw, key, prompt);
    ((QLineEdit*)valueWidget)->setText(mParent->ourSettingsPtr->getSetting(key).toString());
    connect(new FocusWatcher(valueWidget), &FocusWatcher::focusChanged, this, [this,lower, higher](){validateRange(lower,higher);});
}

/* uint */    settingsItem::settingsItem(settingsWidget *sw, QWidget *parent, QString key, QString prompt, unsigned int lower, unsigned int higher) : QWidget(parent)
{
    valueWidget = new QLineEdit(this);
    valueWidget->setMaximumWidth(MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH);
    valueWidget->setMaximumWidth(MUSICALPI_SETTINGS_NUMBERS_MIN_WIDTH);
    thisType = uint_t;
    settingsItemShared(sw, key, prompt);
    ((QLineEdit*)valueWidget)->setText(mParent->ourSettingsPtr->getSetting(key).toString());
    connect(new FocusWatcher(valueWidget), &FocusWatcher::focusChanged, this, [this, lower, higher](){validateRange(lower,higher);});
}
/* string */  settingsItem::settingsItem(settingsWidget *sw, QWidget *parent, QString key, QString prompt, int length) : QWidget(parent)
{
    valueWidget = new QLineEdit(this);
    valueWidget->setMaximumWidth(length);
    valueWidget->setMinimumWidth(length);
    thisType = string_t;
    settingsItemShared(sw, key, prompt);
    ((QLineEdit*)valueWidget)->setText(mParent->ourSettingsPtr->getSetting(key).toString());
}
/* bool */    settingsItem::settingsItem(settingsWidget *sw, QWidget *parent, QString key, QString prompt) : QWidget(parent)
{
    valueWidget = new QCheckBox(this);
    thisType = bool_t;
    settingsItemShared(sw, key, prompt);
    ((QCheckBox*)valueWidget)->setChecked(mParent->ourSettingsPtr->getSetting(key).toBool());
}

/* shared */ void settingsItem::settingsItemShared(settingsWidget *sw, QString key, QString prompt)
{
    ourParent = sw;   // This will be the settings
    mParent = sw->mParent;  // we need a main window pointer
    sw->values[key] = this;  // Self-register in the map
    sw->innerLayout->addWidget(this); // and putourselves in the layout

    hb = new QHBoxLayout(this);
    hb->setAlignment(Qt::AlignBottom);

    m = new QLabel(this);  // starts blank
    m->setMinimumWidth(MUSICALPI_SETTINGS_MSG_MIN_WIDTH);
    p = new QLabel(prompt,this);
    p->setAlignment(Qt::AlignRight | Qt::AlignCenter);
    hb->addWidget(p);
    hb->addWidget(valueWidget);
    hb->addWidget(m);
    this->setObjectName(key);
    m->setProperty("ErrorMessage",true);

    hb->setContentsMargins(0,0,0,0);
}

settingsItem::~settingsItem()
{
}

int settingsItem::toInt()
{
    assert(thisType == int_t || thisType == uint_t );
    return ((QLineEdit*)valueWidget)->text().toInt();
}
unsigned int settingsItem::toUInt()
{
    assert(thisType == uint_t );
    return ((QLineEdit*)valueWidget)->text().toInt();
}
QString settingsItem::toString()
{
    if(thisType == bool_t) return (((QCheckBox*)valueWidget)->checkState() ? "true" : "false");
    else return ((QLineEdit*)valueWidget)->text();
}
bool settingsItem::toBool()
{
    assert(thisType == bool_t);
    return ((QCheckBox*)valueWidget)->checkState();
}

QLabel* settingsItem::msgPtr()
{
    return m;
}
int settingsItem::getPromptWidth()
{
    return p->fontMetrics().boundingRect(p->text()).width();
}

void settingsItem::setPromptWidth(int w)
{
    p->setMinimumWidth(w);
    p->setMaximumWidth(w);
}

bool settingsItem::validateRange(int bottom, int top)
{
    // Validate int (or uint) field, that it is a number and is in range [bottom,top], giving message in label if not (clearing if so)
    bool ok;
    int newVal = ((QLineEdit*)valueWidget)->text().toInt(&ok);
    if(!ok)
    {
        m->setText("Invalid integer number format");
        valueWidget->setFocus();
        return false;
    }
    if(newVal < bottom || newVal > top)
    {
        m->setText("Value must be between " + QString::number(bottom) + " and " + QString::number(top));
        valueWidget->setFocus();
        return false;
    }
    m->setText("");
    return true; // Note on success we aren't affecting focus
}
void settingsItem::paintEvent(QPaintEvent *)  // This is here so we can use stylesheet styling if needed
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
