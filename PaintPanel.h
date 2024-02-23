#ifndef PAINTPANEL_H
#define PAINTPANEL_H

#include <QWidget>

#include "PaintAreaMain.h"
#include "PaintToolbar.h"

class PaintPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PaintPanel(QWidget *parent = nullptr);

private:
    PaintAreaMain *_paintAreaMain;
    PaintAreaMainWrapper *_paintAreaMainWrapper;
    PaintToolBar *_paintToolBar;
};

#endif // PAINTPANEL_H
