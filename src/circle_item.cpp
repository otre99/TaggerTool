#include "circle_item.h"
#include "editdialog.h"
#include <QCursor>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

#include "imagecanvas.h"
#include "undo_cmds.h"

extern Helper globalHelper;

CircleItem::CircleItem(const QPointF &center, double radius, const QString &label,
                       QGraphicsItem *parent, bool ready)
    : QGraphicsEllipseItem(0, 0, 2*radius, 2*radius, parent) {
    setFlags(QGraphicsItem::ItemIsFocusable |
             QGraphicsItem::ItemSendsGeometryChanges);

    setPos(center - QPointF{radius, radius});

    __setLocked(this, !ready);
    if (ready) {
        setSelected(ready);
    }

    __setLabel(this, label);
    auto p = pen();
    p.setWidthF(Helper::penWidth());
    setPen(p);
    setAcceptHoverEvents(true);
}

void CircleItem::helperParametersChanged() {
    prepareGeometryChange();
    __calculateLabelSize(m_label);
    QPen p = pen();
    p.setWidthF(Helper::penWidth());
    setPen(p);
}

void CircleItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
    (void)widget;
    QPen p = pen();
    painter->setPen(p);

    QRectF brect = rect(); // boundingRect();
    if (m_moveEnable) {
        painter->setBrush(QBrush(Helper::kUnlockedBBoxColor));
    } else {
        painter->setBrush(QBrush(Helper::kLockedBBoxColor));
    }
    if (!m_moveEnable) {
        QPen pp = p;
        pp.setWidthF(Helper::kLineWidth);
        pp.setCosmetic(true);
        painter->setPen(pp);
        painter->drawEllipse(brect);
        painter->setPen(p);
    }
    if (m_moveEnable) {
        auto pp = p;
        pp.setCosmetic(true);
        pp.setWidthF(Helper::kLineWidth);
//        pp.setWidthF(qMin(1.0, p.widthF()));
        pp.setColor(Qt::black);
        painter->setPen(pp);
        painter->drawEllipse(brect);

        const QPointF c = rect().center();
        qreal r = radius();
        painter->drawLine(QPointF{c.x()-r, c.y()}, QPointF{c.x()+r, c.y()});
        painter->drawLine(QPointF{c.x(), c.y()-r}, QPointF{c.x(), c.y()+r});

        if (m_currentCorner==kBorder){
            pp.setStyle(Qt::DotLine);
            painter->setPen(pp);
            painter->drawLine(c, m_lastPt);
        }
    }
    if (m_showLabel) {
        painter->setFont(globalHelper.fontLabel());
        p.setColor(Qt::black);
        painter->setPen(p);

        brect = boundingRect();
        qreal dx = (brect.width()-m_labelLen)/2;
        QRectF lb_rect(brect.x()+dx, brect.y(), m_labelLen, m_labelHeight);
        painter->fillRect(lb_rect, Helper::kLabelColor);
        painter->drawText(lb_rect, Qt::AlignVCenter | Qt::AlignHCenter, m_label);
    }
}

void CircleItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_currentCorner == kCenter || !m_moveEnable)
        QGraphicsEllipseItem::mouseMoveEvent(event);
    else {
        const QPointF pos = event->pos();
        const QPointF cpt = rect().center();
        const qreal new_radius = Helper::pointLen(pos-cpt);

        setRect({
            cpt.x()-new_radius,
            cpt.y()-new_radius,
            2*new_radius,
            2*new_radius
        });
        m_lastPt = pos;
    }
}

void CircleItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    m_currentCorner = kInvalid;
    if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier) &&
        event->button() == Qt::LeftButton) {
        __swapStackOrder(this, scene()->items(event->scenePos()));
    } else if (event->modifiers() == Qt::ShiftModifier &&
               event->button() == Qt::LeftButton) {
        setLocked(m_moveEnable);
    } else if (event->button() == Qt::RightButton && m_moveEnable) {
        showEditDialog(this, event->screenPos());
    } else {
        m_currentCorner = positionInside(event->pos());
        m_oldPos = pos();
        m_oldRect = rect();
        if (m_currentCorner == kCenter || !m_moveEnable) {
            QGraphicsEllipseItem::mousePressEvent(event);
        } else {
            update();
            m_lastPt = event->pos();
        }
    }
}

void CircleItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    QGraphicsEllipseItem::mouseDoubleClickEvent(event);
}

void CircleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    setCursor(Qt::ArrowCursor);
    QGraphicsEllipseItem::mouseReleaseEvent(event);

    bool cond1 = m_oldRect != rect();
    bool cond2 = m_oldPos != pos();
    m_currentCorner = kInvalid;
    if (cond1 || cond2 ) {
        auto canvas = dynamic_cast<ImageCanvas *>(this->scene());
        if (cond1) {
            canvas->undoStack()->push(
                new RadiusChangeCircleCommand(m_oldRect, rect(), this));
            m_oldRect = rect();
        }
        if (cond2){
            canvas->undoStack()->push(
                new MoveItemCommand(m_oldPos, pos(), this));
            m_oldPos = pos();
        }
    }
    update();
}

//void CircleItem::keyPressEvent(QKeyEvent *event) {
//   if (event->key() == Qt::Key_Return) {
//        this->setLocked(true);
//   } else {
//        QGraphicsItem::keyPressEvent(event);
//   }
//}

QRectF CircleItem::boundingRect() const {
    QRectF br = QGraphicsEllipseItem::boundingRect();
    double dw = 0;
    if (br.width() < m_labelLen) {
        dw = m_labelLen - br.width();
    }
    qreal o = pen().widthF() / 2.0;
    return rect().adjusted(-o-dw/2, -o -m_labelHeight, dw/2 + o, o);
}


// private
CircleItem::CORNER CircleItem::positionInside(const QPointF &pos) {
    const qreal th = pen().widthF()/2;
    const qreal r = radius();
    const qreal dist = Helper::pointLen( pos-rect().center() );
    qreal delta = r - dist;

    if ( dist < r - th )
        return kCenter;

    if (qAbs(delta) < th)
        return kBorder;

    return kInvalid;
}


QPointF CircleItem::center() const {
    return mapToScene(rect().center());
}


qreal  CircleItem::radius() const {
    return rect().width()/2;
}
