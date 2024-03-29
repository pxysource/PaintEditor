#include "PaintPanel.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QDebug>
#include <QSizePolicy>
#include <QScrollArea>
#include <QGraphicsScene>

PaintPanel::PaintPanel(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *hLayout = new QHBoxLayout();
    this->setLayout(hLayout);

    _paintAreaMainWrapper = new PaintAreaMainWrapper();
    _paintAreaMainWrapper->setObjectName("paintAreaMainWrapper");
    _paintAreaMainWrapper->setAutoFillBackground(true);
    QPalette p1 = _paintAreaMainWrapper->palette();
    p1.setColor(QPalette::Background, Qt::black);
    _paintAreaMainWrapper->setPalette(p1);

    QGraphicsScene *scene1 = new QGraphicsScene(_paintAreaMainWrapper);
    _paintAreaMain = new PaintAreaMain(scene1, _paintAreaMainWrapper);
    _paintAreaMain->setObjectName("paintAreaMain");
//    _paintAreaMain->setBackgroundBrush(QColor("#778b88"));
    _paintAreaMain->setMinimumSize(1000, 800);
    _paintAreaMain->setMaximumSize(4000, 3200);

    _paintToolBar = new PaintToolBar(this);
    _paintToolBar->setMinimumWidth(100);
    _paintToolBar->setMaximumWidth(180);

    this->layout()->addWidget(_paintToolBar);
    this->layout()->addWidget(_paintAreaMainWrapper);

    connect(_paintToolBar, &PaintToolBar::paintTypeChanged, this, [this](EPaintType type)
    {
        _paintAreaMain->SetPaintType(type);
    });
    connect(_paintToolBar, &PaintToolBar::imageOptChanged,
            _paintAreaMain, &PaintAreaMain::ImageOptChangedHandler);
}
