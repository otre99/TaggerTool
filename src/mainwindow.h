#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCryptographicHash>
#include <QDir>
#include <QMainWindow>

#include "annimgmanager.h"
#include "imagecanvas.h"
#include "imgstringlistmodel.h"
#include "recentprojects.h"
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
  void resizeEvent(QResizeEvent *event) override final;
  void closeEvent(QCloseEvent *ev) override final;

  // from command line
  bool loadImagesAndAnnotations(const QString &annImg, const QString &annFolder,
                                const QString &imageId);

 private slots:
  // Add new annotation items
  // TODO(otre99): these function are not necesary, the corresponding
  // QAction::triggered() signal can be connected with
  // ImageCanvas::prepareForNew* slot directly
  void on_addNewBbox_triggered();               // BBox
  void on_actionNew_Point_triggered();          // Point
  void on_actionAdd_New_Polygon_triggered();    // Polygon
  void on_actionAdd_New_LineStrip_triggered();  // LineStrip
  void on_actionAdd_Circle_Item_triggered();    // Circle
  void on_actionAdd_New_Line_triggered();       // Line

  // Export annotations
  void on_actionExportYolo_triggered();  // Yolo
  void on_actionNative_triggered();      // Native
  void on_actionExportCoco_triggered();  // Coco

  // Load new project
  void on_pbLoadImgAnn_clicked();

  // Save local changes
  void on_saveLocalChanges_triggered();

  // Connected to QUndoStack::cleanChanged signal
  void on_NeedSaveChangeUndo(bool clean);

  // For other changes not managed by QUndoStack
  void on_NeedSaveChange();

  // when user changed image/label:
  // TODO (otre99): remove these functions and consider these connection
  // instead:
  // textChanged() -> on_NeedSaveChange()
  // currentTextChanged() -> on_NeedSaveChange()
  void on_pTextImgDescription_textChanged();
  void on_comboBoxImgLabel_currentTextChanged(const QString &arg1);

  // This functions are basically designed to update the font size displayed on
  // the image canvas. It's a rather inelegant function that I need to tweak.
  void updateSettings();
  void on_doubleSpinBoxPtRadius_valueChanged(double arg1);
  void on_spinBoxLabelPixSize_valueChanged(int arg1);

  // User annotation interaction
  void on_actionShowBboxes_triggered();  // show/hide non editable item
  void on_listViewImgNames_clicked(const QModelIndex &index);  // click on imahe
  void on_actionNext_triggered();                              // next image
  void on_actionPrevious_triggered();                          // previus image
  void on_actionzoom100_triggered();                           // zoom 100%
  void on_actionGrid_triggered(bool checked);                  // show grid
  void on_actionFit_Into_View_triggered();   // fit image to the view
  void on_toolButtonAddImgLabels_clicked();  // edit image labels
  void on_actionShow_Hide_Labels_triggered(bool checked);  // show/hide labels

 private:
  bool loadImagesAndAnnotations(const QString &annImg,
                                const QString &annFolder);
  void setUp();
  void addNewUniqueItem(QComboBox *cbox, const QString &label, bool selected);
  QStringList getLabelsFromComboBox(QComboBox *cbox);

  // QGraphicsItem *m_currentItem;
  Ui::MainWindow *ui;
  AnnImgManager m_annImgManager;
  ImageCanvas m_imageCanvas;
  ImgStringListModel m_imageListModel;
  QModelIndex m_current_index{};
  bool m_needToSaveNotUndo;
  QLabel *m_displayLabel;
  RecentProjects m_recentProjectsManager;
};

#endif  // MAINWINDOW_H
