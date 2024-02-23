#ifndef PAINTTOOLBAR_H
#define PAINTTOOLBAR_H

#include <QWidget>
#include <QGroupBox>
#include <QCheckBox>

#include "Types.h"

class CheckBoxWithPaintType : public QCheckBox
{
    Q_OBJECT
public:
    CheckBoxWithPaintType(EPaintType type, QWidget *parent = nullptr) : QCheckBox(parent)
      , _paintType(type)
    {
        connect(this, &QCheckBox::stateChanged, this, [this](int state)
        {
            if (state == Qt::CheckState::Checked)
            {
                emit paintTypeChanged(_paintType);
            }
        });
    }

signals:
    void paintTypeChanged(EPaintType type);

private:
    EPaintType _paintType;
};

class PaintToolBar : public QGroupBox
{
    Q_OBJECT
public:
    explicit PaintToolBar(QWidget *parent = nullptr);

signals:
    void paintTypeChanged(EPaintType type);
    void imageOptChanged(int optMode);

private:
    void addCheckBox(EPaintType type);
};

#endif // PAINTTOOLBAR_H
