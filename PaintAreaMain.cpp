#include "PaintAreaMain.h"

#include <QDebug>
#include <QStackedLayout>
#include <QResizeEvent>
#include <QApplication>

PaintAreaMain::PaintAreaMain(QWidget *parent) : PaintArea(parent)
  ,_paintImage(nullptr)
{
    initGrid();
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

void PaintAreaMain::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.save();
    drawGrid(painter);
    drawXYCoordinateText(painter);
    painter.restore();

    PaintArea::paintEvent(event);
}

void PaintAreaMain::wheelEvent(QWheelEvent *event)
{
    PaintArea::wheelEvent(event);

    /*
     * Ctrl + 鼠标滚轮，进行绘图区域的缩放。
     */
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        QPoint numDegrees = event->angleDelta();
        int step = 0;

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
        }
        else
        {
            qDebug() << "Zoom in: " << curWidth << " " << curHeight;
        }

        if ((curWidth > this->maximumWidth()) || (curWidth < this->minimumWidth()))
        {
            return;
        }
        if ((curHeight > this->maximumHeight()) || (curHeight < this->minimumHeight()))
        {
            return;
        }

        resize(curWidth, curHeight);

        if (event->angleDelta().y() > 0)
        {
            _gridGap += 0.1;
        }
        else if (event->angleDelta().y() < 0)
        {
            _gridGap -= 0.1;
        }
        else
        {

        }

        if (_gridGap >= _gridMaxGap)
        {
            _gridGap = _gridMinGap;
        }
        else if (_gridGap <= _gridMinGap)
        {
            _gridGap = _gridMaxGap;
        }

        update();
    }
}

void PaintAreaMain::resizeEvent(QResizeEvent *event)
{
    PaintArea::resizeEvent(event);
    _gridColLineStartX = 0;
    _gridRowLineStartY = 0;
    update();
}

void PaintAreaMain::drawXYCoordinateText(QPainter &painter)
{
    // TODO: 使用控件绑定到鼠标位置...
    QString xy = QString("X:%0 Y:%1").arg(QString::number(this->cursor().pos().x())).arg(QString::number(this->cursor().pos().y()));
    painter.drawText(QPoint(20, this->height() - 30), xy);
}

void PaintAreaMain::initGrid()
{
    _gridMinGap = 20;
    _gridMaxGap = 30;
    _gridGap = (_gridMinGap + _gridMaxGap) / 2;
    _gridColLineStartX = 0;
    _gridRowLineStartY = 0;
    _gridLightPen = QPen(QColor("#f0fff0"));
    _gridLightPen.setStyle(Qt::PenStyle::DotLine);
    _gridLightPen.setWidth(1);
    _gridDarkPen  = QPen(QColor("#f0fff0"));
    _gridDarkPen.setStyle(Qt::PenStyle::DotLine);
    _gridDarkPen.setWidth(2);
}

void PaintAreaMain::drawGrid(QPainter &painter)
{
    int lineCnt = 0;
    qreal bigY = _gridRowLineStartY;
    qreal smallY = _gridRowLineStartY;

    painter.setPen(_gridDarkPen);

    /*
     * 绘制行线条。
     */
    while (true)
    {
        painter.drawLine(QPointF(0.0, bigY), QPointF(this->width(), bigY));
        painter.drawLine(QPointF(0.0, smallY), QPointF(this->width(), smallY));

        bigY += _gridGap;
        smallY += _gridGap;
        if ((smallY <= 0) || (bigY >= this->height()))
        {
            break;
        }

        lineCnt++;
        if (lineCnt == 10)
        {
            painter.setPen(_gridDarkPen);
            lineCnt = 0;
        }
        else
        {
            painter.setPen(_gridLightPen);
        }
    }

    lineCnt = 0;
    qreal bigX = _gridColLineStartX;
    qreal smallX = _gridColLineStartX;
    painter.setPen(_gridDarkPen);

    while (true)
    {
        painter.drawLine(QPointF(bigX, 0.0), QPointF(bigX, this->height()));
        painter.drawLine(QPointF(smallX, 0.0), QPointF(smallX, this->width()));

        bigX += _gridGap;
        smallX += _gridGap;
        if ((smallX <=0 ) || (bigX >= this->width()))
        {
            break;
        }

        lineCnt++;
        if (lineCnt == 10)
        {
            painter.setPen(_gridDarkPen);
            lineCnt = 0;
        }
        else
        {
            painter.setPen(_gridLightPen);
        }
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
