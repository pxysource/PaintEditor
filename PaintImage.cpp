#include "PaintImage.h"

#include <QFileDialog>
#include <QApplication>
#include <QWheelEvent>
#include <QDebug>

PaintImage::PaintImage(QGraphicsScene *scene, QWidget *parent) : PaintArea(scene, parent), _saveEnabled(false)
{
    _image = new QImage();
}

PaintImage::~PaintImage()
{
    delete _image;
}

void PaintImage::LoadImage()
{
//    QString path = QFileDialog::getOpenFileName(nullptr, "选择图片", "", "Images (*.png *.xpm *.jpg);;All Files (*)");
//    _image->load(path);
//    this->resize(_image->width(), _image->height());
    this->viewport()->update();
}

void PaintImage::SaveImage()
{
    _saveEnabled = true;
    this->viewport()->update();
    QString path = QFileDialog::getSaveFileName(nullptr, "保存图片", "", "Images (*.png *.xpm *.jpg);;All Files (*)");
    _image->save(path);
}

void PaintImage::ClearImage()
{
    close();
}

void PaintImage::paintEvent(QPaintEvent *event)
{
    PaintArea::paintEvent(event);

//    QPainter painter(this->viewport());
//    QRectF sourceRect(0.0, 0.0, this->width(), this->height());
//    QRectF targetRect(0.0, 0.0, this->width(), this->height());
//    painter.drawImage(targetRect, *_image, sourceRect);

//    this->paintShapes();
}

void PaintImage::wheelEvent(QWheelEvent *event)
{
    PaintArea::wheelEvent(event);

    /*
     * Ctrl + 鼠标滚轮，进行绘图区域的缩放。
     */
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        QPoint numDegrees = event->angleDelta();
        int step = 0;
        qreal g = 0;

        if (!numDegrees.isNull())
        {
            step = numDegrees.y();
        }

        event->accept();

        int curWidth = this->width();
        int curHeight = this->height();
        curWidth += step;
        curHeight += step;
        qDebug() << step;

        if (step > 0)
        {
            qDebug() << "Zoom out: " << curWidth << " " << curHeight;
            g = 2;
        }
        else
        {
            qDebug() << "Zoom in: " << curWidth << " " << curHeight;
            g = 0.5;
        }

        if ((curWidth > this->maximumWidth()) || (curWidth < this->minimumWidth()))
        {
            return;
        }
        if ((curHeight > this->maximumHeight()) || (curHeight < this->minimumHeight()))
        {
            return;
        }

        qDebug() << this->matrix().m11();
        this->scale(g, g);
        qDebug() << this->matrix().m11();
        this->viewport()->update();

//        *_image = _image->scaled(curWidth, curHeight);
//        qDebug() << _image->size();
//        resize(curWidth, curHeight);
    }
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
        QPainter painter(this->viewport());
        paintAllShapes(painter);
    }
}
