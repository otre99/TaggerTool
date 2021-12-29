#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QObject>
#include <QPropertyAnimation>
#include <QUndoStack>

#include "annimgmanager.h"
#include "utils.h"

class ImageCanvas : public QGraphicsScene {
  Q_OBJECT
 private:
  QPixmap m_currentImage;
  QString m_imageId;
  QString m_bboxLabel;
  bool m_waitingForObj;
  bool m_drawObjStarted;
  QPointF m_begPt;
  QPointF m_endPt;
  bool m_needSaveChanges;
  Helper::CustomItemType m_waitingForTypeObj;

 public:
  ImageCanvas(QObject *parent = nullptr);
  void reset(const QImage &img, const QString &img_id);
  void addAnnotations(const Annotations &ann);

  Annotations annotations();
  void clear();
  QSize imageSize();
  QString imageId();
  void hideBoundingBoxes();
  void helperParametersChanged();
  void showBoundingBoxes();
  bool showLabels() const { return m_showLabels; };

  void prepareForNewBBox(QString label = QString());
  void prepareForNewPoint(const QString &label = QString());
  void prepareForNewLine(const QString &label = QString());
  void prepareForNewPolygon(const QString &label = QString());

  bool addingNewObj() { return m_waitingForObj; }
  void showLabels(bool show);

  void drawBackground(QPainter *painter, const QRectF &rect) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
  void drawForeground(QPainter *painter, const QRectF &rect) override;
  void keyPressEvent(QKeyEvent *keyEvent) override;

  bool showGrid() const { return m_showGrid; }

 public slots:
  void setShowGrid(bool show);
  void removeItemCmd(QGraphicsItem *item);

 signals:
  void bboxItemToEditor(QGraphicsItem *iten, int reason);
  void needSaveChanges();
  void deferredRemoveItem(QGraphicsItem *item);

 private:
  bool m_showLabels{true};
  bool m_showGrid{true};
  QPolygonF m_currentPolygon{};
};

#endif  // IMAGECANVAS_H
