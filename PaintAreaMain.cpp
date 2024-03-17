#include "PaintAreaMain.h"

#include <QDebug>
#include <QStackedLayout>
#include <QResizeEvent>
#include <QApplication>
#include <QFileDialog>
#include <QImage>
#include <QGraphicsPixmapItem>

PaintAreaMain::PaintAreaMain(QGraphicsScene *scene, QWidget *parent) : PaintArea(scene, parent)
  ,_paintImage(nullptr)
{
    this->setBackgroundBrush(QPixmap(":/images/background1.png"));

    _curPosLabel = new QLabel(this);
    _curPosLabel->move(20, this->height() - 80);
    _curPosLabel->setText(QString("X:0 Y:0"));
    _curPosLabel->setMinimumSize(100, 40);
    _curPosLabel->setMaximumSize(300, 40);
//    _curPosLabel->setAutoFillBackground(true);
    _curPosLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);

    QFont font = _curPosLabel->font();
    font.setPointSize(14);
    font.setWeight(QFont::Bold);
    font.setItalic(true);
    font.setUnderline(true);
    _curPosLabel->setFont(font);

    _curPosLabel->setStyleSheet("background-color:transparent;color:red;");
//    QPalette p = _curPosLabel->palette();
//    p.setColor(QPalette::ColorRole::Background, Qt::transparent);
//    p.setColor(QPalette::ColorRole::WindowText, Qt::red);
//    _curPosLabel->setPalette(p);
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
        QString path = QFileDialog::getOpenFileName(nullptr, "选择图片", "", "Images (*.png *.xpm *.jpg);;All Files (*)");
//        _paintImage = new PaintImage(this);
//        _paintImage->setObjectName("PaintImage");
        QGraphicsScene *scene = new QGraphicsScene(this);
        QGraphicsPixmapItem *aa = scene->addPixmap(QPixmap(path));
        _paintImage = new PaintImage(scene, this);
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
    PaintArea::paintEvent(event);
    QPainter painter(this->viewport());
    paintAllShapes(painter);
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
        qreal g = 0;

        if (!numDegrees.isNull())
        {
            step = numDegrees.y();
        }

        event->accept();

        qDebug() << step;

        if (step > 0)
        {
            g = 2;
            qDebug() << "Zoom out: " << g;
        }
        else
        {
            g = 0.5;
            qDebug() << "Zoom in: " << g;
        }

        qDebug() << this->matrix().m11();
        this->scale(g, g);
        qDebug() << this->matrix().m11();
    }
}

void PaintAreaMain::resizeEvent(QResizeEvent *event)
{
    PaintArea::resizeEvent(event);
    _curPosLabel->move(20, this->height() - 80);
}

void PaintAreaMain::mouseMoveEvent(QMouseEvent *e)
{
    PaintArea::mouseMoveEvent(e);
    this->updateXYCoordinateText();
}

void PaintAreaMain::updateXYCoordinateText()
{
    QPoint pos = this->AdjustedPos(this->mapFromGlobal(this->cursor().pos()));
    QString xy = QString("X:%0 Y:%1").arg(QString::number(pos.x())).arg(QString::number(pos.y()));
    _curPosLabel->setText(xy);
    _curPosLabel->adjustSize();
}

PaintAreaMainWrapper::PaintAreaMainWrapper(QWidget *parent) : QWidget(parent)
  ,_openToResizeChild(false)
{
}

void PaintAreaMainWrapper::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

//    if (!_openToResizeChild && !this->children().isEmpty())
    if (!this->children().isEmpty())
    {
        auto p = this->findChild<PaintAreaMain *>("paintAreaMain");

        if (p != nullptr)
        {
            p->resize(event->size());
            _openToResizeChild = true;
        }
    }
}
