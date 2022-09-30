#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_BigFileHardLink.h"

class BigFileHardLink : public QMainWindow
{
    Q_OBJECT

public:
    BigFileHardLink(QWidget *parent = Q_NULLPTR);

private:
    Ui::BigFileHardLinkClass ui;
};
