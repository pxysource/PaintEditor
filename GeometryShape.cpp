#include "GeometryShape.h"

#include <complex>
#include <QDebug>
#include <QFileDialog>

GeometryShape::GeometryShape() : _state(0)
  , _completed(false)
  , _selected(false)
  , _paintType(EPaintType::EPT_None)
  , _moveEnabled(false)
  , _dragResizeEnabled(false)
  , _valid(true)
{
    initPen();
}

void GeometryShape::adjustPenWidthToDefault(QPen &pen, const QPainter &painter, qreal defaultPenWidth)
{
    pen.setWidthF(defaultPenWidth / painter.transform().m11());
}

void GeometryShape::initPen()
{
    _pointPen.setCapStyle(Qt::PenCapStyle::RoundCap);
    _pointPen.setWidthF(DefaultPointPenWidth);
    _pointPen.setColor(Qt::red);

    _linePen.setCapStyle(Qt::PenCapStyle::RoundCap);
    _linePen.setWidthF(DefaultLinePenWidth);
    _linePen.setColor(Qt::red);

    _guidePointPen.setCapStyle(Qt::PenCapStyle::RoundCap);
    _guidePointPen.setWidthF(DefaultGuidePointPenWidth);
//    _guidePointPen.setBrush(QColor("#7cfc00"));
    _guidePointPen.setBrush(Qt::yellow);

    _guideLinePen.setStyle(Qt::PenStyle::DashLine);
    _guideLinePen.setCapStyle(Qt::PenCapStyle::RoundCap);
    _guideLinePen.setWidthF(DefaultGuideLinePenWidth);
    _guideLinePen.setBrush(QColor("#7cfc00"));
}

Point::Point()
{
    _paintType = EPaintType::EPT_Point;
}

void Point::Paint(QPainter &painter)
{
    if (_point.isNull())
    {
        return;
    }

    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_pointPen);
        painter.drawPoint(_point);

        painter.setPen(_guideLinePen);
        QRectF rf(_point.x() - DELTA, _point.y() - DELTA, DELTA * 2, DELTA * 2);
        painter.drawArc(rf, 0, 360 * 16);
        return;
    }

    painter.setPen(_pointPen);
    painter.drawPoint(_point);
}

void Point::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    if (paintStateType != EPST_Painting)
    {
        return;
    }

    _state++;
    state = _state;

    if (state == 1)
    {
        _state = state;
        _point = point;
        _completed = true;
        qDebug() << point;
    }
}

bool Point::Contains(QPoint point)
{
    QLineF linef(_point, point);
    if (linef.length() <= DELTA)
    {
        return true;
    }

    return false;
}

void Point::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldPoint = _point;
}

void Point::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;
    _point = _oldPoint + aa;
}

Line::Line() : _isNeedGuideLine(false)
{
    _paintType = EPaintType::EPT_Line;
}

void Line::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guideLinePen);
        painter.drawLine(_guideLine);
        return;
    }

    if (_isNeedGuideLine)
    {
        painter.setPen(_guideLinePen);

        if (!_guideLine.p1().isNull() && !_guideLine.p2().isNull())
        {
            painter.drawLine(_guideLine);
        }
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        painter.drawLine(_line);
    }
}

void Line::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    switch (paintStateType)
    {
    case EPaintStateType::EPST_Painting:
        _state++;
        state = _state;

        if (state == 1)
        {
            _line.setP1(point);
            _guideLine.setP1(point);
            _isNeedGuideLine = true;
            _completed = false;
        }

        break;
    case EPaintStateType::EPST_PaintEnd:
        _state++;
        state = _state;

        if (state == 2)
        {
            _line.setP2(point);
            _guideLine.setP2(point);
            _isNeedGuideLine = false;
            _completed = true;
        }

        break;
    case EPaintStateType::EPST_GuidePaintting:
        _guideLine.setP2(point);
        break;
    default:
        break;
    }
}

bool Line::Contains(QPoint point)
{
    /*         c
     *         +
     *         |
     * a +-----+------+ b
     *         d
     */
    QLineF ac(_line.p1(), point);
    QLineF bc(_line.p2(), point);
    QLineF ab(_line);
    QLineF ba(_line.p2(), _line.p1());

    qreal angleA = ab.angleTo(ac);
    if (angleA > 180)
    {
        angleA = 360 - angleA;
    }

    qreal angleB = ba.angleTo(bc);
    if (angleB > 180)
    {
        angleB = 360 - angleB;
    }

    if ((angleA <= 90) && (angleB <= 90))
    {
        qreal pi = std::acos(-1);

        /*
         * 注意：此处计算cos必须利用Π，否则可能会导致cos计算值错误！
         */
        qreal ad = ac.length() * std::cos(angleA * pi / 180);

        /*
         * 勾股定理：ad ^ 2 + cd ^ 2 = ac ^ 2
         */
        qreal cd = std::sqrt(std::pow(ac.length(), 2) - std::pow(ad, 2));

        if (cd <= DELTA)
        {
            return true;
        }
    }

    return false;
}

void Line::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldLine = _line;
}

void Line::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;

    _line.setP1(_oldLine.p1() + aa);
    _line.setP2(_oldLine.p2() + aa);
    _guideLine.setP1(_line.p1());
    _guideLine.setP2(_line.p2());
}

Arc::Arc() : _isNeedGuideArc(false)
{
    _paintType = EPaintType::EPT_Arc;
}

void Arc::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guideLinePen);
        QLineF lf1(_guideCenter, _guideArcP2);
        QLineF lf2(_guideCenter, _guideArcP3);
        QRectF rf(_guideCenter.x() - lf1.length(), _guideCenter.y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, lf1.angle() * 16, (lf2.angle() - lf1.angle()) * 16);
        return;
    }

    if (_isNeedGuideArc)
    {
        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideCenter);
        painter.drawPoint(_guideArcP2);

        painter.setPen(_guideLinePen);
        painter.drawLine(_guideCenter, _guideArcP2);
        painter.drawLine(_guideCenter, _guideArcP3);

        QLineF lf1(_guideCenter, _guideArcP2);
        QLineF lf2(_guideCenter, _guideArcP3);
        QRectF rf(_guideCenter.x() - lf1.length(), _guideCenter.y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, lf1.angle() * 16, (lf2.angle() - lf1.angle()) * 16);
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        QLineF lf1(_center, _curArcP2);
        QLineF lf2(_center, _curArcP3);
        QRectF rf(_center.x() - lf1.length(), _center.y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, lf1.angle() * 16, (lf2.angle() - lf1.angle()) * 16);
    }
}

void Arc::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    switch (paintStateType)
    {
    case EPaintStateType::EPST_Painting:
        _state++;
        state = _state;

        if (state == 1)
        {
            _center = point;
            _guideCenter = point;
            _completed = false;
        }
        else if (state == 2)
        {
            _curArcP2 = point;
            _guideArcP2 = point;
            _isNeedGuideArc = true;
        }
        else if (state == 3)
        {
            _curArcP3 = point;
            _guideArcP3 = point;
            _isNeedGuideArc = false;
            _completed = true;
        }
        else
        {
        }

        break;
    case EPaintStateType::EPST_PaintEnd:
//        _state++;
//        state = _state;

//        if (state == 3)
//        {
//            _curArcP3 = point;
//            _guideArcP3 = point;
//            _isNeedGuideArc = false;
//            _isCompleted = true;
//        }

        break;
    case EPaintStateType::EPST_GuidePaintting:
        _guideArcP3 = point;
        break;
    default:
        break;
    }
}

bool Arc::Contains(QPoint point)
{
    /*
     *   b
     *   +
     *   |    + p
     *   |
     *   |
     *   +-------+ a
     *   o
     */

    QLineF op(_center, point);
    QLineF oa(_center, _curArcP2);
    QLineF ob(_center, _curArcP3);

    if (std::abs(op.length() - oa.length()) <= DELTA)
    {
        if (((op.angle() >= oa.angle()) && (op.angle() <= ob.angle())) ||
                ((op.angle() <= oa.angle()) && (op.angle() >= ob.angle())))
        {
            return true;
        }
    }

    return false;
}

void Arc::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldCenter = _center;
    _oldCurArcP2 = _curArcP2;
    _oldCurArcP3 = _curArcP3;
}

void Arc::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;

    _center = _oldCenter + aa;
    _curArcP2 = _oldCurArcP2 + aa;
    _curArcP3 = _oldCurArcP3 + aa;
    _guideCenter = _center;
    _guideArcP2 = _curArcP2;
    _guideArcP3 = _curArcP3;
}

Circle::Circle() : _isNeedGuide(false)
{
    _paintType = EPaintType::EPT_Circle;
}

void Circle::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guideLinePen);
        QLineF lf1(_guideRadiusLine);
        QRectF rf(_guideRadiusLine.p1().x() - lf1.length(), _guideRadiusLine.p1().y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, 0, 360 * 16);
        return;
    }

    if (_isNeedGuide)
    {
        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideRadiusLine.p1());
        painter.drawPoint(_guideRadiusLine.p2());

        painter.setPen(_guideLinePen);
        QLineF lf1(_guideRadiusLine);
        painter.drawLine(_guideRadiusLine);
        QRectF rf(_guideRadiusLine.p1().x() - lf1.length(), _guideRadiusLine.p1().y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, 0, 360 * 16);
    }

    if (_completed)
    {
        painter.setPen(_linePen);

        QLineF lf1(_radiusLine);
        QRectF rf(_radiusLine.p1().x() - lf1.length(), _radiusLine.p1().y() - lf1.length(), lf1.length() * 2, lf1.length() * 2);
        painter.drawArc(rf, 0, 360 * 16);
    }
}

void Circle::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    switch (paintStateType)
    {
    case EPaintStateType::EPST_Painting:
        _state++;
        state = _state;

        if (state == 1)
        {
            _radiusLine.setP1(point);
            _isNeedGuide = true;
            _completed = false;
        }

        break;
    case EPaintStateType::EPST_PaintEnd:
        _state++;
        state = _state;

        if (state == 2)
        {
            _radiusLine.setP2(point);
            _isNeedGuide = false;
            _completed = true;
        }

        break;
    case EPaintStateType::EPST_GuidePaintting:
        _guideRadiusLine = _radiusLine;
        _guideRadiusLine.setP2(point);
        break;
    default:
        break;
    }
}

bool Circle::Contains(QPoint point)
{
    /*
     *  +
     *  |   + p
     *  |
     *  +-----+ a
     *  o
     */

    QLineF op(_radiusLine.p1(), point);
    QLineF oa(_radiusLine);

    if (op.length() <= oa.length())
    {
        return true;
    }

    return false;
}

void Circle::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldRadiusLine = _radiusLine;
}

void Circle::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;

    _radiusLine.setP1(_oldRadiusLine.p1() + aa);
    _radiusLine.setP2(_oldRadiusLine.p2() + aa);
    _guideRadiusLine.setP1(_radiusLine.p1());
    _guideRadiusLine.setP2(_radiusLine.p2());
}

Rect::Rect() : _isNeedGuide(false)
  , _cursorShape(Qt::CursorShape::CrossCursor)
  , _dragCursorShape(Qt::CursorShape::CrossCursor)
{
    _paintType = EPaintType::EPT_Rect;
}

Rect::~Rect()
{
}

void Rect::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected)
    {
        if (_dragResizeEnabled)
        {
            painter.setPen(_guideLinePen);
            painter.drawRect(_guideRect);
            return;
        }

        if (_moveEnabled)
        {
            painter.setPen(_guidePointPen);
            painter.drawPoint(_guideRect.topLeft());
            painter.drawPoint(_guideRect.bottomRight());

            painter.setPen(_guideLinePen);
            painter.drawRect(_guideRect);
            return;
        }

        painter.setPen(_guideLinePen);
        painter.drawRect(_guideRect);

        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideRect.topLeft());
        painter.drawPoint(_guideRect.center().x(), _guideRect.top());
        painter.drawPoint(_guideRect.topRight());
        painter.drawPoint(_guideRect.left(), _guideRect.center().y());
        painter.drawPoint(_guideRect.right(), _guideRect.center().y());
        painter.drawPoint(_guideRect.bottomLeft());
        painter.drawPoint(_guideRect.center().x(), _guideRect.bottom());
        painter.drawPoint(_guideRect.bottomRight());

        return;
    }

    if (_isNeedGuide)
    {
        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideRect.topLeft());
        painter.drawPoint(_guideRect.bottomRight());

        painter.setPen(_guideLinePen);
        painter.drawRect(_guideRect);
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        painter.drawRect(_rect);
    }
}

void Rect::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    switch (paintStateType)
    {
    case EPaintStateType::EPST_Painting:
        _state++;
        state = _state;

        if (state == 1)
        {
            _p1 = point;
            _isNeedGuide = true;
            _completed = false;
        }

        break;
    case EPaintStateType::EPST_PaintEnd:
        _state++;
        state = _state;

        if (state == 2)
        {
            updateRect(_rect, _p1, point);
            _isNeedGuide = false;
            _completed = true;

            if ((_rect.width() < DELTA) || (_rect.height() < DELTA))
            {
                _valid = false;
            }
            else
            {
                _valid = true;
            }
        }

        break;
    case EPaintStateType::EPST_GuidePaintting:
        updateRect(_guideRect, _p1, point);
        break;
    default:
        break;
    }
}

bool Rect::Contains(QPoint point)
{
    return _rect.contains(point);
}

void Rect::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldP1 = _rect.topLeft();
    _oldP2 = _rect.bottomRight();
}

void Rect::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;
    QPoint p1 = _oldP1 + aa;
    QPoint p2 = _oldP2 + aa;
    updateRect(_rect, p1, p2);
    updateRect(_guideRect, p1, p2);
}

Qt::CursorShape Rect::GetResizeCursorShape(QPoint point)
{
    _cursorShape = Qt::CursorShape::CrossCursor;

    /*
     *
     *  ↖                                ↗
     *    ↘                            ↙
     *   +---+----------------------+---+       <-----+
     *   |   |      Top ↕           |   |             |
     *   +---+----------------------+---+             |
     *   |   |                      |   |             |
     *   |   |     Target Rect      |   |             |
     *   | ↔ |                      | ↔ |             +  Externel Rect
     *   |   |                      |   |             |
     *   +---+----------------------+---+             |
     *   |   |      Bottom    ↕     |   |             |
     *   +---+----------------------+---+       <-----+
     *    ↗                            ↖
     *  ↙                                ↘
     *
     */
    QRect curRect(_rect);
    QRect rectExternel(curRect.left() - DELTA, curRect.top() - DELTA,
                       curRect.width() + 2 * DELTA, curRect.height() + 2 * DELTA);

    if (!(rectExternel.contains(point) && !curRect.contains(point)))
    {
        return _cursorShape;
    }

    QRect topRect(curRect.left(), curRect.top() - DELTA, curRect.width(), DELTA);
    QRect bottomRect(curRect.left(), curRect.bottom(), curRect.width(), DELTA);

    QRect leftRect(curRect.left() - DELTA, curRect.top(), DELTA, curRect.height());
    QRect rightRect(curRect.right(), curRect.top(), DELTA, curRect.height());

    QRect leftTopRect(curRect.left() - DELTA, curRect.top() - DELTA, DELTA, DELTA);
    QRect rightBottom(curRect.right(), curRect.bottom(), DELTA, DELTA);

    QRect leftBottom(curRect.left() - DELTA, curRect.bottom(), DELTA, DELTA);
    QRect RightTop(curRect.right(), curRect.top() - DELTA, DELTA, DELTA);

    if (topRect.contains(point) || bottomRect.contains(point))
    {
        _cursorShape = Qt::CursorShape::SizeVerCursor;
        return _cursorShape;
    }
    if (leftRect.contains(point) || rightRect.contains(point))
    {
        _cursorShape = Qt::CursorShape::SizeHorCursor;
        return _cursorShape;
    }
    if (leftTopRect.contains(point) || rightBottom.contains(point))
    {
        _cursorShape = Qt::CursorShape::SizeFDiagCursor;
        return _cursorShape;
    }
    if (leftBottom.contains(point) || RightTop.contains(point))
    {
        _cursorShape = Qt::CursorShape::SizeBDiagCursor;
        return _cursorShape;
    }

    return _cursorShape;
}

void Rect::DragResize(const QPoint &point)
{
    if (!_dragResizeEnabled)
    {
        return;
    }

    switch (_cursorShape)
    {
    case Qt::CursorShape::SizeVerCursor:
        this->dragResizeRectVertical(_rect, point);
        this->dragResizeRectVertical(_guideRect, point);
        break;
    case Qt::CursorShape::SizeHorCursor:
        this->dragResizeRectHorical(_rect, point);
        this->dragResizeRectHorical(_guideRect, point);
        break;
    case Qt::CursorShape::SizeFDiagCursor:
        this->dragResizeRectVertical(_rect, point);
        this->dragResizeRectVertical(_guideRect, point);
        this->dragResizeRectHorical(_rect, point);
        this->dragResizeRectHorical(_guideRect, point);
        break;
    case Qt::CursorShape::SizeBDiagCursor:
        this->dragResizeRectVertical(_rect, point);
        this->dragResizeRectVertical(_guideRect, point);
        this->dragResizeRectHorical(_rect, point);
        this->dragResizeRectHorical(_guideRect, point);
        break;
    default:
        break;
    }
}

void Rect::SetDragResizeEnabled(bool enable)
{
    GeometryShape::SetDragResizeEnabled(enable);
    _dragCursorShape = _cursorShape;
}

void Rect::updateRect(QRect &rect, const QPoint &p1, const QPoint &p2)
{
    /*
     * o
     * +-----------------------------------> x
     * | p1     p2            p1      p2
     * | +      +             +       +
     * |  \      \           /       /
     * |   \      \         /       /
     * |    \      \       /       /
     * |     +      +     +       +
     * |     p2     p1    p2      p1
     * |
     * v
     *
     * Y
     *
     */
    if ((p1.x() < p2.x()) && (p1.y() < p2.y()))
    {
        rect.setTopLeft(p1);
        rect.setBottomRight(p2);
    }
    else if ((p1.x() > p2.x()) && (p1.y() > p2.y()))
    {
        rect.setTopLeft(p2);
        rect.setBottomRight(p1);
    }
    else if ((p1.x() > p2.x()) && (p1.y() < p2.y()))
    {
        rect.setTopRight(p1);
        rect.setBottomLeft(p2);
    }
    else if ((p1.x() < p2.x()) && (p1.y() > p2.y()))
    {
        rect.setTopRight(p2);
        rect.setBottomLeft(p1);
    }
    else
    {
        rect.setTopLeft(p1);
        rect.setBottomRight(p2);
    }
}

void Rect::dragResizeRectVertical(QRect &rect, const QPoint &point)
{
    int y = 0;
    int delta = DELTA / 2;

    y = point.y();

    if (y < rect.top())
    {
        rect.setTop(y + delta);
    }
    else if ((y > rect.top()) && (y < rect.bottom()))
    {
        if (std::abs(y - rect.top()) <= DELTA)
        {
            _rect.setTop(y - delta);
        }
        else if (std::abs(y - rect.bottom()) <= DELTA)
        {
            rect.setBottom(y + delta);
        }
        else
        {
            return;
        }
    }
    else if (y > rect.bottom())
    {
        rect.setBottom(y - delta);
    }
    else
    {
        return;
    }
}

void Rect::dragResizeRectHorical(QRect &rect, const QPoint &point)
{
    int x = 0;
    int delta = DELTA / 2;

    x = point.x();
    if (x < rect.left())
    {
        rect.setLeft(point.x() + delta);
    }
    else if ((x > rect.left()) && (x < rect.right()))
    {
        if ((x - rect.left()) <= DELTA)
        {
            rect.setLeft(point.x() - delta);
        }
        else if ((rect.right() - x) <= DELTA)
        {
            rect.setRight(point.x() + delta);
        }
        else
        {
            return;
        }
    }
    else if (x > rect.right())
    {
        rect.setRight(point.x() - delta);
    }
    else
    {
        return;
    }
}

Ellipse::Ellipse()
{
    _paintType = EPaintType::EPT_Ellipse;
}

void Ellipse::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideRect.center());
        painter.setPen(_guideLinePen);
        painter.drawEllipse(_guideRect);
        return;
    }

    if (_isNeedGuide)
    {
        painter.setPen(_guidePointPen);
        painter.drawPoint(_guideRect.center());

        painter.setPen(_guideLinePen);
        painter.drawEllipse(_guideRect);
        painter.drawLine(_guideRect.left(), _guideRect.top() + _guideRect.height() / 2,
                         _guideRect.right(), _guideRect.top() + _guideRect.height() / 2);
        painter.drawLine(_guideRect.left() + _guideRect.width() / 2, _guideRect.top(),
                         _guideRect.left() + _guideRect.width() / 2, _guideRect.bottom());
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        painter.drawEllipse(_rect);
    }
}

Polygon::Polygon() : _isShowGuide(false)
{
    _paintType = EPaintType::EPT_Polygon;
}

void Polygon::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guideLinePen);
        painter.drawPolygon(_guidePolygon);
        return;
    }

    if (_isShowGuide)
    {
        painter.setPen(_guideLinePen);
        painter.drawPolygon(_guidePolygon);
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        painter.drawPolygon(_polygon);
    }
}

void Polygon::UpdateState(EPaintStateType paintStateType, QPoint point)
{
    int state = 0;

    switch (paintStateType)
    {
    case EPaintStateType::EPST_Painting:
        _state++;
        state = _state;

        _polygon.putPoints(state - 1, 1, point.x(), point.y());

        if (state == 1)
        {
            _isShowGuide = true;
            _completed = false;
        }

        break;
    case EPaintStateType::EPST_PaintEnd:
        _state++;
        state = _state;

        _isShowGuide = false;
        _completed = true;
        break;
    case EPaintStateType::EPST_GuidePaintting:
        _guidePolygon = _polygon;
        _guidePolygon.push_back(point);
        break;
    default:
        break;
    }
}

bool Polygon::Contains(QPoint point)
{
    return _polygon.containsPoint(point, Qt::FillRule::OddEvenFill);
}

void Polygon::MoveBegin(const QPoint &point)
{
    GeometryShape::MoveBegin(point);
    _oldPolygon = _polygon;
}

void Polygon::Move(QPoint point)
{
    QPoint aa = point - _moveStartCursorPoint;

    _polygon.clear();
    foreach (auto item, _oldPolygon) {
        _polygon.append(item + aa);
    }
    _guidePolygon.clear();
    _guidePolygon.append(_polygon);
}

Polyline::Polyline()
{
    _paintType = EPaintType::EPT_Polyline;
}

void Polyline::Paint(QPainter &painter)
{
    GeometryShape::Paint(painter);

    if (_selected && _moveEnabled)
    {
        painter.setPen(_guideLinePen);
        painter.drawPolyline(_guidePolygon);
        return;
    }

    if (_isShowGuide)
    {
        painter.setPen(_guideLinePen);
        painter.drawPolyline(_guidePolygon);
    }

    if (_completed)
    {
        painter.setPen(_linePen);
        painter.drawPolyline(_polygon);
    }
}

GeometryShape *GeometryShapeFactory::CreateGeometryShape(EPaintType paintType)
{
    GeometryShape *shape = nullptr;

    switch (paintType)
    {
    case EPaintType::EPT_Point:
        shape = new Point();
        break;
    case EPaintType::EPT_Line:
        shape = new Line();
        break;
    case EPaintType::EPT_Arc:
        shape = new Arc();
        break;
    case EPaintType::EPT_Circle:
        shape = new Circle();
        break;
    case EPaintType::EPT_Rect:
        shape = new Rect();
        break;
    case EPaintType::EPT_Ellipse:
        shape = new Ellipse();
        break;
    case EPaintType::EPT_Polygon:
        shape = new Polygon();
        break;
    case EPaintType::EPT_Polyline:
        shape = new Polyline();
        break;
    default:
        qCritical() << "Error: Invalid paint type!";
        break;
    }

    return  shape;
}
