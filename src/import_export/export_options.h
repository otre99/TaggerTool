#ifndef EXPORT_OPTIONS_H
#define EXPORT_OPTIONS_H
#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>

#include "annimgmanager.h"
#include "utils.h"

/**
 * @brief User controlled settings shared by every exporter.
 *
 * The exporters never look at the raw project annotations. They call
 * applyExportOptions() first, which resolves this configuration into a plain
 * Annotations value holding exactly what should be written out. That keeps the
 * conversion rules (polygon -> bounding box and friends), the filters and the
 * label mapping in one place instead of once per format.
 */
struct ExportOptions {
  /// What to do with one annotation type.
  enum class Handling {
    Skip,       ///< Do not export annotations of this type at all.
    Native,     ///< Export unchanged, in the type's own representation.
    AsBBox,     ///< Convert to the annotation's bounding box.
    AsPolygon,  ///< Convert to a closed polygon.
  };

  enum class Format { Native, Coco, Yolo };
  enum class ClassOrder { Alphabetical, FirstSeen };
  enum class YoloLayout { LabelsOnly, FullDataset };
  enum class ImageMode { None, Copy, Symlink };

  Format format{Format::Native};
  QString outputPath;

  // ---- per annotation type handling -------------------------------------
  /// Keyed by Helper::CustomItemType. Missing entries fall back to Native.
  QHash<int, Handling> handling;
  /// Number of segments used when a circle is turned into a polygon.
  int circleSegments{16};
  /// Side length, in pixels, of the box generated for a point annotation.
  double pointBoxSize{8.0};

  // ---- label selection and renaming --------------------------------------
  bool filterLabels{false};
  QSet<QString> includedLabels;
  /// Original label -> exported class name. Lets several labels merge into one
  /// class without touching the project.
  QHash<QString, QString> labelMap;

  // ---- geometry filters ---------------------------------------------------
  double minWidth{0.0};
  double minHeight{0.0};
  double minArea{0.0};
  bool clipToImage{true};
  bool dropOutside{true};
  bool skipOccluded{false};
  bool skipTruncated{false};
  bool skipCrowded{false};
  bool skipEmptyImages{false};

  // ---- YOLO ---------------------------------------------------------------
  YoloLayout yoloLayout{YoloLayout::LabelsOnly};
  ImageMode imageMode{ImageMode::None};
  bool writeDataYaml{true};
  bool splitTrainVal{false};
  double valRatio{0.2};
  int splitSeed{0};
  ClassOrder classOrder{ClassOrder::Alphabetical};

  // ---- JSON based formats -------------------------------------------------
  bool prettyJson{true};
  bool cocoIncludeEmptyImages{true};
  bool cocoIscrowdFromFlag{true};

  Handling handlingFor(Helper::CustomItemType type) const;
  /// Exported class name for @p label, after applying labelMap.
  QString exportLabel(const QString &label) const;
  /// Whether @p label passes the label filter.
  bool acceptsLabel(const QString &label) const;

  /// Persist to / restore from QSettings under @p group, so that the next
  /// export of the same format starts from the previous choices.
  void save(const QString &group) const;
  void load(const QString &group);

  /// Every annotation type, in the order the UI presents them.
  static QVector<Helper::CustomItemType> allTypes();
  static QString typeName(Helper::CustomItemType type);
};

/// Counts produced by a dry run, used for the dialog's preview.
struct ExportStats {
  int images{0};          ///< Images that would appear in the output.
  int emptyImages{0};     ///< Images left without a single annotation.
  int kept{0};            ///< Annotations written.
  int dropped{0};         ///< Annotations removed by a filter or by Skip.
  int boxes{0};           ///< Of those kept, how many are bounding boxes.
  int polygons{0};        ///< Of those kept, how many are polygons/strips.
  QMap<QString, int> perClass;  ///< Exported class name -> count.
};

/**
 * @brief Resolve @p opt against @p in and return what should be exported.
 *
 * Conversions are applied first (so a polygon asked to export AsBBox lands in
 * the returned bboxes), then the label filter, the renaming and the geometry
 * filters. @p stats, when given, accumulates across calls.
 */
Annotations applyExportOptions(const Annotations &in, const ExportOptions &opt,
                               int imgW, int imgH,
                               ExportStats *stats = nullptr);

/// Total number of annotations held by @p ann, whatever their type.
int annotationCount(const Annotations &ann);

/// Bounding box of a polygon, or a null rect when it has no points.
QRectF polygonBoundingBox(const QPolygonF &poly);

#endif  // EXPORT_OPTIONS_H
