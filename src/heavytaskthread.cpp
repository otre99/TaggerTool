#include "heavytaskthread.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
#include <QXmlStreamWriter>

namespace {
bool saveAnnToPascalFormat(const Annotations &ann, const QString &outputFile) {
  QFile ofile(outputFile);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed exporting annotation to " << outputFile << " file!";
    return false;
  }
  QXmlStreamWriter xmlOut(&ofile);
  xmlOut.setAutoFormatting(true);

  xmlOut.writeStartDocument();

  xmlOut.writeStartElement("annotation");

  // filename, folderm segmented
  xmlOut.writeTextElement("filename", ann.image_name);
  xmlOut.writeTextElement("folder", "");
  xmlOut.writeTextElement("segmented", "0");

  // size
  xmlOut.writeStartElement("size");
  xmlOut.writeTextElement("depth", "3");
  xmlOut.writeTextElement("height", QString("%1").arg(ann.img_h));
  xmlOut.writeTextElement("width", QString("%1").arg(ann.img_w));
  xmlOut.writeEndElement();

  for (auto &&box : ann.bboxes) {
    xmlOut.writeStartElement("object");

    // name, difficult, occluded, pose, truncated
    xmlOut.writeTextElement("name", box.getLabel());
    xmlOut.writeTextElement("difficult", box.getCrowded() ? "1" : "0");
    xmlOut.writeTextElement("occluded", box.getOccluded() ? "1" : "0");
    xmlOut.writeTextElement("pose", "Unspecified");
    xmlOut.writeTextElement("truncated", box.getTruncated() ? "1" : "0");

    // bndbox
    xmlOut.writeStartElement("bndbox");
    const auto p1 = box.pt1();
    const auto p2 = box.pt2();
    xmlOut.writeTextElement("xmin", QString("%1").arg(p1.x()));
    xmlOut.writeTextElement("ymin", QString("%1").arg(p1.y()));
    xmlOut.writeTextElement("xmax", QString("%1").arg(p2.x()));
    xmlOut.writeTextElement("ymax", QString("%1").arg(p2.y()));
    xmlOut.writeEndElement();

    xmlOut.writeEndElement();
  }
  xmlOut.writeEndElement();
  xmlOut.writeEndDocument();
  return true;
}
} // namespace

HeavyTaskThread::HeavyTaskThread() {}

HeavyTaskThread::~HeavyTaskThread() {
  m_exit_ = true;
  m_waitCond.notify_all();
  wait();
}

void HeavyTaskThread::run() {
  for (;;) {
    if (m_exit_)
      break;
    m_mutex.lock();
    m_waitCond.wait(&m_mutex);
    m_mutex.unlock();
    if (m_exit_)
      break;

    switch (m_currentTask) {
    case CollectAnnotations: {
      internalStartTask(
          std::bind(&HeavyTaskThread::collectAllAnnotationsTask, this));
      break;
    }
    case ExportCOCOAnnotations: {
      internalStartTask(
          std::bind(&HeavyTaskThread::exportCOCOAnnotationsTask, this));
      break;
    }
    case ExportPASCALAnnotations: {
      internalStartTask(
          std::bind(&HeavyTaskThread::exportPascalAnnotationsTask, this));
      break;
    }
    default:
      break;
    }
  }
}

void HeavyTaskThread::startTask(TaskName taskname) {
  m_currentTask = taskname;
  m_waitCond.notify_all();
}

bool HeavyTaskThread::isTaskRunning() const { return m_taskIsRunning; }

void HeavyTaskThread::killTaskAndWait() {
  m_abort_ = true;
  while (m_taskIsRunning) {
    msleep(1);
  }
}

void HeavyTaskThread::internalStartTask(std::function<bool()> funct) {
  m_taskIsRunning = true;
  m_abort_ = false;
  emit taskStarted();
  bool res = funct();
  m_taskIsRunning = false;
  emit taskFinished(res);
}

void HeavyTaskThread::updateProgress(size_t index) {
  const size_t K = std::max(m_ncount / 100, size_t{1});
  if (index % K == 0) {
    const int p = static_cast<int>((100.0 * index) / m_ncount);
    emit progress(p);
  }
}

bool HeavyTaskThread::collectAllAnnotationsTask() {
  const auto img_ids = annImgManager->imageIds();
  m_ncount = img_ids.size();
  size_t index = 0;
  for (auto &id : img_ids) {
    const auto ann = annImgManager->annotations(id);
    for (const auto &box : ann.bboxes) {
      uniqueLabels.insert(box.getLabel());
    }
    updateProgress(index++);
    if (m_abort_) {
      uniqueLabels.clear();
      return false;
    }
  }
  return true;
}

bool HeavyTaskThread::exportCOCOAnnotationsTask() {
  const auto img_ids = annImgManager->imageIds();
  m_ncount = img_ids.size();
  size_t index = 0;

  int annId = 0;
  int imgId = 0;
  int labelId = 0;
  QJsonArray annList{};
  QJsonArray imgList{};
  QJsonArray catList{};

  QMap<QString, int> label2id;
  int lbIndex = 0;
  for (const auto &lb : qAsConst(uniqueLabels)) {
    label2id[lb] = lbIndex;
    QJsonObject jsonCat;
    jsonCat["id"] = lbIndex++;
    jsonCat["name"] = lb;
    jsonCat["supercategory"] = "";
    catList.push_back(jsonCat);
  }

  for (auto &id : img_ids) {
    const Annotations &ann = annImgManager->annotations(id);
    QJsonObject jsonImg;
    jsonImg["id"] = imgId;
    jsonImg["width"] = ann.img_w;
    jsonImg["height"] = ann.img_h;
    jsonImg["file_name"] = ann.image_name;
    jsonImg["license"] = "";
    jsonImg["flickr_url"] = "";
    jsonImg["coco_url"] = "";
    jsonImg["date_captured"] = "";
    imgList.push_back(jsonImg);

    // bboxes
    for (const auto &box : ann.bboxes) {
      if (uniqueLabels.contains(box.getLabel())) {

        QJsonObject jsonBox;
        jsonBox["id"] = annId++;
        jsonBox["image_id"] = imgId;
        jsonBox["category_id"] = label2id[box.getLabel()];

        const auto p1 = box.pt1();
        const auto p2 = box.pt2();
        const qreal x = p1.x();
        const qreal y = p1.y();
        const qreal w = p2.x() - x;
        const qreal h = p2.y() - y;
        jsonBox["bbox"] = QJsonArray{x, y, w, h};
        jsonBox["area"] = w * h;
        jsonBox["iscrowd"] = box.getCrowded();
        // jsonBox["segmentation"] = QJsonArray{};

        annList.push_back(jsonBox);
      }
    }
    ++imgId;
    updateProgress(index++);
    if (m_abort_) {
      uniqueLabels.clear();
      return false;
    }
  }

  QJsonObject root;
  root["annotations"] = annList;
  root["images"] = imgList;
  root["categories"] = catList;

  const QByteArray out = QJsonDocument(root).toJson(
      useJsonCompactFmt ? QJsonDocument::Compact : QJsonDocument::Indented);
  QFile ofile(outputDirOrFile);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed exporting annotations to " << outputDirOrFile
             << " file!";
    return false;
  }
  ofile.write(out);
  return true;
}

bool HeavyTaskThread::exportPascalAnnotationsTask() {
  const auto img_ids = annImgManager->imageIds();
  m_ncount = img_ids.size();
  size_t index = 0;

  QDir outDir(outputDirOrFile);
  for (auto &id : img_ids) {
    const Annotations &ann = annImgManager->annotations(id);

    // bboxes
    for (const auto &box : ann.bboxes) {
      if (uniqueLabels.contains(box.getLabel())) {
        saveAnnToPascalFormat(ann, outDir.filePath(id + ".xml"));
      }
    }
    updateProgress(index++);
    if (m_abort_) {
      uniqueLabels.clear();
      return false;
    }
  }
  return true;
}
