#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include "utils.h"
#include <QDebug>
#include "annotationfilter.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      current_bbox_item_(nullptr)
{
    ui->setupUi(this);
    SetUp();
}

MainWindow::~MainWindow() { delete ui; }


void MainWindow::SetUp()
{
    ui->bbox_editor_->setScene(&image_canvas_);
    ui->list_view_img_names_->setModel(&image_list_model_);

    this->tabifyDockWidget(ui->tool_panel2_, ui->tool_panel1_);
    connect(&image_canvas_, SIGNAL(BBoxItemToEditor(QGraphicsItem *, int)),
            this, SLOT(DisplayBBoxInfo(QGraphicsItem *, int)));


    ui->mainToolBar->addWidget(ui->zoom_spin_box_);

    connect(&image_canvas_, &ImageCanvas::selectionChanged, this, &MainWindow::SelectionChangedOnImageCanvas);
    connect(this, &MainWindow::export_annotations, &worker_, &ExpensiveRoutines::export_annotations);


    connect(&worker_, &ExpensiveRoutines::JobStarted,
            ui->progress_bar_, &QProgressBar::setVisible);

    connect(&worker_, &ExpensiveRoutines::Progress,
            ui->progress_bar_, &QProgressBar::setValue);

    connect(&worker_, &ExpensiveRoutines::JobFinished,
            ui->progress_bar_, &QProgressBar::setVisible);

    ui->statusBar->addPermanentWidget(ui->progress_bar_);
    ui->progress_bar_->setVisible(false);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    ui->tool_panel1_->setMaximumWidth(this->width()/4);
}


void MainWindow::on_list_view_img_names__activated(const QModelIndex &index)
{
    const QString image_id = image_list_model_.GetOriginalText(index);

    // if ( image_id == image_canvas_.GetImageId() ) return;

    auto image = ann_img_manager_.GetImage(image_id);
    auto ann = ann_img_manager_.GetAnnotations(image_id);
    image_canvas_.Reset(image, image_id);
    const QStringList keys = ann.keys();
    for (const auto &key : keys){
        image_canvas_.AddBoundingBoxes(ann[key], QStringList() << key);
    }
    DisplayImageInfo();
}

void MainWindow::on_line_edit_img_filter__editingFinished()
{
    QRegExp regexp(ui->line_edit_img_filter_->text(),Qt::CaseInsensitive, QRegExp::Wildcard);
    image_list_model_.setStringList( ann_img_manager_.GetListOfImageIds().filter(regexp) );
}

void MainWindow::DisplayImageInfo()
{

}

void MainWindow::DisplayBBoxInfo(QGraphicsItem *newFocusItem, int reason)
{
    auto *bbox_item = dynamic_cast<BoundingBoxItem*>(newFocusItem);
    if (bbox_item!=current_bbox_item_) return;

    if (current_bbox_item_) {
        ui->combox_box_bbox_label_->setCurrentText(current_bbox_item_->GetLabel());
        const auto r = current_bbox_item_->BoundingBoxCoordinates();
        ui->line_edit_bbox_x_->setText(QString::number(r.x()));
        ui->line_edit_bbox_w_->setText(QString::number(r.width()));
        ui->line_edit_bbox_y_->setText(QString::number(r.y()));
        ui->line_edit_bbox_h_->setText(QString::number(r.height()));
    } else {
        ui->line_edit_bbox_x_->setText("");
        ui->line_edit_bbox_w_->setText("");
        ui->line_edit_bbox_y_->setText("");
        ui->line_edit_bbox_h_->setText("");
    }
}


void MainWindow::SelectionChangedOnImageCanvas()
{
    const auto selected_items = image_canvas_.selectedItems();

    if (selected_items.empty())
        current_bbox_item_= nullptr;
    else if (selected_items.size()==1)
        current_bbox_item_ = dynamic_cast<BoundingBoxItem*>( selected_items[0] );
    else {
        if (current_bbox_item_ == dynamic_cast<BoundingBoxItem*>( selected_items[0] ) ) {
            selected_items[0]->setSelected(false);
            current_bbox_item_ = dynamic_cast<BoundingBoxItem*>( selected_items[1] );
        } else {
            selected_items[1]->setSelected(false);
            current_bbox_item_ = dynamic_cast<BoundingBoxItem*>( selected_items[0] );
        }
    }
    DisplayBBoxInfo(current_bbox_item_, 0);
}

void MainWindow::on_zoom_spin_box__valueChanged(int arg1)
{
    ui->bbox_editor_->resetTransform();
    ui->bbox_editor_->scale(arg1/100.0, arg1/100.0);
}

void MainWindow::on_add_new_bbox__triggered()
{
    image_canvas_.PrepareForNewBBox();
    image_canvas_.ShowNiceText("Adding new bbox");
}

void MainWindow::on_push_button_change_bbox__clicked()
{
    if (current_bbox_item_){
        current_bbox_item_->SetLabel(ui->combox_box_bbox_label_->currentText());
        QRectF coords( ui->line_edit_bbox_x_->text().toDouble(),
                       ui->line_edit_bbox_y_->text().toDouble(),
                       ui->line_edit_bbox_w_->text().toDouble(),
                       ui->line_edit_bbox_h_->text().toDouble());
        current_bbox_item_->SetCoordinates(coords);
    }
}

void MainWindow::on_add_cls_to_filter_list__clicked()
{
    const int rows = ui->table_filter_list_->rowCount()+1;
    ui->table_filter_list_->setRowCount(rows);
    ui->table_filter_list_->setItem(rows-1, 0, new QTableWidgetItem(ui->le_label_to_add_->text()));
    ui->table_filter_list_->setItem(rows-1, 1, new QTableWidgetItem(ui->le_label_to_add_->text()));
}

void MainWindow::on_tb_load_image_folder__clicked()
{
    const QString img_folder = QFileDialog::getExistingDirectory(this);
    if (img_folder.isEmpty()) return;
    ui->le_input_image_folder_->setText(img_folder);
}


void MainWindow::on_tb_load_ann_folder__clicked()
{
    const QString ann_folder = QFileDialog::getExistingDirectory(this);
    if (ann_folder.isEmpty()) return;
    ui->le_input_ann_folder_->setText(ann_folder);
}

void MainWindow::on_pb_load_img_ann__clicked()
{
    ann_img_manager_.Reset(ui->le_input_image_folder_->text(),
                           ui->le_input_ann_folder_->text(),
                           ui->cbbox_ann_format_->currentText());

    image_list_model_.setStringList(ann_img_manager_.GetListOfImageIds());
}

void MainWindow::on_remove_cls_to_filter_list__clicked()
{
    const auto ranges = ui->table_filter_list_->selectedRanges();
    QVector<int> rows_to_remove;
    for (auto &r : ranges){
        for (int i=r.topRow(); i<=r.bottomRow();++i)
            rows_to_remove.push_back(i);
    }
    std::sort(rows_to_remove.begin(), rows_to_remove.end());
    std::reverse_iterator<QVector<int>::Iterator> r = rows_to_remove.rbegin();
    while (r!=rows_to_remove.rend())
    {
        ui->table_filter_list_->removeRow(*r);
        ++r;
    }
}

void MainWindow::on_pb_export__clicked()
{
    QString out_path;
    if ( ui->combo_box_line_format_->currentIndex() == 2)
        out_path = QFileDialog::getSaveFileName();
    else
        out_path = QFileDialog::getExistingDirectory();
    if (out_path.isEmpty()) return;

    QMap<QString,QString> f;
    for (int i=0;i<ui->table_filter_list_->rowCount(); ++i){
        const auto key = ui->table_filter_list_->item(i,0)->text();
        const auto value = ui->table_filter_list_->item(i,1)->text();
        f[key]=value;
    }
    AnnotationFilter ann_filter;
    ann_filter.SetFilteredLabels(f);

    emit export_annotations(&ann_img_manager_,
                            ann_filter,
                            out_path,
                            ui->combo_box_line_format_->currentIndex());
}


void MainWindow::on_save_local_changes__triggered()
{
    const auto ann = image_canvas_.GetBoundingBoxes();
    const QString img_id = image_canvas_.GetImageId();
    const QSize img_size = image_canvas_.GetImageSize();
    ann_img_manager_.SaveAnnotations(img_id,ann,img_size);
    image_canvas_.ShowNiceText("Saved");
}

void MainWindow::on_remove_bbox__triggered()
{
    image_canvas_.RemoveSelectedBoundingBoxes();
}

void MainWindow::on_action_show_bboxes__triggered()
{
    static bool vis=true;
    if (vis)
        image_canvas_.ShowBoundingBoxes();
    else
        image_canvas_.HideBoundingBoxes();
    vis=!vis;

}
