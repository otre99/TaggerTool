#include "expensiveroutines.h"
#include "utils.h"
#include <QDebug>

ExpensiveRoutines::ExpensiveRoutines() {
  m_thread = new QThread();
  moveToThread(m_thread);
  m_thread->start();
}

ExpensiveRoutines::~ExpensiveRoutines() {
  m_thread->exit();
  m_thread->wait();
  delete m_thread;
}

void ExpensiveRoutines::upateProgress(double p) {
  if (static_cast<int>(p / m_maxIterations) > m_p100) {
    m_p100 = static_cast<int>(p / m_maxIterations);
    emit progress(m_p100);
  }
}

void ExpensiveRoutines::resetProgress(int n) {
  m_maxIterations = n * 0.01;
  m_p100 = 0;
}

void ExpensiveRoutines::exportAnnotations(AnnImgManager *info,
                                          AnnotationFilter filter,
                                          const QString &output_ann_path,
                                          int type_fmt) {
  emit jobStarted(true);
  const QStringList lst = info->imageIds();
  resetProgress(lst.count());

  size_t index_p = 0;
  if (type_fmt == 2) {
    QFile file(output_ann_path);
    file.open(QFile::WriteOnly);
    QTextStream out(&file);
    for (const auto &img_id : lst) {
      auto ann = filter.filter(info->annotations(img_id));
      QSize isize = info->imageSize(img_id);
      out << img_id << "," << isize.width() << "," << isize.height();
      const QStringList labels = ann.keys();
      for (const auto &label : labels) {
        for (const auto &bbox : ann[label]) {
          out << "," << label << "," << bbox.left() << "," << bbox.top() << ","
              << bbox.right() << "," << bbox.bottom();
        }
      }
      out << "\n";
      upateProgress(++index_p);
    }
  } else if (type_fmt == 0) {
    QDir ann_dir(output_ann_path);
    for (const auto &img_id : lst) {
      auto ann = filter.filter(info->annotations(img_id));

      if (!ann.isEmpty()) {

        QFileInfo finfo(img_id);
        QFile ofile(ann_dir.filePath(finfo.baseName() + ".csv"));
        ofile.open(QFile::WriteOnly);
        QTextStream out(&ofile);
        QSize isize = info->imageSize(img_id);
        const QStringList labels = ann.keys();
        for (const auto &label : labels) {
          for (const auto &bbox : ann[label]) {
            out << img_id << "," << isize.width() << "," << isize.height()
                << "," << label << "," << bbox.left() << "," << bbox.top()
                << "," << bbox.right() << "," << bbox.bottom() << "\n";
          }
        }
        out.flush();
        ofile.close();
      }
      upateProgress(++index_p);
    }
  } else if (type_fmt == 1) {
    QDir ann_dir(output_ann_path);
    for (const auto &img_id : lst) {
      QFileInfo finfo(img_id);
      auto ann = filter.filter(info->annotations(img_id));
      if (!ann.isEmpty()) {
        Helper::saveXMLSingleFile(ann_dir.filePath(finfo.baseName() + ".xml"),
                                  ann, img_id, info->imageSize(img_id));
      }
      upateProgress(++index_p);
    }
  }
  jobFinished(false);
}
