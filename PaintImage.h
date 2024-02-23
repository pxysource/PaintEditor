#ifndef PAINTIMAGE_H
#define PAINTIMAGE_H

#include "PaintArea.h"
#include <QImage>

class PaintImage : public PaintArea
{
public:
    explicit PaintImage(QWidget *parent = nullptr);
    ~PaintImage();
    void LoadImage();
    void SaveImage();
    void ClearImage();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QImage *_image;
    bool _saveEnabled;

    void paintShapes();
};

#endif // PAINTIMAGE_H
