#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDir>
#include <QMainWindow>
#include <QTimer>

#include "annimgmanager.h"
#include "heavytaskthread.h"
#include "imagecanvas.h"
#include "imgstringlistmodel.h"

class QLabel;

namespace Ui {
class MainWindow;
}

class QComboBox;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void on_addNewBbox_triggered();
  void on_pbLoadImgAnn_clicked();
  void on_saveLocalChanges_triggered();
  void on_actionShowBboxes_triggered();
  void selectionChangedOnImageCanvas();
  void on_actionShow_Hide_Labels_triggered();
  void on_actionNew_Point_triggered();
  void on_NeedSaveChangeUndo(bool enable);
  void on_NeedSaveChange();
  void on_listViewImgNames_clicked(const QModelIndex &index);
  void on_actionAdd_New_Line_triggered();
  void on_actionNext_triggered();
  void on_actionPrevious_triggered();
  void on_actionAdd_New_Polygon_triggered();
  void on_actionzoom100_triggered();
  void on_actionGrid_triggered(bool checked);
  void on_timeout();
  void on_tBAdd_clicked();
  void on_tBRemove_clicked();
  void on_pTextImgDescription_textChanged();
  void updateSettings();
  void on_doubleSpinBoxPtRadius_valueChanged(double arg1);
  void on_spinBoxLabelPixSize_valueChanged(int arg1);
  void on_actionFit_Into_View_triggered();
  void on_comboBoxImgLabel_currentTextChanged(const QString &arg1);
  void on_toolButtonAddItemLabels_clicked();
  void on_toolButtonAddTags_clicked();
  void on_toolButtonAddImgLabels_clicked();
  void on_actionLoad_project_triggered();
  void on_actionSave_project_triggered();
  void loadImagesAndAnnotations(const QString &annImg,
                                const QString &annFolder);
  void on_actionAdd_New_LineStrip_triggered();
  void on_actionExport_Annotations_triggered();

  void on_actionAdd_Circle_Item_triggered();

  private:
  void setUp();
  void displayImageInfo();
  void addNewUniqueItem(QComboBox *cbox, const QString &label, bool selected);
  QStringList getLabelsFromComboBox(QComboBox *cbox);

  QTimer m_timer;
  QGraphicsItem *m_currentItem;
  Ui::MainWindow *ui;
  AnnImgManager m_annImgManager;
  ImageCanvas m_imageCanvas;
  ImgStringListModel m_imageListModel;
  QModelIndex m_current_index{};
  bool m_needToSaveNotUndo;
  HeavyTaskThread m_heavyTaskThread;
  QLabel *m_displayLabel;
};

#endif // MAINWINDOW_H
