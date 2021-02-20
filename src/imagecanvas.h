#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include "bboxitem.h"
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QObject>
#include <QPropertyAnimation>
#include <QUndoStack>

// class NiceText : public QGraphicsTextItem {
//  QPropertyAnimation anim;

// public:
//  NiceText() {
//    anim.setTargetObject(this);
//    anim.setPropertyName("opacity");
//    anim.setKeyValueAt(0.0, 0.0);
//    anim.setKeyValueAt(0.5, 1.0);
//    anim.setKeyValueAt(1.0, 0.0);
//    anim.setDuration(1200);
//  }

//  void display() {
//    setOpacity(0.0);
//    setVisible(true);
//    anim.start();
//  }
//};

class ImageCanvas : public QGraphicsScene {
  Q_OBJECT
private:
  QPixmap m_currentImage;
  QString m_imageId;
  QSizeF m_defaultBboxSize;
  QString m_bboxLabel;
  bool m_waitingForNewBbox;
  bool m_drawBboxStarted;
  QPointF m_begPt;
  QPointF m_endPt;
  bool m_needSaveChanges;
  // NiceText display_text_;
  // TODO (otre99): to allow Ctr+Z
  // QUndoStack undoStack_;

public:
  void showNiceText(const QString &txt);
  ImageCanvas(QObject *parent = nullptr);
  void reset(const QImage &img, const QString &img_id);
  void addBoundingBoxes(const QList<QRectF> &bboxes,
                        const QStringList &labels = QStringList());
  QMap<QString, QList<QRectF>> boundingBoxes();
  void removeSelectedBoundingBoxes();
  void clear();
  QSize imageSize();
  QString imageId();
  void hideBoundingBoxes();
  void showBoundingBoxes();
  void prepareForNewBBox(QString label = QString(), QSizeF bboxSize = QSizeF(),
                         bool what = true);
  bool addingNewBBox() { return m_waitingForNewBbox; }

  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void drawForeground(QPainter *painter, const QRectF &rect) override;

signals:
  void bboxItemToEditor(QGraphicsItem *iten, int reason);
};

#endif // IMAGECANVAS_H
