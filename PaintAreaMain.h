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

private:
    PaintImage *_paintImage;
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
