#ifndef PAINTAREAMAIN_H
#define PAINTAREAMAIN_H

#include <QWidget>
#include <QLabel>
#include "PaintArea.h"
#include "PaintImage.h"

class PaintAreaMain : public PaintArea
{
    Q_OBJECT
public:
//    explicit PaintAreaMain(QWidget *parent = nullptr);
    PaintAreaMain(QGraphicsScene *scene, QWidget *parent = nullptr);
    void SetPaintType(EPaintType type) override;

public slots:
    void ImageOptChangedHandler(int optMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    PaintImage *_paintImage;
    QLabel *_curPosLabel;
    void updateXYCoordinateText();
};

class PaintAreaMainWrapper : public QWidget
{
    Q_OBJECT
public:
    explicit PaintAreaMainWrapper(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    bool _openToResizeChild;
};

#endif // PAINTAREAMAIN_H
