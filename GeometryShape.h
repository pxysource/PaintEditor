#ifndef GEOMETRYSHAPE_H
#define GEOMETRYSHAPE_H

#include <QObject>
#include <QPoint>
#include <QPainter>

#include "Types.h"

class GeometryShape
{
public:
//    constexpr static int END_STATE = -1;

    enum EPaintStateType
    {
        EPST_Painting = 1,
        EPST_PaintEnd,
        EPST_GuidePaintting,
    };

    GeometryShape();
    virtual ~GeometryShape() {}
    virtual void Paint(QPainter &painter) = 0;
    virtual void UpdateState(EPaintStateType paintStateType, QPoint point) = 0;
    virtual bool Contains(QPoint point)
    {
        Q_UNUSED(point);
        return false;
    }
    virtual void MoveBegin(const QPoint& point)
    {
        _moveEnabled = true;
        _moveStartCursorPoint = point;
    }
    virtual void Move(QPoint point)
    {
        Q_UNUSED(point);
    }
    virtual void MoveEnd(const QPoint& point)
    {
        _moveEnabled = false;
        this->Move(point);
    }
    virtual void SaveToFile()
    {
    }

    int GetState() const
    {
        return _state;
    }
    bool GetCompleted() const
    {
        return _completed;
    }

    virtual void SetSelected(bool selected)
    {
        _selected = selected;
    }
    bool GetSelected() const
    {
        return _selected;
    }
    EPaintType GetPaintType() const
    {
        return _paintType;
    }

    virtual Qt::CursorShape GetResizeCursorShape(QPoint point)
    {
        Q_UNUSED(point);
        return Qt::CursorShape::CrossCursor;
    }
    virtual void DragResize(const QPoint &point)
    {
        Q_UNUSED(point);
    }
    virtual void SetDragResizeEnabled(bool enable)
    {
        _dragResizeEnabled = enable;
    }
    bool IsValid()
    {
        return _valid;
    }

protected:
    QPen _pointPen;
    QPen _linePen;
    QPen _guidePointPen;
    QPen _guideLinePen;
    int _state;
    bool _completed;
    bool _selected;
    EPaintType _paintType;
    QPoint _moveStartCursorPoint;
    bool _moveEnabled;
    bool _dragResizeEnabled;
    bool _valid;

private:
    void initPen();
};

class Point : public GeometryShape
{
public:
    constexpr static int DELTA = 10;

    Point();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;

private:
    QPoint _point;
    QPoint _oldPoint;
};

class Line : public GeometryShape
{
public:
    constexpr static int DELTA = 5;

    Line();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;

private:
    QLine _line;
    QLine _oldLine;
    QLine _guideLine;
    bool _isNeedGuideLine;
};

class Arc : public GeometryShape
{
public:
    constexpr static int DELTA = 10;

    Arc();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;

private:
    QPoint _center;
    QPoint _curArcP2;
    QPoint _curArcP3;
    QPoint _oldCenter;
    QPoint _oldCurArcP2;
    QPoint _oldCurArcP3;
    QPoint _guideCenter;
    QPoint _guideArcP2;
    QPoint _guideArcP3;
    bool _isNeedGuideArc;
};

class Circle : public GeometryShape
{
public:
    Circle();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;

private:
    QLine _radiusLine;
    QLine _oldRadiusLine;
    QLine _guideRadiusLine;
    bool _isNeedGuide;
};

class Rect : public GeometryShape
{
public:
    constexpr static int DELTA = 20;

    Rect();
    ~Rect();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;
    Qt::CursorShape GetResizeCursorShape(QPoint point) override;
    void DragResize(const QPoint &point) override;
    void SetDragResizeEnabled(bool enable) override;

protected:
    bool _isNeedGuide;
    QRect _rect;
    QRect _guideRect;

    QPoint _p1;
    QPoint _oldP1;
    QPoint _oldP2;

    Qt::CursorShape _cursorShape;
    Qt::CursorShape _dragCursorShape;

    void updateRect(QRect &rect, const QPoint &p1, const QPoint &p2);
    void dragResizeRectVertical(QRect &rect, const QPoint& point);
    void dragResizeRectHorical(QRect &rect, const QPoint& point);
};

class Ellipse : public Rect
{
public:
    Ellipse();
    void Paint(QPainter &painter) override;
};

class Polygon : public GeometryShape
{
public:
    Polygon();
    void Paint(QPainter &painter) override;
    void UpdateState(EPaintStateType paintStateType, QPoint point) override;
    bool Contains(QPoint point) override;
    void MoveBegin(const QPoint &point) override;
    void Move(QPoint point) override;

protected:
    QPolygon _polygon;
    QPolygon _oldPolygon;
    QPolygon _guidePolygon;
    bool _isShowGuide;
};

class Polyline : public Polygon
{
public:
    Polyline();
    void Paint(QPainter &painter) override;
};

class GeometryShapeFactory
{
public:
    GeometryShapeFactory() = delete;
    ~GeometryShapeFactory() = delete;
    GeometryShapeFactory(const GeometryShapeFactory&) = delete;
    static GeometryShape *CreateGeometryShape(EPaintType paintType);
};

#endif // GEOMETRYSHAPE_H
