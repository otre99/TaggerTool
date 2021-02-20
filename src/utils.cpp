#include "utils.h"
#include <QDebug>
#include <QDomDocument>
#include <QRectF>
#include <QTextStream>
#include <QXmlStreamWriter>

QMap<QString, QList<QRectF>> Helper::readCSVSingleFile(const QString &path) {
  QMap<QString, QList<QRectF>> result;
  QFile file(path);
  file.open(QIODevice::ReadOnly);
  if (!file.exists())
    return result;
  QTextStream in(&file);

  while (!in.atEnd()) {
    const QString line = in.readLine();
    const QStringList items = line.split(",");
    if (items.isEmpty() || items.size() < 2)
      break;
    const QString image_id = items[0].trimmed();
    const int w = items[1].toInt();
    const int h = items[2].toInt();
    const QString cls = items[3].trimmed();
    const double x1 = items[4].toDouble();
    const double y1 = items[5].toDouble();
    const double x2 = items[6].toDouble();
    const double y2 = items[7].toDouble();
    result[cls].append(QRectF(QPointF(x1, y1), QPointF(x2, y2)));
  }
  return result;
}

QMap<QString, QList<QRectF>> Helper::readXMLingleFile(const QString &path) {
  QMap<QString, QList<QRectF>> ann;
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) {
    qDebug() << "Error while loading file";
    return ann;
  }
  QDomDocument xmlBOM;
  xmlBOM.setContent(&f);
  f.close();

  QDomElement root = xmlBOM.documentElement();
  QDomNodeList bboxes = root.elementsByTagName("object");
  for (int i = 0; i < bboxes.size(); ++i) {
    auto element = bboxes.item(i).toElement();
    auto name = element.elementsByTagName("name").item(0).toElement();
    QString label = name.text();
    auto bndbox = element.elementsByTagName("bndbox").item(0).toElement();
    auto e = bndbox.elementsByTagName("xmin").item(0).toElement();
    const double xmin = e.text().toDouble();
    e = bndbox.elementsByTagName("ymin").item(0).toElement();
    const double ymin = e.text().toDouble();
    e = bndbox.elementsByTagName("xmax").item(0).toElement();
    const double xmax = e.text().toDouble();
    e = bndbox.elementsByTagName("ymax").item(0).toElement();
    const double ymax = e.text().toDouble();
    ann[label] << QRectF(QPointF(xmin, ymin), QPointF(xmax, ymax));
  }
  return ann;
}

void Helper::saveCSVSingleFile(const QString &path,
                               const QMap<QString, QList<QRectF>> &bboxes,
                               const QString &img_id, const QSize &img_size) {
  QFile file(path);
  file.open(QIODevice::WriteOnly);
  QTextStream in(&file);

  const QStringList keys = bboxes.keys();
  for (const auto &key : keys) {
    for (const auto &rect : bboxes[key]) {
      in << img_id << ',' << img_size.width() << ',' << img_size.height() << ','
         << key << ',' << rect.left() << ',' << rect.top() << ','
         << rect.right() << ',' << rect.bottom() << '\n';
    }
  }
  in.flush();
}

void Helper::saveXMLSingleFile(const QString &path,
                               const QMap<QString, QList<QRectF>> &bboxes,
                               const QString &img_id, const QSize &img_size) {
  QFile xmlfile(path);
  if (!xmlfile.open(QFile::WriteOnly)) {
    qDebug() << "Error writing annotation file " << path;
    return;
  }
  QXmlStreamWriter xmlstream(&xmlfile);
  xmlstream.setAutoFormatting(true);

  // TODO (otre99): I need to chenge some stuff here
  xmlstream.writeStartElement("annotation");
  xmlstream.writeTextElement("folder", "Mixed_Cleaned");
  xmlstream.writeTextElement("filename", img_id);

  xmlstream.writeStartElement("source");
  xmlstream.writeTextElement("database", "SimpleQtBBox");
  xmlstream.writeTextElement("annotation", "SimpleQtBBox exporter");
  xmlstream.writeTextElement("image", "SimpleQtBBox exporter");
  xmlstream.writeEndElement(); // end source

  xmlstream.writeStartElement("size");
  xmlstream.writeTextElement("width", QString::number(img_size.width()));
  xmlstream.writeTextElement("height", QString::number(img_size.height()));
  xmlstream.writeTextElement("depth", QString::number(3));
  xmlstream.writeEndElement(); // end size

  const QStringList labels = bboxes.keys();
  for (const auto &label : labels) {
    for (const auto &box : bboxes[label]) {
      xmlstream.writeStartElement("object");
      xmlstream.writeTextElement("name", label);
      xmlstream.writeStartElement("bndbox");
      xmlstream.writeTextElement("xmin",
                                 QString::number(static_cast<int>(box.left())));
      xmlstream.writeTextElement("ymin",
                                 QString::number(static_cast<int>(box.top())));
      xmlstream.writeTextElement(
          "xmax", QString::number(static_cast<int>(box.right())));
      xmlstream.writeTextElement(
          "ymax", QString::number(static_cast<int>(box.bottom())));
      xmlstream.writeEndElement(); // end bndbox
      xmlstream.writeEndElement(); // end object
    }
  }
  xmlstream.writeEndElement(); // end annotation
  xmlstream.writeEndDocument();
  xmlfile.close();
}

QRectF Helper::buildRectFromTwoPoints(const QPointF &p1, const QPointF &p2) {
  double rw = qAbs(p2.x() - p1.x());
  double rh = qAbs(p2.y() - p1.y());
  QPointF topleft(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
  return {topleft, QSizeF(rw, rh)};
}

QColor Helper::colorFromLabel(const QString &label) {
  const QStringList lst = QColor::colorNames();
  int index = std::hash<std::string>()(label.toStdString()) % lst.count();
  return {lst[index]};
}
