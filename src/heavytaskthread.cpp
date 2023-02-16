#include "heavytaskthread.h"

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
    default:
      break;
    }
  }
}

void HeavyTaskThread::startTask(TaskName taskname) {
  m_currentTask = taskname;
  m_waitCond.notify_all();
}

void HeavyTaskThread::KillTaskAndWait() {
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
  collectedAnnotations.clear();
  m_ncount = img_ids.size();
  size_t index = 0;
  for (auto &id : img_ids) {
    const auto ann = annImgManager->annotations(id);
    for (const auto &box : ann.bboxes) {
      uniqueLabels.insert(box.getLabel());
    }

    collectedAnnotations.push_back(ann);
    updateProgress(index++);
    if (m_abort_) {
      collectedAnnotations.clear();
      uniqueLabels.clear();
      return false;
    }
  }
  return true;
}

void HeavyTaskThread::updateProgress(size_t index) {
  const size_t K = std::max(m_ncount / 100, size_t{1});
  if (index % K == 0) {
    const int p = static_cast<int>((100.0 * index) / m_ncount);
    // qDebug() << "Progress" << p;
    emit progress(p);
  }
}
