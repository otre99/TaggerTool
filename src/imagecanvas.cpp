#include "imagecanvas.h"

#include <QDebug>
#include <QGraphicsView>
#include <QPainter>

#include "bboxitem.h"
#include "imagecanvascommands.h"
#include "utils.h"

ImageCanvas::ImageCanvas(QObject *parent)
    : QGraphicsScene(parent),
      m_defaultBboxSize(20, 20),
      m_waitingForNewBbox(false),
      m_drawBboxStarted(false) {
    //  addItem(&display_text_);
}

void ImageCanvas::hideBoundingBoxes() {
  for (auto item : items()) {
    if (!item->isSelected()) {
      item->setVisible(false);
    }
  }
}

void ImageCanvas::showNiceText(const QString &txt) {
//  QGraphicsView *view = views().first();
//  QPointF pos = view->mapToScene(QPoint(10, 10));
//  display_text_.setPos(pos);
//  display_text_.setPlainText(txt);
//  display_text_.display();
}

void ImageCanvas::prepareForNewBBox(QString label, QSizeF bboxSize, bool what) {
  m_waitingForNewBbox = what;
  if (label != QString()) m_bboxLabel = label;
  if (bboxSize != QSizeF()) m_defaultBboxSize = bboxSize;
  views().first()->viewport()->setCursor(Qt::CrossCursor);
}

void ImageCanvas::showBoundingBoxes() {
  for (auto item : items()) {
    item->setVisible(true);
  }
}

void ImageCanvas::drawBackground(QPainter *painter, const QRectF &rect) {
  if (!m_currentImage.isNull()) painter->drawPixmap(0, 0, m_currentImage);
}

void ImageCanvas::reset(const QImage &img, const QString &img_id) {
  m_currentImage = QPixmap::fromImage(img);
  setSceneRect(0, 0, m_currentImage.width(), m_currentImage.height());
  clear();
  m_imageId = img_id;
  update();
}

void ImageCanvas::addBoundingBoxes(const QList<QRectF> &bboxes,
                                   const QStringList &labels) {
  if (labels.isEmpty()) {
    for (const auto &rect : bboxes) {
      QRectF frect = sceneRect() & rect;
      if (frect.isNull() || frect.isEmpty() || !frect.isValid()) continue;
      addItem(new BoundingBoxItem(frect));
    }
  } else if (labels.count() != bboxes.count()) {
    for (const auto &rect : bboxes) {
      QRectF frect = sceneRect() & rect;
      if (frect.isNull() || frect.isEmpty() || !frect.isValid()) continue;
      addItem(new BoundingBoxItem(frect, labels[0]));
    }
  } else {
    for (int i = 0; i < bboxes.count(); ++i) {
      QRectF frect = sceneRect() & bboxes[i];
      if (frect.isNull() || frect.isEmpty() || !frect.isValid()) continue;
      addItem(new BoundingBoxItem(frect, labels[i]));
    }
  }
}

void ImageCanvas::removeSelectedBoundingBoxes() {
  for (auto *item : selectedItems()) {
    removeItem(item);
    delete item;
  }
}

void ImageCanvas::clear() {
  for (auto *item : items()) {
    auto *to_del = dynamic_cast<BoundingBoxItem *>(item);
    if (to_del) {
      removeItem(item);
      delete item;
    }
  }
}

QMap<QString, QList<QRectF>> ImageCanvas::boundingBoxes() {
  QMap<QString, QList<QRectF>> result;
  for (const auto &item : items()) {
    auto *bbox = dynamic_cast<BoundingBoxItem *>(item);
    if (bbox) {
      result[bbox->m_info].append(bbox->boundingBoxCoordinates());
    }
  }
  return result;
}

QSize ImageCanvas::imageSize() { return m_currentImage.size(); }
QString ImageCanvas::imageId() { return m_imageId; }

void ImageCanvas::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_waitingForNewBbox) {
    m_endPt = m_begPt = mouseEvent->scenePos();
    m_drawBboxStarted = true;
    m_waitingForNewBbox = false;
  }
  QGraphicsScene::mousePressEvent(mouseEvent);
}

void ImageCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_drawBboxStarted) {
    m_endPt = mouseEvent->scenePos();
    update();
  }
  QGraphicsScene::mouseMoveEvent(mouseEvent);
}
void ImageCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) {
  if (m_drawBboxStarted) {
    m_drawBboxStarted = false;
    views().first()->viewport()->setCursor(Qt::ArrowCursor);
    if (m_begPt == m_endPt) return;

    auto bbox = Helper::buildRectFromTwoPoints(m_begPt, m_endPt);
    auto *item = new BoundingBoxItem(bbox, m_bboxLabel, nullptr, true);
    addItem(item);
    // undoStack_.push(new AddBBoxCommand(this, item) );

    update();
  }
  QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

void ImageCanvas::drawForeground(QPainter *painter, const QRectF &rect) {
  QGraphicsScene::drawForeground(painter, rect);
  if (m_drawBboxStarted)
    painter->drawRect(Helper::buildRectFromTwoPoints(m_begPt, m_endPt));
}
