#ifndef PAINTAREA_H
#define PAINTAREA_H

#include <QWidget>
#include <QPen>
#include <QMap>

#include "Types.h"
#include "GeometryShape.h"

class PaintArea : public QWidget
{
    Q_OBJECT
public:
    constexpr static Qt::CursorShape DefaultCursorShape = Qt::CursorShape::CrossCursor;

    explicit PaintArea(QWidget *parent = nullptr);
    virtual ~PaintArea();

    bool eventFilter(QObject *object, QEvent *event) override;
    virtual void SetPaintType(EPaintType type);

signals:

protected:

    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
//    void dragEnterEvent(QDragEnterEvent *event) override;
//    void dragMoveEvent(QDragMoveEvent *event) override;
//    void dropEvent(QDropEvent *event) override;
//    void startDrag(Qt::DropActions supportedActions) override;

    void paintAllShapes(QPainter& painter);

private:
    EPaintType _paintType;
    GeometryShape *_lastPaintShape;
    GeometryShape *_lastSelectedShape;

    QMap<EPaintType, QList<GeometryShape *> *> _coreMap;
    QList<GeometryShape *> _selectedList;

    bool _mouseButtonPressEnabled;
    bool _mouseButtonReleaseEnabled;
    bool _mouseMoveEnabled;
    bool _moveEnabled;
    bool _dragResizeEnabled;

    qreal _scaleX;
    qreal _scaleY;

    QPoint _moveStartCursorPos;

    void paintCursorLine();
    /**
     * @brief Pulse edge check.
     * @param[in] stateCache Old state.
     * @param[in] state Current state.
     * @return int:
     *         1 - Rising edge.
     *         -1 - Falling edge.
     *         0 - None, hgih/low level.
     */
    int edgeCheck(bool stateCache, bool state);
    /**
     * @brief selectOneShape 多选时，选中或则取消选中某个图形。
     * @param[in] point 鼠标左键点击位置。
     * @details
     * Ctrl + 鼠标左键进行多选：
     * - 未添加，则添加到列表。
     * - 已添加，从列表中删除。
     */
    void multiSelectHandler(const QPoint &point);
    /**
     * @brief singleSelectHandler
     * @param point
     * @return
     */
    void singleSelectPressHandler(const QPoint &point);
    /**
     * @brief singleSelectReleaseHandler
     * @param point
     * @return true - 松开位置在图形内，false - 松开位置不在图形内。
     */
    bool singleSelectReleaseHandler(const QPoint &point);
    /**
     * @brief moveReleaseHandler
     * @param point
     * @return true - 需要重绘图新，false - 无需重绘图形。
     */
    bool moveReleaseHandler(const QPoint &point);
    void cursorShapeHandler(const QPoint &point);
    void selectAllShapes();
    void deleteSelectedShapes();
};

#endif // PAINTAREA_H
