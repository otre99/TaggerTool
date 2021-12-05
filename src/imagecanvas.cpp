#include "imagecanvas.h"

#include <QDebug>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QPainter>

#include "bbox_item.h"
#include "line_item.h"
#include "point_item.h"
#include "polygon_item.h"

ImageCanvas::ImageCanvas(QObject *parent)
    : QGraphicsScene(parent), m_waitingForObj(false), m_drawObjStarted(false) {}

void ImageCanvas::hideBoundingBoxes() {
  const auto all_items = items();
  for (auto item : all_items) {
    if (!item->isSelected()) {
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
    painter->drawLine(
        QPoint{m_currentImage.width() / 2, 0},
        QPoint{m_currentImage.width() / 2, m_currentImage.height()});
    painter->drawLine(
        QPoint{0, m_currentImage.height() / 2},
        QPoint{m_currentImage.width(), m_currentImage.height() / 2});
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
}

void ImageCanvas::addAnnotations(const Annotations &ann) {
  for (auto &bbox : ann.bboxes) {
    QRectF frect =
        QRectF{{bbox.x1, bbox.y1}, QPointF{bbox.x2, bbox.y2}} & sceneRect();
    if (frect.isNull() || frect.isEmpty() || !frect.isValid()) continue;
    auto item = new BoundingBoxItem(frect, bbox.label);
    item->setShowLabel(m_showLabels);
    addItem(item);
  }
  for (auto &pt : ann.points) {
    auto item = new PointItem({pt.x, pt.y}, pt.label);
    addItem(item);
  }
  for (auto &l : ann.lines) {
    auto item = new LineItem({l.x1, l.y1}, {l.x2, l.y2}, l.label);
    addItem(item);
  }

  for (auto &poly : ann.polygons) {
    QPolygonF p;
    const int n = poly.xArray.size();
    for (int i = 0; i < n; ++i) {
      p.append({poly.xArray[i], poly.yArray[i]});
    }
    auto item = new PolygonItem(p, poly.label);
    addItem(item);
  }
}

void ImageCanvas::removeUnlockedItems() {
  const auto all_items = items();
  bool needSaveData = false;
  for (auto *item : all_items) {
    auto ptr = dynamic_cast<CustomItem *>(item);
    if (ptr && ptr->isLocked() == false) {
      removeItem(item);
      delete item;
      needSaveData = true;
    }
  }
  if (needSaveData) emit needSaveChanges();
}

void ImageCanvas::clear() {
  const auto all_items = items();
  for (auto *item : all_items) {
    auto *to_del = dynamic_cast<CustomItem *>(item);
    if (to_del) {
      removeItem(item);
      delete item;
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

      if (item->type() == Helper::kPolygon) {
        auto poly_item = dynamic_cast<PolygonItem *>(item);
        const QPolygonF pdata = poly_item->mapToScene(poly_item->polygon());

        Polygon poly;
        poly.label = poly_item->label();
        for (const auto &pt : pdata) {
          poly.xArray.push_back(pt.x());
          poly.yArray.push_back(pt.y());
        }
        ann.polygons.push_back(poly);
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

    if (m_waitingForTypeObj == Helper::kPolygon) {
      if (m_currentPolygon.empty()) {
        views()[0]->setMouseTracking(true);
      }
      m_currentPolygon.append(mouseEvent->scenePos());
      m_drawObjStarted = true;
    }

    if (m_waitingForTypeObj == Helper::kPoint) {
      views().first()->viewport()->setCursor(Qt::ArrowCursor);
      auto *item =
          new PointItem(mouseEvent->scenePos(), m_bboxLabel, nullptr, true);
      addItem(item);
      m_waitingForObj = false;
      emit needSaveChanges();
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
      auto *item = new BoundingBoxItem(bbox, m_bboxLabel, nullptr, true);
      item->setShowLabel(m_showLabels);
      addItem(item);
      emit needSaveChanges();
      m_drawObjStarted = false;
    }

    if (m_waitingForTypeObj == Helper::kLine && (m_begPt != m_endPt)) {
      auto *item = new LineItem(m_begPt, m_endPt, m_bboxLabel, nullptr, true);
      item->setShowLabel(m_showLabels);
      addItem(item);
      emit needSaveChanges();
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
            auto *item =
                new PolygonItem(m_currentPolygon, m_bboxLabel, nullptr, true);
            item->setShowLabel(m_showLabels);
            addItem(item);
            emit needSaveChanges();
          }
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

    if (m_waitingForTypeObj == Helper::kPolygon) {
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
      painter->drawPolyline(m_currentPolygon);
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
