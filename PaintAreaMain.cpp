#include "PaintAreaMain.h"

#include <QDebug>
#include <QStackedLayout>
#include <QResizeEvent>

PaintAreaMain::PaintAreaMain(QWidget *parent) : PaintArea(parent)
  ,_paintImage(nullptr)
{
}

void PaintAreaMain::SetPaintType(EPaintType type)
{
    PaintArea::SetPaintType(type);
    if (_paintImage != nullptr)
    {
        _paintImage->SetPaintType(type);
    }
}

void PaintAreaMain::ImageOptChangedHandler(int optMode)
{
    if (_paintImage == nullptr)
    {
        _paintImage = new PaintImage(this);
        _paintImage->setObjectName("PaintImage");
        _paintImage->move(100, 100);
    }

    if (optMode == 1)
    {
        _paintImage->show();
        _paintImage->LoadImage();
    }
    else if (optMode == 2)
    {
        _paintImage->SaveImage();
    }
    else if (optMode == 3)
    {
        _paintImage->ClearImage();
    }
}

PaintAreaMainWrapper::PaintAreaMainWrapper(QWidget *parent) : QWidget(parent)
  ,_openToResizeChild(false)
{
}

void PaintAreaMainWrapper::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (!_openToResizeChild && !this->children().isEmpty())
    {
        auto p = this->findChild<PaintAreaMain *>("paintAreaMain");
        p->resize(event->size());
        _openToResizeChild = true;
    }
}
