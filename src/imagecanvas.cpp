#include "imagecanvas.h"

#include <QDebug>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>

#include "bbox_item.h"
#include "line_item.h"
#include "point_item.h"
#include "polygon_item.h"
#include "undo_cmds.h"

ImageCanvas::ImageCanvas(QObject *parent)
    : QGraphicsScene(parent), m_waitingForObj(false), m_drawObjStarted(false) {
  QObject::connect(this, &ImageCanvas::deferredRemoveItem, this,
                   &ImageCanvas::removeItemCmd, Qt::QueuedConnection);
}

void ImageCanvas::hideBoundingBoxes() {
  const auto all_items = items();
  for (auto item : all_items) {
    CustomItem *citem = dynamic_cast<CustomItem *>(item);
    if (citem && citem->isLocked()) {
      item->setVisible(false);
    }
  }
}

void ImageCanvas::helperParametersChanged() {
  const auto all_items = items();
  for (auto item : all_items) {
    CustomItem *citem = dynamic_cast<CustomItem *>(item);
    if (citem) {
      citem->helperParametersChanged();
    }
  }
}

void ImageCanvas::prepareForNewBBox(QString label) {
  m_waitingForObj = true;
  if (label != QString()) m_bboxLabel = label;
  views().first()->viewport()->setCursor(Qt::CrossCursor);
  m_waitingForTypeObj = Helper::kBBox;
}

void ImageCanvas::prepareForNewPoint(const QString &label) {
  m_waitingForObj = true;
  m_bboxLabel = label;
  views().first()->viewport()->setCursor(Qt::CrossCursor);
  m_waitingForTypeObj = Helper::kPoint;
}

void ImageCanvas::prepareForNewLine(const QString &label) {
  m_waitingForObj = true;
  m_bboxLabel = label;
  views().first()->viewport()->setCursor(Qt::CrossCursor);
  m_waitingForTypeObj = Helper::kLine;
}

void ImageCanvas::prepareForNewPolygon(const QString &label) {
  m_waitingForObj = true;
  m_bboxLabel = label;
  views().first()->viewport()->setCursor(Qt::CrossCursor);
  m_waitingForTypeObj = Helper::kPolygon;
  m_currentPolygon.clear();
}

void ImageCanvas::prepareForNewLineStrip(const QString &label) {
  prepareForNewPolygon(label);
  m_waitingForTypeObj = Helper::kLineStrip;
}

void ImageCanvas::showBoundingBoxes() {
  for (auto item : items()) {
    item->setVisible(true);
  }
}

void ImageCanvas::showLabels(bool show) {
  for (auto item : items()) {
    auto it = dynamic_cast<CustomItem *>(item);
    if (it) it->setShowLabel(show);
  }
  m_showLabels = show;
}

void ImageCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
  if (!m_currentImage.isNull()) painter->drawPixmap(0, 0, m_currentImage);

  QPen p = painter->pen();
  p.setColor(Qt::black);
  p.setCosmetic(true);
  painter->setPen(p);
  if (m_showGrid) {
    // upper + left

    qreal w = m_currentImage.width();
    qreal h = m_currentImage.height();
    const int n = 3;
    for (int i = 1; i < n + 1; ++i) {
      painter->drawLine(QPointF{i * w / (n + 1), 0},
                        QPointF{i * w / (n + 1), h});
      painter->drawLine(QPointF{0, i * h / (n + 1)},
                        QPointF{w, i * h / (n + 1)});
    }
    painter->drawLine(QPointF{0, 0.5 * 0.75 * h}, QPointF{w, 0.5 * 0.75 * h});
  }
}

void ImageCanvas::reset(const QImage &img, const QString &img_id) {
  m_currentImage = QPixmap::fromImage(img);

  const int marginW = Helper::kImageMarging;
  const int marginH = Helper::kImageMarging;
  setSceneRect(-marginW, -marginH, m_currentImage.width() + 2 * marginW,
               m_currentImage.height() + 2 * marginH);
  clear();
  m_imageId = img_id;
  update();
  m_undoStack.clear();
}

void ImageCanvas::addAnnotations(const Annotations &ann) {
  m_undoStack.beginMacro("Load Annotations");
  for (auto &bbox : ann.bboxes) {
    QRectF frect =
        QRectF{{bbox.x1, bbox.y1}, QPointF{bbox.x2, bbox.y2}} & sceneRect();
    if (frect.isNull() || frect.isEmpty() || !frect.isValid()) continue;

    m_undoStack.push(new AddBBoxCommand(frect, bbox.label, false));
  }
  for (auto &pt : ann.points) {
    m_undoStack.push(
        new AddPointCommand({pt.x, pt.y}, pt.label, false, nullptr));
  }
  for (auto &l : ann.lines) {
    m_undoStack.push(new AddLineCommand({l.x1, l.y1}, {l.x2, l.y2}, l.label,
                                        false, nullptr));
  }

  for (auto &poly : ann.polygons) {
    QPolygonF p;
    const int n = poly.xArray.size();
    for (int i = 0; i < n; ++i) {
      p.append({poly.xArray[i], poly.yArray[i]});
    }
    m_undoStack.push(new AddPolygonCommand(p, poly.label, false, nullptr));
  }

  for (auto &lst : ann.line_strips) {
    QPolygonF p;
    const int n = lst.xArray.size();
    for (int i = 0; i < n; ++i) {
      p.append({lst.xArray[i], lst.yArray[i]});
    }
    m_undoStack.push(new AddLineStripCommand(p, lst.label, false, nullptr));
  }

  m_undoStack.endMacro();
  m_undoStack.setClean();
}

void ImageCanvas::clear() {
  const auto all_items = items();
  for (auto *item : all_items) {
    auto *to_del = dynamic_cast<CustomItem *>(item);
    if (to_del) {
      removeItem(item);
    }
  }
}

Annotations ImageCanvas::annotations() {
  Annotations ann;
  ann.image_name = m_imageId;
  ann.img_h = m_currentImage.height();
  ann.img_w = m_currentImage.width();

  const auto all_items = items();
  for (const auto &item : all_items) {
    auto *bbox = dynamic_cast<CustomItem *>(item);
    if (bbox) {
      if (item->type() == Helper::kBBox) {
        BBox bbox;
        auto bbox_item = dynamic_cast<BoundingBoxItem *>(item);
        const QRectF frect = bbox_item->boundingBoxCoordinates();
        bbox.x1 = frect.topLeft().x();
        bbox.y1 = frect.topLeft().y();
        bbox.x2 = frect.bottomRight().x();
        bbox.y2 = frect.bottomRight().y();
        bbox.label = bbox_item->label();
        ann.bboxes.push_back(bbox);
        continue;
      }

      if (item->type() == Helper::kLine) {
        Line line;
        auto line_item = dynamic_cast<LineItem *>(item);
        const QPointF p1 = line_item->p1();
        const QPointF p2 = line_item->p2();
        line.x1 = p1.x();
        line.y1 = p1.y();
        line.x2 = p2.x();
        line.y2 = p2.y();
        line.label = line_item->label();
        ann.lines.append(line);
        continue;
      }

      if (item->type() == Helper::kPoint) {
        Point pt;
        auto pt_item = dynamic_cast<PointItem *>(item);
        const QPointF ct = pt_item->center();
        pt.x = ct.x();
        pt.y = ct.y();
        pt.label = pt_item->label();
        ann.points.push_back(pt);
        continue;
      }

      if (item->type() == Helper::kPolygon ||
          item->type() == Helper::kLineStrip) {
        auto poly_item = dynamic_cast<PolygonItem *>(item);
        const QPolygonF pdata =
            poly_item->getPolygonCoords();  // mapToScene(poly_item->polygon());

        Polygon poly;
        poly.label = poly_item->label();
        for (const auto &pt : pdata) {
          poly.xArray.push_back(pt.x());
          poly.yArray.push_back(pt.y());
        }
        if (item->type() == Helper::kPolygon) {
          ann.polygons.push_back(poly);
        } else {
          ann.line_strips.push_back(poly);
        }
        continue;
      }
    }
  }
  return ann;
}

QSize ImageCanvas::imageSize() { return m_currentImage.size(); }
QString ImageCanvas::imageId() { return m_imageId; }

void ImageCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_waitingForObj) {
    m_endPt = m_begPt = mouseEvent->scenePos();
    if (m_waitingForTypeObj == Helper::kBBox ||
        m_waitingForTypeObj == Helper::kLine) {
      m_drawObjStarted = true;
      m_waitingForObj = false;
    }

    if (m_waitingForTypeObj == Helper::kPolygon ||
        m_waitingForTypeObj == Helper::kLineStrip) {
      if (m_currentPolygon.empty()) {
        views()[0]->setMouseTracking(true);
      }
      m_currentPolygon.append(mouseEvent->scenePos());
      m_drawObjStarted = true;
    }

    if (m_waitingForTypeObj == Helper::kPoint) {
      views().first()->viewport()->setCursor(Qt::ArrowCursor);
      undoStack()->push(new AddPointCommand(mouseEvent->scenePos(), m_bboxLabel,
                                            true, nullptr));
      m_waitingForObj = false;
    }
  }
  QGraphicsScene::mousePressEvent(mouseEvent);
}

void ImageCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_drawObjStarted) {
    m_endPt = mouseEvent->scenePos();
    update();
  }
  QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void ImageCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_drawObjStarted) {
    views().first()->viewport()->setCursor(Qt::ArrowCursor);
    if (m_waitingForTypeObj == Helper::kBBox && (m_begPt != m_endPt)) {
      auto bbox = Helper::buildRectFromTwoPoints(m_begPt, m_endPt);
      auto cmd = new AddBBoxCommand(bbox, m_bboxLabel, true, nullptr);
      m_undoStack.push(cmd);
      m_drawObjStarted = false;
    }

    if (m_waitingForTypeObj == Helper::kLine && (m_begPt != m_endPt)) {
      undoStack()->push(
          new AddLineCommand(m_begPt, m_endPt, m_bboxLabel, true));
      m_drawObjStarted = false;
    }

    if (m_waitingForTypeObj == Helper::kPolygon) {
      int n = m_currentPolygon.size();

      if (n >= 3) {
        qreal d =
            Helper::pointLen(m_currentPolygon[0] - m_currentPolygon.last());
        if (d < Helper::kPointRadius * Helper::kInvScaleFactor) {
          if (n > 3) {
            m_currentPolygon.removeLast();
            Helper::imageCanvas()->undoStack()->push(new AddPolygonCommand(
                m_currentPolygon, m_bboxLabel, true, nullptr));
          }
          m_drawObjStarted = false;
          m_waitingForObj = false;
          views()[0]->setMouseTracking(false);
        }
      }
    }
    if (m_waitingForTypeObj == Helper::kLineStrip) {
      int n = m_currentPolygon.size();
      if (n >= 2) {
        if (mouseEvent->button() == Qt::RightButton) {
          Helper::imageCanvas()->undoStack()->push(new AddLineStripCommand(
              m_currentPolygon, m_bboxLabel, true, nullptr));
          m_drawObjStarted = false;
          m_waitingForObj = false;
          views()[0]->setMouseTracking(false);
        }
      }
    }
    update();
  }
  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ImageCanvas::drawForeground(QPainter *painter, const QRectF &rect) {
  QGraphicsScene::drawForeground(painter, rect);
  if (m_drawObjStarted) {
    auto p = painter->pen();
    p.setWidthF(3);
    p.setCosmetic(true);
    painter->setPen(p);

    if (m_waitingForTypeObj == Helper::kBBox)
      painter->drawRect(Helper::buildRectFromTwoPoints(m_begPt, m_endPt));
    if (m_waitingForTypeObj == Helper::kLine)
      painter->drawLine(m_begPt, m_endPt);

    if (m_waitingForTypeObj == Helper::kPolygon ||
        m_waitingForTypeObj == Helper::kLineStrip) {
      painter->setBrush(Qt::black);
      for (int i = 0; i < m_currentPolygon.count(); ++i) {
        if (i == 0) {
          painter->drawEllipse(m_currentPolygon[0],
                               Helper::kPointRadius * Helper::kInvScaleFactor,
                               Helper::kPointRadius * Helper::kInvScaleFactor);
        } else {
          painter->drawEllipse(
              m_currentPolygon[i],
              0.5 * Helper::kPointRadius * Helper::kInvScaleFactor,
              0.5 * Helper::kPointRadius * Helper::kInvScaleFactor);
        }
      }
      bool drawp = m_waitingForTypeObj == Helper::kPolygon;
      if (drawp) {
        painter->drawPolyline(m_currentPolygon);
      } else {
        painter->drawPolyline(m_currentPolygon);
      }
      if (m_currentPolygon.last() != m_endPt)
        painter->drawLine(m_currentPolygon.last(), m_endPt);
    }
  }
}

void ImageCanvas::keyPressEvent(QKeyEvent *keyEvent) {
  if (m_drawObjStarted) {
    if (keyEvent->key() == Qt::Key_Escape &&
        m_waitingForTypeObj == Helper::kPolygon) {
      m_drawObjStarted = false;
      m_waitingForObj = false;
      update();
    }
  }
  QGraphicsScene::keyPressEvent(keyEvent);
}

void ImageCanvas::setShowGrid(bool show) {
  m_showGrid = show;
  update();
}

void ImageCanvas::removeItemCmd(QGraphicsItem *item) {
  m_undoStack.push(new RemoveItemCommand(item));
}
