#include "PaintArea.h"

#include <algorithm>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QPen>
#include <QPoint>
#include <QList>
#include <QApplication>
#include <QStackedLayout>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDropEvent>
#include <QDrag>
#include <QGraphicsView>
#include <QGraphicsScene>

PaintArea::PaintArea(QGraphicsScene *scene, QWidget *parent) : QGraphicsView(scene, parent)
  , _paintType(EPaintType::EPT_None)
  , _lastPaintShape(nullptr)
  , _lastSelectedShape(nullptr)
  , _mouseButtonPressEnabled(true)
  , _mouseButtonReleaseEnabled(true)
  , _mouseMoveEnabled(true)
  , _moveEnabled(false)
  , _dragResizeEnabled(false)
{
    for (int i = EPaintType::EPT_Point; i < EPaintType::EPT_End; i++)
    {
        _coreMap.insert(static_cast<EPaintType>(i), new QList<GeometryShape *>());
    }

    _selectedList.clear();

    // 开启追踪鼠标，可触发mouseMoveEvent事件
    setMouseTracking(true);
    // 光标：+
    setCursor(Qt::CrossCursor);
    // 可触发keyPressEvent事件
    setFocusPolicy(Qt::StrongFocus);
    QWidget::installEventFilter(this);

    // 隐藏水平滚动条
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    // 隐藏垂直滚动条
    this->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
}

PaintArea::~PaintArea()
{
    for (auto list : _coreMap.values())
    {
        for (auto item : *list)
        {
            if (item != nullptr)
            {
                delete item;
            }
        }

        delete list;
    }
}

bool PaintArea::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)
    bool ret = false;

    switch (event->type())
    {
    case QEvent::MouseButtonPress:
        ret = !_mouseButtonPressEnabled;
        break;
    case QEvent::MouseButtonRelease:
        ret = !_mouseButtonReleaseEnabled;
        break;
    case QEvent::MouseMove:
        ret = !_mouseMoveEnabled;
        break;
    case QEvent::KeyPress:
    {
        QKeyEvent *keyEvent = reinterpret_cast<QKeyEvent *>(event);
        if (((keyEvent->key() == Qt::Key_A) && ((keyEvent->modifiers() & Qt::ControlModifier) == Qt::ControlModifier)) ||
                (keyEvent->key() == Qt::Key_Delete))
        {
            ret = false;
        }
        else
        {
            ret = true;
        }
    }
        break;
    default:
        break;
    }

    return ret;
}

void PaintArea::SetPaintType(EPaintType type)
{
    if (!_coreMap.contains(type))
    {
        qCritical() << "Error: SetPaintType(), Invalid paint type! " << type;
//        _mouseMoveEnabled = false;
        return;
    }

    _mouseMoveEnabled = true;
    _paintType = type;
    qDebug() << "type" << type;
}

void PaintArea::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
//    QPainter painter(this);
//    paintAllShapes(painter);
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    EPaintType paintType = _paintType;
    GeometryShape *shape = nullptr;
    QPoint eventPos = this->AdjustedPos(event->pos());

    if ((event->button() == Qt::MouseButton::LeftButton) &&
            ((QApplication::keyboardModifiers() == Qt::AltModifier)))
    {
        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            this->setCursor(Qt::CursorShape::SizeAllCursor);
            _moveStartCursorPos = eventPos;
            return;
        }
    }

    if (!_coreMap.contains(paintType))
    {
        qCritical() << "Error: mousePressEvent(), invalid paint type!";
        return;
    }

    switch (event->button())
    {
    case Qt::MouseButton::LeftButton:
        qDebug() << this->objectName() << ": Left button press." << eventPos;

        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            if (QApplication::keyboardModifiers() == Qt::ControlModifier)
            {
                qDebug() << "Key: Ctrl + LeftButton, press";
                this->multiSelectHandler(eventPos);
                break;
            }

            this->singleSelectPressHandler(eventPos);
            if (_selectedList.count() > 0)
            {
                break;
            }
        }

        if (_coreMap[paintType]->isEmpty() || _coreMap[paintType]->last()->GetCompleted())
        {
            shape = GeometryShapeFactory::CreateGeometryShape(paintType);
            _coreMap[paintType]->append(shape);
        }

        _lastPaintShape = _coreMap[paintType]->last();
        _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_Painting, eventPos);

        if (_lastPaintShape->GetCompleted())
        {
            this->viewport()->update();
        }

        break;
    case Qt::MouseButton::RightButton:
        switch (paintType) {
        case EPaintType::EPT_Polygon:
        case EPaintType::EPT_Polyline:
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_PaintEnd, QPoint());
            _lastPaintShape = nullptr;
            this->viewport()->update();
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void PaintArea::mouseReleaseEvent(QMouseEvent *event)
{
    EPaintType paintType = _paintType;
    QPoint eventPos = this->AdjustedPos(event->pos());

    if ((event->button() == Qt::MouseButton::LeftButton) &&
            ((QApplication::keyboardModifiers() == Qt::AltModifier)))
    {
        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            this->setCursor(DefaultCursorShape);
            return;
        }
    }

    if (!_coreMap.contains(paintType))
    {
        //qCritical() << "Error: mouseReleaseEvent(), Invalid paint type!";
        return;
    }

    switch (event->button()) {
    case Qt::MouseButton::LeftButton:
        qDebug() << this->objectName() << ": Left button release.";

        if (QApplication::keyboardModifiers() == Qt::ControlModifier)
        {
            qDebug() << "Key: Ctrl + LeftButton, release";
            break;
        }

        /*
         * 拖拽改变图形的大小，完成。
         */
        if (_dragResizeEnabled)
        {
            _dragResizeEnabled = false;
            _selectedList.last()->SetDragResizeEnabled(false);
            this->viewport()->update();
            break;
        }

        /*
         * 移动图形，完成。
         */
        if (_moveEnabled)
        {
            for (auto item : _selectedList)
            {
                item->MoveEnd(eventPos);
            }

            _moveEnabled = false;
            this->viewport()->update();
            break;
        }

        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            if (singleSelectReleaseHandler(eventPos))
            {
                break;
            }
        }

        switch (paintType)
        {
        case EPaintType::EPT_Line:
        case EPaintType::EPT_Circle:
        case EPaintType::EPT_Rect:
        case EPaintType::EPT_Ellipse:
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_PaintEnd, eventPos);
            if (_lastPaintShape->GetCompleted() && !_lastPaintShape->IsValid())
            {
                qWarning() << "Warn: Invalid shape! " << paintType;
                _coreMap[paintType]->removeOne(_lastPaintShape);
                delete _lastPaintShape;
            }

            _lastPaintShape = nullptr;
            this->viewport()->update();
            break;
        default:
            break;
        }

        break;
    case Qt::MouseButton::RightButton:
        break;
    default:
        break;
    }
}

void PaintArea::mouseMoveEvent(QMouseEvent *e)
{
    EPaintType paintType = _paintType;
    QPoint eventPos = this->AdjustedPos(e->pos());

    if ((_lastPaintShape == nullptr) || _lastPaintShape->GetCompleted())
    {
        if (((e->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) &&
                (QApplication::keyboardModifiers() == Qt::AltModifier))
        {
            this->setCursor(Qt::CursorShape::SizeAllCursor);
            this->move(this->pos() + (eventPos - _moveStartCursorPos));
            return;
        }
    }

    if (!_coreMap.contains(paintType))
    {
        //qDebug() << "Error: mouseMoveEvent(), Invalid paint type!";
        return;
    }

    cursorShapeHandler(eventPos);

    /*
     * Ctrl + 鼠标移动，忽略...
     */
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        return;
    }

    /*
     * 拖拽图形改变其大小，进行中...
     */
    if (_dragResizeEnabled)
    {
        _selectedList.last()->DragResize(eventPos);
        this->viewport()->update();
        return;
    }

    /*
     * 移动图形，进行中...
     */
    if (_moveEnabled)
    {
        for (auto item : _selectedList)
        {
            item->Move(eventPos);
        }

        this->viewport()->update();
        return;
    }

    switch (paintType)
    {
    case EPaintType::EPT_Line:
    case EPaintType::EPT_Arc:
    case EPaintType::EPT_Circle:
    case EPaintType::EPT_Rect:
    case EPaintType::EPT_Ellipse:
    case EPaintType::EPT_Polygon:
    case EPaintType::EPT_Polyline:
        if ((_lastPaintShape != nullptr) && !_lastPaintShape->GetCompleted())
        {
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_GuidePaintting, eventPos);
            this->viewport()->update();
        }
        break;
    default:
        break;
    }
}

void PaintArea::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_A))
    {
        //qDebug() << "Key: Ctrl + A";
        selectAllShapes();
    }
    if ((event->key() == Qt::Key_Delete))
    {
        //qDebug() << "Key: Del";
        deleteSelectedShapes();
    }
}

void PaintArea::wheelEvent(QWheelEvent *event)
{
    QWidget::wheelEvent(event);

    /*
     * Ctrl + 鼠标滚轮，进行绘图区域的缩放。
     */
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
//        qDebug() << this->cursor().pos();
//        qDebug() << this->pos();
        QPoint numDegrees = event->angleDelta();
        int step = 0;

        if (!numDegrees.isNull())
        {
            step = numDegrees.y();
        }

        event->accept();
    }
}

void PaintArea::paintAllShapes(QPainter& painter)
{
    painter.save();
    //painter.rotate(60);
    painter.scale(this->transform().m11(), this->transform().m22());

    for (auto list : _coreMap.values())
    {
        if (!list->isEmpty())
        {
            for (auto item : *list)
            {
                if (item != nullptr)
                {
                    item->Paint(painter);
                }
            }
        }
    }

    painter.restore();
}

void PaintArea::paintCursorLine()
{
    // 绘制鼠标，效果太差，拖动有阴影。
//    QPainter painter(this);
//    painter.setPen(_guideLinePen);
//    painter.drawLine(_curCursorPoint.x() - 20, _curCursorPoint.y(), _curCursorPoint.x() + 20, _curCursorPoint.y());
    //    painter.drawLine(_curCursorPoint.x(), _curCursorPoint.y() - 20, _curCursorPoint.x(), _curCursorPoint.y() + 20);
}

int PaintArea::edgeCheck(bool stateCache, bool state)
{
    if (!stateCache && state)
    {
        return 1;
    }
    else if (stateCache && !state)
    {
        return -1;
    }

    return 0;
}

void PaintArea::multiSelectHandler(const QPoint &point)
{
    for (auto list : _coreMap.values())
    {
        if (!list->isEmpty())
        {
            auto it = std::find_if(list->begin(), list->end(), [=](GeometryShape *rect){
                return rect->Contains(point);
            });

            if (it != list->end())
            {
                if (_selectedList.contains(*it))
                {
                    (*it)->SetSelected(false);
                    _selectedList.removeOne(*it);
                }
                else
                {
                    (*it)->SetSelected(true);
                    _selectedList.append(*it);
                }

                break;
            }
        }
    }
}

void PaintArea::singleSelectPressHandler(const QPoint &point)
{
    GeometryShape *shape = nullptr;

    for (auto list : _coreMap.values())
    {
        if ((list != nullptr) && !list->isEmpty())
        {
            auto it = std::find_if(list->begin(), list->end(), [=](GeometryShape *rect){
                return rect->Contains(point);
            });

            if (it != list->end())
            {
                shape = *it;
                break;
            }
        }
    }

    if (shape != nullptr)
    {
        /*
         * 上一次选择为多选，本次单选时：
         * 点击其中某一个选中项，设置选中项目为可移动状态。
         */
        if ((_selectedList.count() > 1) && (_selectedList.contains(shape)))
        {
            for (auto item : _selectedList)
            {
                item->MoveBegin(point);
            }
            _moveEnabled = true;
        }
        else
        {
            for (auto item : _selectedList)
            {
                item->SetSelected(false);
            }

            _selectedList.clear();
        }

        _moveEnabled = true;
        shape->SetSelected(true);
        shape->MoveBegin(point);
        _selectedList.append(shape);
    }
    else
    {
        if (_selectedList.count() == 1)
        {
            if(_selectedList.last()->GetResizeCursorShape(point) != Qt::CursorShape::CrossCursor)
            {
                _dragResizeEnabled = true;
                _selectedList.last()->SetDragResizeEnabled(true);
            }
            else
            {
                _selectedList.last()->SetSelected(false);
                _selectedList.clear();
            }
        }
        else
        {
            for (auto item : _selectedList)
            {
                item->SetSelected(false);
            }
            _selectedList.clear();
        }
    }
}

bool PaintArea::singleSelectReleaseHandler(const QPoint &point)
{
    bool ret = false;
    GeometryShape *shape = nullptr;

    for (auto list : _coreMap.values())
    {
        if ((list != nullptr) && !list->isEmpty())
        {
            auto it = std::find_if(list->begin(), list->end(), [=](GeometryShape *rect){
                return rect->Contains(point);
            });

            if (it != list->end())
            {
                shape = *it;
                break;
            }
        }
    }

    if (shape != nullptr)
    {
        /*
         * 上一次选择为多选，本次单选时：
         * 点击其中某一个选中项，变为单选。
         */
        if ((_selectedList.count() > 1) && (_selectedList.contains(shape)))
        {
            for (auto item : _selectedList)
            {
                item->SetSelected(false);
            }

            _selectedList.clear();
            shape->SetSelected(true);
            _selectedList.append(shape);
        }

        ret = true;
    }

    return ret;
}

void PaintArea::cursorShapeHandler(const QPoint &point)
{
    bool useDefaultCursorShape = true;
    Qt::CursorShape cursorShape = Qt::CursorShape::CrossCursor;

    if (_moveEnabled || _dragResizeEnabled)
    {
        return;
    }

    if ((_lastPaintShape == nullptr) || _lastPaintShape->GetCompleted())
    {
        for (auto list : _coreMap.values())
        {
            if (list != nullptr)
            {
                for (auto item : *list)
                {
                    if (item->Contains(point))
                    {
                        cursorShape = Qt::SizeAllCursor;
                        useDefaultCursorShape = false;
                        break;
                    }

                    cursorShape = item->GetResizeCursorShape(point);
                    if (cursorShape != Qt::CursorShape::CrossCursor)
                    {
                        useDefaultCursorShape = false;
                        break;
                    }
                }
            }

            if (!useDefaultCursorShape)
            {
                break;
            }
        }

        this->setCursor(cursorShape);
    }
}

void PaintArea::selectAllShapes()
{
    for (auto item : _selectedList)
    {
        item->SetSelected(false);
    }
    _selectedList.clear();

    for (auto list : _coreMap.values())
    {
        if (!list->isEmpty())
        {
            for (auto item : *list)
            {
                _selectedList.append(item);
                item->SetSelected(true);
            }
        }
    }
}

void PaintArea::deleteSelectedShapes()
{
    if (!_selectedList.isEmpty())
    {
        for (auto item : _selectedList)
        {
            item->SetSelected(false);
            _coreMap[item->GetPaintType()]->removeOne(item);
            delete item;
        }

        _selectedList.clear();
        this->viewport()->update();
    }
}

QPoint PaintArea::AdjustedPos(const QPoint &point) const
{
    QPointF newPos(point);
    newPos.setX(newPos.x() / this->matrix().m11());
    newPos.setY(newPos.y() / this->matrix().m22());
    return newPos.toPoint();
}
