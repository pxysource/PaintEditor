#include "PaintToolbar.h"

#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPushButton>

PaintToolBar::PaintToolBar(QWidget *parent) : QGroupBox(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    this->setLayout(vLayout);

    addCheckBox(EPaintType::EPT_Point);

    addCheckBox(EPaintType::EPT_Line);
    addCheckBox(EPaintType::EPT_Arc);
    addCheckBox(EPaintType::EPT_Polyline);

    addCheckBox(EPaintType::EPT_Circle);
    addCheckBox(EPaintType::EPT_Rect);
    addCheckBox(EPaintType::EPT_Ellipse);
    addCheckBox(EPaintType::EPT_Polygon);

    QPushButton *btn = new QPushButton();
    btn->setText("加载图片");
    connect(btn, &QPushButton::clicked, this, [this](){
        emit imageOptChanged(1);
    });
    this->layout()->addWidget(btn);
    btn = new QPushButton();
    btn->setText("保存图片");
    connect(btn, &QPushButton::clicked, this, [this](){
        emit imageOptChanged(2);
    });
    this->layout()->addWidget(btn);
    btn = new QPushButton();
    btn->setText("清除图片");
    connect(btn, &QPushButton::clicked, this, [this](){
        emit imageOptChanged(3);
    });
    this->layout()->addWidget(btn);

    QSpacerItem *si = new QSpacerItem(10,10, QSizePolicy::Fixed, QSizePolicy::Expanding);
    vLayout->addSpacerItem(si);
}

void PaintToolBar::addCheckBox(EPaintType type)
{
    CheckBoxWithPaintType *checkBox = new CheckBoxWithPaintType(type);
    checkBox->setText(PaintTypeStrings::GetStringZh(type));
    checkBox->setAutoExclusive(true);
    checkBox->setCheckable(true);
    checkBox->setChecked(false);

    this->layout()->addWidget(checkBox);
    connect(checkBox, SIGNAL(paintTypeChanged(EPaintType)), this, SIGNAL(paintTypeChanged(EPaintType)));
}
