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

PaintArea::PaintArea(QWidget *parent) : QWidget(parent)
  , _paintType(EPaintType::EPT_None)
  , _lastPaintShape(nullptr)
  , _lastSelectedShape(nullptr)
  , _mouseButtonPressEnabled(true)
  , _mouseButtonReleaseEnabled(true)
  , _mouseMoveEnabled(true)
  , _moveEnabled(false)
  , _dragResizeEnabled(false)
  , _scaleX(1)
  , _scaleY(1)
{
    for (int i = EPaintType::EPT_Point; i < EPaintType::EPT_End; i++)
    {
        _coreMap.insert(static_cast<EPaintType>(i), new QList<GeometryShape *>());
    }
    qDebug() << _coreMap.count();

    _selectedList.clear();

    // 开启追踪鼠标，可触发mouseMoveEvent事件
    setMouseTracking(true);
    // 光标：+
    setCursor(Qt::CrossCursor);
    // 可触发keyPressEvent事件
    setFocusPolicy(Qt::StrongFocus);
    QWidget::installEventFilter(this);

    initGrid();
    this->setAcceptDrops(true);
}

PaintArea::~PaintArea()
{
    for (auto list : _coreMap.values())
    {
        if (list != nullptr)
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
}

bool PaintArea::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object);
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
    QWidget::paintEvent(event);
    QPainter painter(this);

    painter.save();
    drawGrid(painter);
    drawXYCoordinateText(painter);
    painter.restore();


//    painter.rotate(60);
    painter.scale(_scaleX, _scaleY);
    paintAllShapes(painter);
}

void PaintArea::mousePressEvent(QMouseEvent *event)
{
    EPaintType paintType = _paintType;
    GeometryShape *shape = nullptr;

    if ((event->button() == Qt::MouseButton::LeftButton) &&
            ((QApplication::keyboardModifiers() == Qt::AltModifier)))
    {
        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            _moveStartCursorPos = event->pos();
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
        qDebug() << this->objectName() << ": Left button press." << this->cursor().pos();

        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            if (QApplication::keyboardModifiers() == Qt::ControlModifier)
            {
                qDebug() << "Key: Ctrl + LeftButton, press";
                this->multiSelectHandler(event->pos());
                break;
            }

            this->singleSelectPressHandler(event->pos());
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
        _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_Painting, event->pos());

        if (_lastPaintShape->GetCompleted())
        {
            update();
        }

        break;
    case Qt::MouseButton::RightButton:
        switch (paintType) {
        case EPaintType::EPT_Polygon:
        case EPaintType::EPT_Polyline:
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_PaintEnd, QPoint());
            _lastPaintShape = nullptr;
            update();
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

    if ((event->button() == Qt::MouseButton::LeftButton) &&
            ((QApplication::keyboardModifiers() == Qt::AltModifier)))
    {
        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            return;
        }
    }

    if (!_coreMap.contains(paintType))
    {
        qCritical() << "Error: mouseReleaseEvent(), Invalid paint type!";
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

        if (this->dragReleaseHandler())
        {
            break;
        }

        if (moveReleaseHandler(event->pos()))
        {
            break;
        }

        if ((_lastPaintShape == nullptr) || (_lastPaintShape->GetCompleted()))
        {
            if (singleSelectReleaseHandler(event->pos()))
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
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_PaintEnd, event->pos());
            _lastPaintShape = nullptr;
            update();
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

    if ((_lastPaintShape == nullptr) || _lastPaintShape->GetCompleted())
    {
        if (((e->buttons() & Qt::MouseButton::LeftButton) == Qt::MouseButton::LeftButton) &&
                (QApplication::keyboardModifiers() == Qt::AltModifier))
        {
            this->setCursor(Qt::CursorShape::SizeAllCursor);
            this->move(this->pos() + (e->pos() - _moveStartCursorPos));
            return;
        }
        else
        {
            this->setCursor(Qt::CursorShape::CrossCursor);
        }
    }

    if (!_coreMap.contains(paintType))
    {
        qDebug() << "Error: mouseMoveEvent(), Invalid paint type!";
        return;
    }

    cursorShapeHandler(e->pos());

    /*
     * Ctrl + 鼠标移动，忽略...
     */
    if (QApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        return;
    }

    if (dragResizeHandler(e->pos()))
    {
        return;
    }

    if (moveHandler(e->pos()))
    {
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
            _lastPaintShape->UpdateState(GeometryShape::EPaintStateType::EPST_GuidePaintting, e->pos());
            update();
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
        qDebug() << "Key: Ctrl + A";
        selectAllHandler();
    }
    if ((event->key() == Qt::Key_Delete))
    {
        qDebug() << "Key: Del";
        deleteHandler();
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
        QPoint numDegrees = event->angleDelta();
        int step = 0;

        if (!numDegrees.isNull())
        {
            step = numDegrees.y();
        }

        event->accept();

        int curWidth = this->width();
        int curHeight = this->height();
        curWidth += step;
        curHeight += step;
        qDebug() << step;

        if (step > 0)
        {
            qDebug() << "Zoom out: " << curWidth << " " << curHeight;
        }
        else
        {
            qDebug() << "Zoom in: " << curWidth << " " << curHeight;
        }

        if ((curWidth > this->maximumWidth()) || (curWidth < this->minimumWidth()))
        {
            return;
        }
        if ((curHeight > this->maximumHeight()) || (curHeight < this->minimumHeight()))
        {
            return;
        }

        int oldWidth = this->width();
        int oldHeight = this->height();
        _scaleX += (curWidth - oldWidth) * 1.0 / oldWidth;
        _scaleY += (curHeight - oldHeight) * 1.0 / oldHeight;

        resize(curWidth, curHeight);

        if (event->angleDelta().y() > 0)
        {
            _gridGap += 0.1;
        }
        else if (event->angleDelta().y() < 0)
        {
            _gridGap -= 0.1;
        }
        else
        {

        }

        if (_gridGap >= _gridMaxGap)
        {
            _gridGap = _gridMinGap;
        }
        else if (_gridGap <= _gridMinGap)
        {
            _gridGap = _gridMaxGap;
        }

        update();
    }
}

void PaintArea::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    _gridColLineStartX = 0;
    _gridRowLineStartY = 0;
    update();
}

void PaintArea::paintAllShapes(QPainter& painter)
{
    for (auto list : _coreMap.values())
    {
        if (list != nullptr)
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
}

void PaintArea::drawXYCoordinateText(QPainter &painter)
{
    // TODO: 使用控件绑定到鼠标位置...
    QString xy = QString("X:%0 Y:%1").arg(QString::number(this->cursor().pos().x())).arg(QString::number(this->cursor().pos().y()));
    painter.drawText(QPoint(20, this->height() - 30), xy);
}

void PaintArea::initGrid()
{
    _gridMinGap = 20;
    _gridMaxGap = 30;
    _gridGap = (_gridMinGap + _gridMaxGap) / 2;
    _gridColLineStartX = 0;
    _gridRowLineStartY = 0;
    _gridLightPen = QPen(QColor("#f0fff0"));
    _gridLightPen.setStyle(Qt::PenStyle::DotLine);
    _gridLightPen.setWidth(1);
    _gridDarkPen  = QPen(QColor("#f0fff0"));
    _gridDarkPen.setStyle(Qt::PenStyle::DotLine);
    _gridDarkPen.setWidth(2);
}

void PaintArea::drawGrid(QPainter &painter)
{
    int lineCnt = 0;
    qreal bigY = _gridRowLineStartY;
    qreal smallY = _gridRowLineStartY;

    painter.setPen(_gridDarkPen);

    /*
     * 绘制行线条。
     */
    while (true)
    {
        painter.drawLine(QPointF(0.0, bigY), QPointF(this->width(), bigY));
        painter.drawLine(QPointF(0.0, smallY), QPointF(this->width(), smallY));

        bigY += _gridGap;
        smallY += _gridGap;
        if ((smallY <= 0) || (bigY >= this->height()))
        {
            break;
        }

        lineCnt++;
        if (lineCnt == 10)
        {
            painter.setPen(_gridDarkPen);
            lineCnt = 0;
        }
        else
        {
            painter.setPen(_gridLightPen);
        }
    }

    lineCnt = 0;
    qreal bigX = _gridColLineStartX;
    qreal smallX = _gridColLineStartX;
    painter.setPen(_gridDarkPen);

    while (true)
    {
        painter.drawLine(QPointF(bigX, 0.0), QPointF(bigX, this->height()));
        painter.drawLine(QPointF(smallX, 0.0), QPointF(smallX, this->width()));

        bigX += _gridGap;
        smallX += _gridGap;
        if ((smallX <=0 ) || (bigX >= this->width()))
        {
            break;
        }

        lineCnt++;
        if (lineCnt == 10)
        {
            painter.setPen(_gridDarkPen);
            lineCnt = 0;
        }
        else
        {
            painter.setPen(_gridLightPen);
        }
    }
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
        if ((list != nullptr) && !list->isEmpty())
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
    for (auto list : _coreMap.values())
    {
        if ((list != nullptr) && !list->isEmpty())
        {
            auto it = std::find_if(list->begin(), list->end(), [=](GeometryShape *rect){
                return rect->Contains(point);
            });

            if (it != list->end())
            {
                /*
                 * 上一次选择为多选，本次单选时：
                 * 点击其中某一个选中项，直接返回。
                 */
                if ((_selectedList.count() > 1) && (_selectedList.contains(*it)))
                {
                    for (auto item : _selectedList)
                    {
                        item->MoveBegin(point);
                    }
                    _moveEnabled = true;

                    break;
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
                (*it)->SetSelected(true);
                (*it)->MoveBegin(point);
                _selectedList.append(*it);

                break;
            }
            else
            {
                for (auto item : _selectedList)
                {
                    if(item->GetResizeCursorShape(point) != Qt::CursorShape::CrossCursor)
                    {
                        qDebug() << "YYYYYYYYY";
                        _dragResizeEnabled = true;
                        item->SetDragResizeEnabled(true);
                    }
                    else
                    {
                        _dragResizeEnabled = false;
                        item->SetDragResizeEnabled(false);
                    }
                }

                if (!_dragResizeEnabled)
                {
                    _selectedList.clear();
                }
            }
        }
    }
}

bool PaintArea::singleSelectReleaseHandler(const QPoint &point)
{
    bool ret = false;

    for (auto list : _coreMap.values())
    {
        if ((list != nullptr) && !list->isEmpty())
        {
            auto it = std::find_if(list->begin(), list->end(), [=](GeometryShape *rect){
                return rect->Contains(point);
            });

            if (it != list->end())
            {
                /*
                 * 上一次选择为多选，本次单选时：
                 * 点击其中某一个选中项，直接返回。
                 */
                if ((_selectedList.count() > 1) && (_selectedList.contains(*it)))
                {
                    for (auto item : _selectedList)
                    {
                        item->SetSelected(false);
                    }

                    _selectedList.clear();

                    (*it)->SetSelected(true);
                    _selectedList.append(*it);
                }

                ret = true;
                break;
            }
        }
    }

    return ret;
}

bool PaintArea::moveReleaseHandler(const QPoint &point)
{
    bool hasSelectedItem = false;
    bool ret = false;

    if (_moveEnabled && !_selectedList.isEmpty())
    {
        for (auto item : _selectedList)
        {
            if (item->GetSelected())
            {
                hasSelectedItem = true;
                item->MoveEnd(point);
            }
        }

        if (hasSelectedItem)
        {
            _moveEnabled = false;
            update();
            ret = true;
        }
    }

    return ret;
}

bool PaintArea::moveHandler(const QPoint &point)
{
    bool hasSelectedItem = false;
    bool ret = false;

    if (_moveEnabled)
    {
        if (!_selectedList.isEmpty())
        {
            for (auto item : _selectedList)
            {
                if (item->GetSelected())
                {
                    item->Move(point);
                    hasSelectedItem = true;
                }
            }

            if (hasSelectedItem)
            {
                _moveEnabled = true;
                update();
                ret = true;
            }
        }
    }

    return ret;
}

void PaintArea::cursorShapeHandler(const QPoint &point)
{
    bool useDefaultCursorShape = true;
    Qt::CursorShape cursorShape = Qt::CursorShape::CrossCursor;

    if ((_lastPaintShape == nullptr) || _lastPaintShape->GetCompleted())
    {
        for (auto list : _coreMap.values())
        {
            if ((list != nullptr))
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

void PaintArea::selectAllHandler()
{
    for (auto item : _selectedList)
    {
        item->SetSelected(false);
    }
    _selectedList.clear();

    for (auto list : _coreMap.values())
    {
        if ((list != nullptr) && !list->isEmpty())
        {
            for (auto item : *list)
            {
                _selectedList.append(item);
                item->SetSelected(true);
            }
        }
    }
}

void PaintArea::deleteHandler()
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
        update();
    }
}

bool PaintArea::dragResizeHandler(const QPoint &point)
{
    bool hasSelectedItem = false;
    bool ret = false;

    if (_dragResizeEnabled)
    {
        if (!_selectedList.isEmpty())
        {
            for (auto item : _selectedList)
            {
                if (item->GetSelected())
                {
                    item->DragResize(point);
                    hasSelectedItem = true;
                }
            }

            if (hasSelectedItem)
            {
                _dragResizeEnabled = true;
                update();
                ret = true;
            }
        }
    }

    return ret;
}

bool PaintArea::dragReleaseHandler()
{
    bool hasSelectedItem = false;
    bool ret = false;

    if (_dragResizeEnabled && !_selectedList.isEmpty())
    {
        for (auto item : _selectedList)
        {
            if (item->GetSelected())
            {
                hasSelectedItem = true;
                item->SetDragResizeEnabled(false);
            }
        }

        if (hasSelectedItem)
        {
            _dragResizeEnabled = false;
            update();
            ret = true;
        }
    }

    return ret;
}
