#include "heavytaskthread.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMap>
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
    default:
      break;
    }
  }
}

void HeavyTaskThread::startTask(TaskName taskname) {
  m_currentTask = taskname;
  m_waitCond.notify_all();
}

bool HeavyTaskThread::isTaskRunning() const {
    return m_taskIsRunning;
}

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

  int annId=0;
  int imgId=0;
  int labelId=0;
  QJsonArray annList{};
  QJsonArray imgList{};
  QJsonArray catList{};

  QMap<QString, int> label2id;
  int lbIndex=0;
  for (const auto &lb : uniqueLabels){
      label2id[lb]=lbIndex;
      QJsonObject jsonCat;
      jsonCat[QString("id")] = lbIndex++;
      jsonCat[QString("name")] = lb;
      jsonCat["supercategory"]="";
      catList.push_back(jsonCat);
  }

  for (auto &id : img_ids) {
    const Annotations &ann = annImgManager->annotations(id);

    // bboxes
    for (const auto &box : ann.bboxes) {
        if ( uniqueLabels.contains(box.getLabel()) ){

            QJsonObject jsonBox;
            jsonBox[QString("id")] = annId++;
            jsonBox[QString("image_id")] = imgId;
            jsonBox[QString("category_id")] = label2id[box.getLabel()];

            const auto p1 = box.pt1();
            const auto p2 = box.pt2();
            const double x = p1.x();
            const double y = p1.y();
            const double w = p2.x()-x;
            const double h = p2.y()-y;
            jsonBox[QString("bbox")] = QJsonArray{x,y,w,h};
            jsonBox[QString("area")] = w*h;
            jsonBox[QString("iscrowd")] = box.getCrowded();
            //jsonBox[QString("segmentation")] = QJsonArray{};

            annList.push_back(jsonBox);
        }
        ++imgId;
    }
    updateProgress(index++);
    if (m_abort_) {
      uniqueLabels.clear();
      return false;
    }
  }

  QJsonObject root;
  root[QString("annotations")] = annList;
  root[QString("images")] = imgList;
  root[QString("categories")] = catList;

  const QByteArray out = QJsonDocument(root).toJson();
  QFile ofile(outputDirOrFile);
  if (!ofile.open(QFile::WriteOnly)) {
    qDebug() << "Failed exporting annotations to " << outputDirOrFile << " file!";
    return false;
  }
  ofile.write(out);
  return true;
}



void HeavyTaskThread::updateProgress(size_t index) {
  const size_t K = std::max(m_ncount / 100, size_t{1});
  if (index % K == 0) {
    const int p = static_cast<int>((100.0 * index) / m_ncount);
    emit progress(p);
  }
}
