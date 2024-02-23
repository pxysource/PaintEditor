#include "PaintImage.h"

#include <QFileDialog>

PaintImage::PaintImage(QWidget *parent) : PaintArea(parent), _saveEnabled(false)
{
    _image = new QImage();
}

PaintImage::~PaintImage()
{
    delete _image;
}

void PaintImage::LoadImage()
{
    QString path = QFileDialog::getOpenFileName(nullptr, "选择图片", "", "Images (*.png *.xpm *.jpg);;All Files (*)");
    _image->load(path);
    this->resize(_image->width(), _image->height());
    update();
}

void PaintImage::SaveImage()
{
    _saveEnabled = true;
    update();
    QString path = QFileDialog::getSaveFileName(nullptr, "保存图片", "", "Images (*.png *.xpm *.jpg);;All Files (*)");
    _image->save(path);
}

void PaintImage::ClearImage()
{
    close();
}

void PaintImage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    QRectF sourceRect(0.0, 0.0, this->width(), this->height());
    QRectF targetRect(0.0, 0.0, this->width(), this->height());
    painter.drawImage(targetRect, *_image, sourceRect);

    this->paintShapes();
}

void PaintImage::paintShapes()
{
    if (_saveEnabled)
    {
        QPainter painter(_image);
        paintAllShapes(painter);
    }
    else
    {
        QPainter painter(this);
        paintAllShapes(painter);
    }
}
