#ifndef TYPES_H
#define TYPES_H

#include <QObject>
#include <QString>

/**
 * @brief The EPaintType enum
 */
enum EPaintType {
    EPT_None = 0,
    EPT_Point,

    EPT_Line,
    EPT_Arc,
    EPT_Polyline,

    EPT_Circle,
    EPT_Rect,
    EPT_Ellipse,
    EPT_Polygon,

    EPT_End
};

class PaintTypeStrings
{
public:
    static QString GetStringEn(EPaintType type)
    {
        switch (type) {
        case EPaintType::EPT_Point:
            return "Point";
        case EPaintType::EPT_Line:
            return "Line";
        case EPaintType::EPT_Arc:
            return "Arc";
        case EPaintType::EPT_Polyline:
            return "Polyline";
        case EPaintType::EPT_Circle:
            return "Circle";
        case EPaintType::EPT_Rect:
            return "Rect";
        case EPaintType::EPT_Ellipse:
            return "Ellipse";
        case EPaintType::EPT_Polygon:
            return "Polygon";
        default:
            break;
        }

        return "Unkown";
    }

    static QString GetStringZh(EPaintType type)
    {
        switch (type) {
        case EPaintType::EPT_Point:
            return QString("点");
        case EPaintType::EPT_Line:
            return QString("直线");
        case EPaintType::EPT_Arc:
            return QString("圆弧");
        case EPaintType::EPT_Polyline:
            return QString("多段线");
        case EPaintType::EPT_Circle:
            return QString("圆");
        case EPaintType::EPT_Rect:
            return QString("矩形");
        case EPaintType::EPT_Ellipse:
            return QString("椭圆");
        case EPaintType::EPT_Polygon:
            return QString("多边形");
        default:
            break;
        }

        return "无";
    }
};

#endif // TYPES_H
