#include "mainwindow.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QPen>
#include <QPoint>
#include <QList>
#include <QVBoxLayout>

#include "PaintPanel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
//    QSizePolicy sp = this->sizePolicy();
//    sp.setVerticalPolicy(QSizePolicy::Policy::Maximum);
//    sp.setHorizontalPolicy(QSizePolicy::Policy::Maximum);
//    this->setSizePolicy(sp);
//    this->setWindowState(Qt::WindowState::WindowMaximized);
    this->setMinimumSize(this->size() * 2);

    PaintPanel *paintPanel = new PaintPanel(this);
    this->setCentralWidget(paintPanel);
    qDebug() << this->width() << "  " << this->height();
    qDebug() << this->centralWidget()->width() << "  " << this->centralWidget()->height();
}

MainWindow::~MainWindow()
{
}
