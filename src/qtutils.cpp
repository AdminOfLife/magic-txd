#pragma once

#include "mainwindow.h"
#include "qtutils.h"

void CalculateWindowSize(QWidget *widget, unsigned int baseWidth, unsigned int minBaseWidth, unsigned int maxBaseWidth, unsigned int minHeight, unsigned int baseHeight) {
    unsigned int minwidth = baseWidth;

    if (minwidth > maxBaseWidth)
        minwidth = maxBaseWidth;

    widget->setMinimumSize(minwidth, minHeight);

    unsigned int new_w, new_h;
    if (minwidth < minBaseWidth)
        new_w = minBaseWidth;
    else
        new_w = minwidth;

    if (widget->width() > new_w)
        new_w = widget->width();

    new_h = baseHeight;

    if (widget->height() > new_h)
        new_h = widget->height();

    widget->resize(new_w, new_h);
}

QHBoxLayout *CreateButtonsLayout(QWidget *Parent) {
    QHBoxLayout *buttonLayout = new QHBoxLayout(Parent);
    buttonLayout->setContentsMargins(QMargins(12, 12, 12, 12));
    buttonLayout->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    buttonLayout->setSpacing(10);
    return buttonLayout;
}

QVBoxLayout *CreateRootLayout(QWidget *Parent) {
    QVBoxLayout *rootLayout = new QVBoxLayout(Parent);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);
    rootLayout->setSizeConstraint(QLayout::SetFixedSize);
    return rootLayout;
}

QHBoxLayout *CreateTopLevelHBoxLayout(QWidget *Parent) {
    QHBoxLayout *topLevelLayout = new QHBoxLayout(Parent);
    topLevelLayout->setContentsMargins(QMargins(12, 12, 12, 12));
    topLevelLayout->setSpacing(10);
    return topLevelLayout;
}

QVBoxLayout *CreateTopLevelVBoxLayout(QWidget *Parent) {
    QVBoxLayout *topLevelLayout = new QVBoxLayout(Parent);
    topLevelLayout->setContentsMargins(QMargins(12, 12, 12, 12));
    topLevelLayout->setSpacing(10);
    return topLevelLayout;
}

QPushButton *CreateButton(QString Text, QWidget *Parent) {
    QPushButton *button = new QPushButton(Text, Parent);
    button->setMinimumWidth(90);
    return button;
}

QWidget *CreateLine(QWidget *Parent) {
    QWidget *line = new QWidget(Parent);
    line->setFixedHeight(1);
    line->setObjectName("hLineBackground");
    return line;
}