#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "annimgmanager.h"
#include "bboxitem.h"
#include "expensiveroutines.h"
#include "imagecanvas.h"
#include "imgstringlistmodel.h"
#include <QDir>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_listViewImgNames_activated(const QModelIndex &index);
  void on_lineEditImgFilter_editingFinished();
  void on_addNewBbox_triggered();
  void on_addLabelToFilterList_clicked();
  void on_tbLoadImageFolder_clicked();
  void on_tbLoadAnnFolder_clicked();
  void on_pbLoadImgAnn_clicked();
  void on_removeLabelToFilterList_clicked();
  void on_pbExport_clicked();
  void on_saveLocalChanges_triggered();
  void on_removeBbox_triggered();
  void on_actionShowBboxes_triggered();
  void on_toolButtonZoom100_clicked();

  void displayBBoxInfo(QGraphicsItem *newFocusItem, int reason);
  void selectionChangedOnImageCanvas();



private:
  void setUp();
  void displayImageInfo();

  Ui::MainWindow *ui;
  AnnImgManager m_annImgManager;
  ImageCanvas m_imageCanvas;
  ImgStringListModel m_imageListModel;
  BoundingBoxItem *current_bbox_item_;
  ExpensiveRoutines m_worker;

signals:
  void exportAnnotations(AnnImgManager *info, AnnotationFilter filter,
                         const QString &output_ann_path, int);
};

#endif // MAINWINDOW_H
