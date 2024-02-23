#ifndef PAINTAREAMAIN_H
#define PAINTAREAMAIN_H

#include <QWidget>
#include "PaintArea.h"
#include "PaintImage.h"

class PaintAreaMain : public PaintArea
{
    Q_OBJECT
public:
    explicit PaintAreaMain(QWidget *parent = nullptr);
    void SetPaintType(EPaintType type) override;

public slots:
    void ImageOptChangedHandler(int optMode);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    PaintImage *_paintImage;

    int _gridMinGap;
    int _gridMaxGap;
    qreal _gridGap;
    qreal _gridColLineStartX;
    qreal _gridRowLineStartY;
    QPen _gridLightPen;
    QPen _gridDarkPen;

    void drawXYCoordinateText(QPainter &painter);
    void initGrid();
    void drawGrid(QPainter &painter);
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
