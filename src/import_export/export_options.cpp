#include "export_options.h"

#include <QSettings>
#include <QtMath>

namespace {

QString handlingKey(Helper::CustomItemType type) {
  return QStringLiteral("handling/") + ExportOptions::typeName(type);
}

QPolygonF circleToPolygon(const QPointF &center, qreal radius, int segments) {
  QPolygonF poly;
  const int n = qMax(3, segments);
  poly.reserve(n);
  for (int i = 0; i < n; ++i) {
    const double a = (2.0 * M_PI * i) / n;
    poly << QPointF(center.x() + radius * std::cos(a),
                    center.y() + radius * std::sin(a));
  }
  return poly;
}

QRectF squareAround(const QPointF &center, double side) {
  return QRectF(center.x() - side / 2.0, center.y() - side / 2.0, side, side);
}

}  // namespace

QVector<Helper::CustomItemType> ExportOptions::allTypes() {
  return {Helper::kBBox,      Helper::kPolygon, Helper::kLineStrip,
          Helper::kCircle,    Helper::kPoint,   Helper::kLine};
}

QString ExportOptions::typeName(Helper::CustomItemType type) {
  switch (type) {
    case Helper::kBBox:
      return QStringLiteral("Bounding boxes");
    case Helper::kPolygon:
      return QStringLiteral("Polygons");
    case Helper::kLineStrip:
      return QStringLiteral("Line strips");
    case Helper::kCircle:
      return QStringLiteral("Circles");
    case Helper::kPoint:
      return QStringLiteral("Points");
    case Helper::kLine:
      return QStringLiteral("Lines");
    default:
      return QStringLiteral("Unknown");
  }
}

ExportOptions::Handling ExportOptions::handlingFor(
    Helper::CustomItemType type) const {
  return handling.value(static_cast<int>(type), Handling::Native);
}

QString ExportOptions::exportLabel(const QString &label) const {
  const QString mapped = labelMap.value(label);
  return mapped.isEmpty() ? label : mapped;
}

bool ExportOptions::acceptsLabel(const QString &label) const {
  if (!filterLabels) return true;
  return includedLabels.contains(label);
}

void ExportOptions::save(const QString &group) const {
  QSettings st;
  st.beginGroup(QStringLiteral("export/") + group);

  for (auto type : allTypes()) {
    st.setValue(handlingKey(type), static_cast<int>(handlingFor(type)));
  }
  st.setValue("circleSegments", circleSegments);
  st.setValue("pointBoxSize", pointBoxSize);

  st.setValue("filterLabels", filterLabels);
  st.setValue("includedLabels", QStringList(includedLabels.begin(),
                                            includedLabels.end()));
  QStringList mapping;
  for (auto it = labelMap.cbegin(); it != labelMap.cend(); ++it) {
    mapping << it.key() << it.value();
  }
  st.setValue("labelMap", mapping);

  st.setValue("minWidth", minWidth);
  st.setValue("minHeight", minHeight);
  st.setValue("minArea", minArea);
  st.setValue("clipToImage", clipToImage);
  st.setValue("dropOutside", dropOutside);
  st.setValue("skipOccluded", skipOccluded);
  st.setValue("skipTruncated", skipTruncated);
  st.setValue("skipCrowded", skipCrowded);
  st.setValue("skipEmptyImages", skipEmptyImages);

  st.setValue("yoloLayout", static_cast<int>(yoloLayout));
  st.setValue("imageMode", static_cast<int>(imageMode));
  st.setValue("writeDataYaml", writeDataYaml);
  st.setValue("splitTrainVal", splitTrainVal);
  st.setValue("valRatio", valRatio);
  st.setValue("splitSeed", splitSeed);
  st.setValue("classOrder", static_cast<int>(classOrder));

  st.setValue("prettyJson", prettyJson);
  st.setValue("cocoIncludeEmptyImages", cocoIncludeEmptyImages);
  st.setValue("cocoIscrowdFromFlag", cocoIscrowdFromFlag);
  st.endGroup();
}

void ExportOptions::load(const QString &group) {
  QSettings st;
  st.beginGroup(QStringLiteral("export/") + group);

  for (auto type : allTypes()) {
    const QVariant v = st.value(handlingKey(type));
    if (v.isValid()) {
      handling[static_cast<int>(type)] =
          static_cast<Handling>(v.toInt());
    }
  }
  circleSegments = st.value("circleSegments", circleSegments).toInt();
  pointBoxSize = st.value("pointBoxSize", pointBoxSize).toDouble();

  filterLabels = st.value("filterLabels", filterLabels).toBool();
  const QStringList inc = st.value("includedLabels").toStringList();
  includedLabels = QSet<QString>(inc.begin(), inc.end());
  const QStringList mapping = st.value("labelMap").toStringList();
  labelMap.clear();
  for (int i = 0; i + 1 < mapping.size(); i += 2) {
    labelMap.insert(mapping[i], mapping[i + 1]);
  }

  minWidth = st.value("minWidth", minWidth).toDouble();
  minHeight = st.value("minHeight", minHeight).toDouble();
  minArea = st.value("minArea", minArea).toDouble();
  clipToImage = st.value("clipToImage", clipToImage).toBool();
  dropOutside = st.value("dropOutside", dropOutside).toBool();
  skipOccluded = st.value("skipOccluded", skipOccluded).toBool();
  skipTruncated = st.value("skipTruncated", skipTruncated).toBool();
  skipCrowded = st.value("skipCrowded", skipCrowded).toBool();
  skipEmptyImages = st.value("skipEmptyImages", skipEmptyImages).toBool();

  yoloLayout = static_cast<YoloLayout>(
      st.value("yoloLayout", static_cast<int>(yoloLayout)).toInt());
  imageMode = static_cast<ImageMode>(
      st.value("imageMode", static_cast<int>(imageMode)).toInt());
  writeDataYaml = st.value("writeDataYaml", writeDataYaml).toBool();
  splitTrainVal = st.value("splitTrainVal", splitTrainVal).toBool();
  valRatio = st.value("valRatio", valRatio).toDouble();
  splitSeed = st.value("splitSeed", splitSeed).toInt();
  classOrder = static_cast<ClassOrder>(
      st.value("classOrder", static_cast<int>(classOrder)).toInt());

  prettyJson = st.value("prettyJson", prettyJson).toBool();
  cocoIncludeEmptyImages =
      st.value("cocoIncludeEmptyImages", cocoIncludeEmptyImages).toBool();
  cocoIscrowdFromFlag =
      st.value("cocoIscrowdFromFlag", cocoIscrowdFromFlag).toBool();
  st.endGroup();
}

QRectF polygonBoundingBox(const QPolygonF &poly) {
  if (poly.isEmpty()) return {};
  return poly.boundingRect();
}

int annotationCount(const Annotations &ann) {
  return static_cast<int>(ann.bboxes.size() + ann.polygons.size() +
                          ann.line_strips.size() + ann.circles.size() +
                          ann.points.size() + ann.lines.size());
}

namespace {

/// Shared per-annotation pipeline: filtering, clipping and bookkeeping.
class ShapeSink {
 public:
  ShapeSink(const ExportOptions &opt, int w, int h, Annotations *out,
            ExportStats *stats)
      : m_opt(opt), m_w(w), m_h(h), m_out(out), m_stats(stats) {
    m_hasImageRect = (w > 0 && h > 0);
    m_imageRect = QRectF(0, 0, w, h);
  }

  void drop() {
    if (m_stats) ++m_stats->dropped;
  }

  /// @return the exported class name, or a null string when the annotation
  /// must be dropped. Bookkeeping for the rejection is already done.
  QString accept(const QString &label) {
    if (!m_opt.acceptsLabel(label)) {
      drop();
      return {};
    }
    const QString mapped = m_opt.exportLabel(label);
    if (mapped.isEmpty()) {
      drop();
      return {};
    }
    return mapped;
  }

  /// Clip, measure and record a box. @return false when it was dropped.
  bool addBBox(QRectF rect, const QString &cls, const QString &dsc,
               bool occluded, bool truncated, bool crowded) {
    rect = rect.normalized();
    if (m_hasImageRect) {
      if (m_opt.dropOutside && !rect.intersects(m_imageRect)) {
        drop();
        return false;
      }
      if (m_opt.clipToImage) {
        rect &= m_imageRect;
      }
    }
    if (!passesSize(rect.width(), rect.height(),
                    rect.width() * rect.height())) {
      drop();
      return false;
    }
    m_out->bboxes.emplaceBack(rect, cls, dsc, occluded, truncated, crowded);
    count(cls);
    if (m_stats) ++m_stats->boxes;
    return true;
  }

  /// Clip, measure and record a polygon. @return false when it was dropped.
  bool addPolygon(QPolygonF poly, const QString &cls, const QString &dsc,
                  bool lineStrip) {
    if (poly.size() < (lineStrip ? 2 : 3)) {
      drop();
      return false;
    }
    if (m_hasImageRect) {
      const QRectF br = poly.boundingRect();
      if (m_opt.dropOutside && !br.intersects(m_imageRect)) {
        drop();
        return false;
      }
      if (m_opt.clipToImage && !m_imageRect.contains(br)) {
        poly = clipPolygon(poly, lineStrip);
        if (poly.size() < (lineStrip ? 2 : 3)) {
          drop();
          return false;
        }
      }
    }
    const QRectF br = poly.boundingRect();
    const double area =
        lineStrip ? br.width() * br.height() : Helper::polygonArea(poly);
    if (!passesSize(br.width(), br.height(), area)) {
      drop();
      return false;
    }
    if (lineStrip) {
      m_out->line_strips.emplaceBack(poly, cls, dsc);
    } else {
      m_out->polygons.emplaceBack(poly, cls, dsc);
    }
    count(cls);
    if (m_stats) ++m_stats->polygons;
    return true;
  }

  void addCircle(const QPointF &center, qreal radius, const QString &cls,
                 const QString &dsc) {
    if (m_hasImageRect && m_opt.dropOutside) {
      const QRectF br(center.x() - radius, center.y() - radius, 2 * radius,
                      2 * radius);
      if (!br.intersects(m_imageRect)) {
        drop();
        return;
      }
    }
    if (!passesSize(2 * radius, 2 * radius, M_PI * radius * radius)) {
      drop();
      return;
    }
    m_out->circles.emplaceBack(center, radius, cls, dsc);
    count(cls);
  }

  void addPoint(const QPointF &pt, const QString &cls, const QString &dsc) {
    if (m_hasImageRect && m_opt.dropOutside && !m_imageRect.contains(pt)) {
      drop();
      return;
    }
    m_out->points.emplaceBack(pt, cls, dsc);
    count(cls);
  }

  void addLine(const QPointF &p1, const QPointF &p2, const QString &cls,
               const QString &dsc) {
    if (m_hasImageRect && m_opt.dropOutside) {
      const QRectF br = QRectF(p1, p2).normalized();
      if (!br.intersects(m_imageRect)) {
        drop();
        return;
      }
    }
    m_out->lines.emplaceBack(p1, p2, cls, dsc);
    count(cls);
  }

 private:
  bool passesSize(double w, double h, double area) const {
    if (w < m_opt.minWidth || h < m_opt.minHeight) return false;
    if (area < m_opt.minArea) return false;
    return true;
  }

  QPolygonF clipPolygon(const QPolygonF &poly, bool lineStrip) const {
    if (lineStrip) {
      // An open strip has no interior to intersect, so its vertices are simply
      // pulled back onto the image.
      QPolygonF clamped;
      clamped.reserve(poly.size());
      for (const QPointF &pt : poly) {
        clamped << QPointF(qBound(0.0, pt.x(), static_cast<double>(m_w)),
                           qBound(0.0, pt.y(), static_cast<double>(m_h)));
      }
      return clamped;
    }
    const QPolygonF clipped = poly.intersected(QPolygonF(m_imageRect));
    return clipped.isEmpty() ? QPolygonF() : clipped;
  }

  void count(const QString &cls) {
    if (!m_stats) return;
    ++m_stats->kept;
    ++m_stats->perClass[cls];
  }

  const ExportOptions &m_opt;
  int m_w, m_h;
  bool m_hasImageRect{false};
  QRectF m_imageRect;
  Annotations *m_out;
  ExportStats *m_stats;
};

}  // namespace

Annotations applyExportOptions(const Annotations &in, const ExportOptions &opt,
                               int imgW, int imgH, ExportStats *stats) {
  using Handling = ExportOptions::Handling;

  Annotations out;
  out.image_name = in.image_name;
  out.label = in.label;
  out.description = in.description;
  out.img_w = imgW;
  out.img_h = imgH;

  ShapeSink sink(opt, imgW, imgH, &out, stats);

  // ---- bounding boxes ----------------------------------------------------
  const Handling hBBox = opt.handlingFor(Helper::kBBox);
  for (const auto &b : in.bboxes) {
    if (hBBox == Handling::Skip) {
      sink.drop();
      continue;
    }
    if ((opt.skipOccluded && b.getOccluded()) ||
        (opt.skipTruncated && b.getTruncated()) ||
        (opt.skipCrowded && b.getCrowded())) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(b.getLabel());
    if (cls.isNull()) continue;

    const QRectF rect = QRectF(b.pt1(), b.pt2()).normalized();
    if (hBBox == Handling::AsPolygon) {
      sink.addPolygon(QPolygonF(rect), cls, b.getDescription(), false);
    } else {
      sink.addBBox(rect, cls, b.getDescription(), b.getOccluded(),
                   b.getTruncated(), b.getCrowded());
    }
  }

  // ---- polygons ----------------------------------------------------------
  const Handling hPoly = opt.handlingFor(Helper::kPolygon);
  for (const auto &p : in.polygons) {
    if (hPoly == Handling::Skip) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(p.getLabel());
    if (cls.isNull()) continue;

    const QPolygonF poly = p.getPolygon();
    if (hPoly == Handling::AsBBox) {
      sink.addBBox(polygonBoundingBox(poly), cls, p.getDescription(), false,
                   false, false);
    } else {
      sink.addPolygon(poly, cls, p.getDescription(), false);
    }
  }

  // ---- line strips -------------------------------------------------------
  const Handling hStrip = opt.handlingFor(Helper::kLineStrip);
  for (const auto &l : in.line_strips) {
    if (hStrip == Handling::Skip) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(l.getLabel());
    if (cls.isNull()) continue;

    const QPolygonF poly = l.getPolygon();
    if (hStrip == Handling::AsBBox) {
      sink.addBBox(polygonBoundingBox(poly), cls, l.getDescription(), false,
                   false, false);
    } else if (hStrip == Handling::AsPolygon) {
      sink.addPolygon(poly, cls, l.getDescription(), false);
    } else {
      sink.addPolygon(poly, cls, l.getDescription(), true);
    }
  }

  // ---- circles -----------------------------------------------------------
  const Handling hCircle = opt.handlingFor(Helper::kCircle);
  for (const auto &c : in.circles) {
    if (hCircle == Handling::Skip) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(c.getLabel());
    if (cls.isNull()) continue;

    const QPointF center = c.center();
    const qreal r = c.radius();
    switch (hCircle) {
      case Handling::AsBBox:
        sink.addBBox(QRectF(center.x() - r, center.y() - r, 2 * r, 2 * r), cls,
                     c.getDescription(), false, false, false);
        break;
      case Handling::AsPolygon:
        sink.addPolygon(circleToPolygon(center, r, opt.circleSegments), cls,
                        c.getDescription(), false);
        break;
      default:
        sink.addCircle(center, r, cls, c.getDescription());
        break;
    }
  }

  // ---- points ------------------------------------------------------------
  const Handling hPoint = opt.handlingFor(Helper::kPoint);
  for (const auto &p : in.points) {
    if (hPoint == Handling::Skip) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(p.getLabel());
    if (cls.isNull()) continue;

    const QPointF pt = p.pt();
    switch (hPoint) {
      case Handling::AsBBox:
        sink.addBBox(squareAround(pt, opt.pointBoxSize), cls,
                     p.getDescription(), false, false, false);
        break;
      case Handling::AsPolygon:
        sink.addPolygon(QPolygonF(squareAround(pt, opt.pointBoxSize)), cls,
                        p.getDescription(), false);
        break;
      default:
        sink.addPoint(pt, cls, p.getDescription());
        break;
    }
  }

  // ---- lines -------------------------------------------------------------
  const Handling hLine = opt.handlingFor(Helper::kLine);
  for (const auto &l : in.lines) {
    if (hLine == Handling::Skip) {
      sink.drop();
      continue;
    }
    const QString cls = sink.accept(l.getLabel());
    if (cls.isNull()) continue;

    const QPointF p1 = l.pt1();
    const QPointF p2 = l.pt2();
    switch (hLine) {
      case Handling::AsBBox:
        sink.addBBox(QRectF(p1, p2).normalized(), cls, l.getDescription(),
                     false, false, false);
        break;
      case Handling::AsPolygon: {
        QPolygonF poly;
        poly << p1 << p2;
        sink.addPolygon(poly, cls, l.getDescription(), true);
        break;
      }
      default:
        sink.addLine(p1, p2, cls, l.getDescription());
        break;
    }
  }

  if (stats) {
    if (annotationCount(out) == 0) {
      ++stats->emptyImages;
      if (!opt.skipEmptyImages) ++stats->images;
    } else {
      ++stats->images;
    }
  }
  return out;
}
