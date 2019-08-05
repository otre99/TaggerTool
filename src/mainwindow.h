#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "imagecanvas.h"
#include <QDir>
#include "imgstringlistmodel.h"
#include "bboxitem.h"
#include "annimgmanager.h"
#include "expensiveroutines.h"

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
    void on_list_view_img_names__activated(const QModelIndex &index);
    void on_line_edit_img_filter__editingFinished();
    void DisplayBBoxInfo(QGraphicsItem *newFocusItem, int reason);
    void SelectionChangedOnImageCanvas();

    void on_zoom_spin_box__valueChanged(int arg1);
    void on_add_new_bbox__triggered();
    void on_push_button_change_bbox__clicked();
    void on_add_cls_to_filter_list__clicked();
    void on_tb_load_image_folder__clicked();
    void on_tb_load_ann_folder__clicked();
    void on_pb_load_img_ann__clicked();
    void on_remove_cls_to_filter_list__clicked();
    void on_pb_export__clicked();
    void on_save_local_changes__triggered();
    void on_remove_bbox__triggered();
    void on_action_show_bboxes__triggered();

private:
    void SetUp();
    void DisplayImageInfo();

    Ui::MainWindow *ui;
    AnnImgManager ann_img_manager_;
    ImageCanvas image_canvas_;
    ImgStringListModel image_list_model_;
    BoundingBoxItem *current_bbox_item_;
    ExpensiveRoutines worker_;

signals:
    void export_annotations(AnnImgManager *info,
                            AnnotationFilter filter,
                            const QString &output_ann_path,
                            int);

};

#endif // MAINWINDOW_H
