/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "graphicsview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *save_local_changes_;
    QAction *add_new_bbox_;
    QAction *remove_bbox_;
    QAction *action_show_bboxes_;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    GraphicsView *bbox_editor_;
    QToolButton *toolButtonZoom100_;
    QProgressBar *progress_bar_;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *tool_panel1_;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout_3;
    QLabel *label_11;
    QHBoxLayout *horizontalLayout_4;
    QLineEdit *le_input_image_folder_;
    QToolButton *tb_load_image_folder_;
    QLabel *label_13;
    QHBoxLayout *horizontalLayout_5;
    QLineEdit *le_input_ann_folder_;
    QToolButton *tb_load_ann_folder_;
    QLabel *label_14;
    QComboBox *cbbox_ann_format_;
    QPushButton *pb_load_img_ann_;
    QLineEdit *line_edit_img_filter_;
    QListView *list_view_img_names_;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QLineEdit *line_edit_bbox_w_;
    QLabel *label_4;
    QLineEdit *line_edit_bbox_y_;
    QLabel *label_5;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *line_edit_bbox_x_;
    QLineEdit *line_edit_bbox_h_;
    QLabel *label_3;
    QComboBox *combox_box_bbox_label_;
    QPushButton *push_button_change_bbox_;
    QDockWidget *tool_panel2_;
    QWidget *dockWidgetContents_10;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout_4;
    QComboBox *combo_box_line_format_;
    QHBoxLayout *horizontalLayout_3;
    QLineEdit *le_label_to_add_;
    QToolButton *add_cls_to_filter_list_;
    QToolButton *remove_cls_to_filter_list_;
    QLabel *label_9;
    QTableWidget *table_filter_list_;
    QPushButton *pb_export_;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(854, 710);
        save_local_changes_ = new QAction(MainWindow);
        save_local_changes_->setObjectName(QString::fromUtf8("save_local_changes_"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/icons/document-save.png"), QSize(), QIcon::Normal, QIcon::Off);
        icon.addFile(QString::fromUtf8(":/icons/icons/document-save.png"), QSize(), QIcon::Normal, QIcon::On);
        save_local_changes_->setIcon(icon);
        add_new_bbox_ = new QAction(MainWindow);
        add_new_bbox_->setObjectName(QString::fromUtf8("add_new_bbox_"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/icons/document-new.svg"), QSize(), QIcon::Normal, QIcon::Off);
        add_new_bbox_->setIcon(icon1);
        remove_bbox_ = new QAction(MainWindow);
        remove_bbox_->setObjectName(QString::fromUtf8("remove_bbox_"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/icons/edit-delete.svg"), QSize(), QIcon::Normal, QIcon::Off);
        remove_bbox_->setIcon(icon2);
        action_show_bboxes_ = new QAction(MainWindow);
        action_show_bboxes_->setObjectName(QString::fromUtf8("action_show_bboxes_"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/icons/im-invisible-user.svg"), QSize(), QIcon::Normal, QIcon::Off);
        action_show_bboxes_->setIcon(icon3);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(2);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(3, 3, 3, 3);
        bbox_editor_ = new GraphicsView(centralWidget);
        bbox_editor_->setObjectName(QString::fromUtf8("bbox_editor_"));
        bbox_editor_->setTabletTracking(true);

        gridLayout->addWidget(bbox_editor_, 0, 1, 1, 1);

        toolButtonZoom100_ = new QToolButton(centralWidget);
        toolButtonZoom100_->setObjectName(QString::fromUtf8("toolButtonZoom100_"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/icons/zoom-fit-best.svg"), QSize(), QIcon::Normal, QIcon::Off);
        toolButtonZoom100_->setIcon(icon4);

        gridLayout->addWidget(toolButtonZoom100_, 0, 0, 1, 1);

        progress_bar_ = new QProgressBar(centralWidget);
        progress_bar_->setObjectName(QString::fromUtf8("progress_bar_"));
        progress_bar_->setValue(0);

        gridLayout->addWidget(progress_bar_, 1, 0, 1, 1);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 854, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setIconSize(QSize(24, 24));
        mainToolBar->setFloatable(false);
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        tool_panel1_ = new QDockWidget(MainWindow);
        tool_panel1_->setObjectName(QString::fromUtf8("tool_panel1_"));
        tool_panel1_->setTabletTracking(true);
        tool_panel1_->setFloating(false);
        tool_panel1_->setFeatures(QDockWidget::DockWidgetClosable);
        tool_panel1_->setAllowedAreas(Qt::LeftDockWidgetArea);
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayout_3 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_3->setSpacing(2);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(3, 3, 3, 3);
        groupBox_5 = new QGroupBox(dockWidgetContents);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        gridLayout_3 = new QGridLayout(groupBox_5);
        gridLayout_3->setSpacing(2);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        gridLayout_3->setContentsMargins(3, 3, 3, 3);
        label_11 = new QLabel(groupBox_5);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_3->addWidget(label_11, 0, 0, 1, 2);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        le_input_image_folder_ = new QLineEdit(groupBox_5);
        le_input_image_folder_->setObjectName(QString::fromUtf8("le_input_image_folder_"));
        le_input_image_folder_->setReadOnly(true);

        horizontalLayout_4->addWidget(le_input_image_folder_);

        tb_load_image_folder_ = new QToolButton(groupBox_5);
        tb_load_image_folder_->setObjectName(QString::fromUtf8("tb_load_image_folder_"));

        horizontalLayout_4->addWidget(tb_load_image_folder_);


        gridLayout_3->addLayout(horizontalLayout_4, 1, 0, 1, 3);

        label_13 = new QLabel(groupBox_5);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_3->addWidget(label_13, 2, 0, 1, 2);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setSpacing(2);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        le_input_ann_folder_ = new QLineEdit(groupBox_5);
        le_input_ann_folder_->setObjectName(QString::fromUtf8("le_input_ann_folder_"));
        le_input_ann_folder_->setReadOnly(true);

        horizontalLayout_5->addWidget(le_input_ann_folder_);

        tb_load_ann_folder_ = new QToolButton(groupBox_5);
        tb_load_ann_folder_->setObjectName(QString::fromUtf8("tb_load_ann_folder_"));

        horizontalLayout_5->addWidget(tb_load_ann_folder_);


        gridLayout_3->addLayout(horizontalLayout_5, 3, 0, 1, 3);

        label_14 = new QLabel(groupBox_5);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        gridLayout_3->addWidget(label_14, 4, 0, 1, 1);

        cbbox_ann_format_ = new QComboBox(groupBox_5);
        cbbox_ann_format_->addItem(QString());
        cbbox_ann_format_->addItem(QString());
        cbbox_ann_format_->setObjectName(QString::fromUtf8("cbbox_ann_format_"));
        cbbox_ann_format_->setEditable(true);

        gridLayout_3->addWidget(cbbox_ann_format_, 4, 1, 1, 1);

        pb_load_img_ann_ = new QPushButton(groupBox_5);
        pb_load_img_ann_->setObjectName(QString::fromUtf8("pb_load_img_ann_"));

        gridLayout_3->addWidget(pb_load_img_ann_, 4, 2, 1, 1);


        verticalLayout_3->addWidget(groupBox_5);

        line_edit_img_filter_ = new QLineEdit(dockWidgetContents);
        line_edit_img_filter_->setObjectName(QString::fromUtf8("line_edit_img_filter_"));

        verticalLayout_3->addWidget(line_edit_img_filter_);

        list_view_img_names_ = new QListView(dockWidgetContents);
        list_view_img_names_->setObjectName(QString::fromUtf8("list_view_img_names_"));
        list_view_img_names_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        list_view_img_names_->setFlow(QListView::TopToBottom);
        list_view_img_names_->setResizeMode(QListView::Adjust);
        list_view_img_names_->setViewMode(QListView::ListMode);
        list_view_img_names_->setUniformItemSizes(false);

        verticalLayout_3->addWidget(list_view_img_names_);

        groupBox_2 = new QGroupBox(dockWidgetContents);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(2);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        gridLayout_2->setContentsMargins(3, 3, 3, 3);
        line_edit_bbox_w_ = new QLineEdit(groupBox_2);
        line_edit_bbox_w_->setObjectName(QString::fromUtf8("line_edit_bbox_w_"));

        gridLayout_2->addWidget(line_edit_bbox_w_, 2, 4, 1, 1);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout_2->addWidget(label_4, 2, 3, 1, 1);

        line_edit_bbox_y_ = new QLineEdit(groupBox_2);
        line_edit_bbox_y_->setObjectName(QString::fromUtf8("line_edit_bbox_y_"));

        gridLayout_2->addWidget(line_edit_bbox_y_, 3, 1, 1, 2);

        label_5 = new QLabel(groupBox_2);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout_2->addWidget(label_5, 3, 3, 1, 1);

        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout_2->addWidget(label, 0, 0, 1, 2);

        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout_2->addWidget(label_2, 2, 0, 1, 1);

        line_edit_bbox_x_ = new QLineEdit(groupBox_2);
        line_edit_bbox_x_->setObjectName(QString::fromUtf8("line_edit_bbox_x_"));

        gridLayout_2->addWidget(line_edit_bbox_x_, 2, 1, 1, 2);

        line_edit_bbox_h_ = new QLineEdit(groupBox_2);
        line_edit_bbox_h_->setObjectName(QString::fromUtf8("line_edit_bbox_h_"));

        gridLayout_2->addWidget(line_edit_bbox_h_, 3, 4, 1, 1);

        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout_2->addWidget(label_3, 3, 0, 1, 1);

        combox_box_bbox_label_ = new QComboBox(groupBox_2);
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->addItem(QString());
        combox_box_bbox_label_->setObjectName(QString::fromUtf8("combox_box_bbox_label_"));
        combox_box_bbox_label_->setEditable(true);

        gridLayout_2->addWidget(combox_box_bbox_label_, 0, 2, 1, 3);

        push_button_change_bbox_ = new QPushButton(groupBox_2);
        push_button_change_bbox_->setObjectName(QString::fromUtf8("push_button_change_bbox_"));

        gridLayout_2->addWidget(push_button_change_bbox_, 4, 4, 1, 1);


        verticalLayout_3->addWidget(groupBox_2);

        tool_panel1_->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, tool_panel1_);
        tool_panel2_ = new QDockWidget(MainWindow);
        tool_panel2_->setObjectName(QString::fromUtf8("tool_panel2_"));
        tool_panel2_->setFeatures(QDockWidget::DockWidgetClosable);
        dockWidgetContents_10 = new QWidget();
        dockWidgetContents_10->setObjectName(QString::fromUtf8("dockWidgetContents_10"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents_10);
        verticalLayout_2->setSpacing(2);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(3, 3, 3, 3);
        groupBox_4 = new QGroupBox(dockWidgetContents_10);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        gridLayout_4 = new QGridLayout(groupBox_4);
        gridLayout_4->setSpacing(2);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QString::fromUtf8("gridLayout_4"));
        gridLayout_4->setContentsMargins(3, 3, 3, 3);
        combo_box_line_format_ = new QComboBox(groupBox_4);
        combo_box_line_format_->addItem(QString());
        combo_box_line_format_->addItem(QString());
        combo_box_line_format_->addItem(QString());
        combo_box_line_format_->setObjectName(QString::fromUtf8("combo_box_line_format_"));
        combo_box_line_format_->setEditable(true);

        gridLayout_4->addWidget(combo_box_line_format_, 0, 1, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        le_label_to_add_ = new QLineEdit(groupBox_4);
        le_label_to_add_->setObjectName(QString::fromUtf8("le_label_to_add_"));

        horizontalLayout_3->addWidget(le_label_to_add_);

        add_cls_to_filter_list_ = new QToolButton(groupBox_4);
        add_cls_to_filter_list_->setObjectName(QString::fromUtf8("add_cls_to_filter_list_"));

        horizontalLayout_3->addWidget(add_cls_to_filter_list_);

        remove_cls_to_filter_list_ = new QToolButton(groupBox_4);
        remove_cls_to_filter_list_->setObjectName(QString::fromUtf8("remove_cls_to_filter_list_"));

        horizontalLayout_3->addWidget(remove_cls_to_filter_list_);


        gridLayout_4->addLayout(horizontalLayout_3, 1, 0, 1, 2);

        label_9 = new QLabel(groupBox_4);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_4->addWidget(label_9, 0, 0, 1, 1);

        table_filter_list_ = new QTableWidget(groupBox_4);
        if (table_filter_list_->columnCount() < 2)
            table_filter_list_->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        table_filter_list_->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        table_filter_list_->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        table_filter_list_->setObjectName(QString::fromUtf8("table_filter_list_"));
        table_filter_list_->setSelectionBehavior(QAbstractItemView::SelectRows);

        gridLayout_4->addWidget(table_filter_list_, 2, 0, 1, 2);

        pb_export_ = new QPushButton(groupBox_4);
        pb_export_->setObjectName(QString::fromUtf8("pb_export_"));
        pb_export_->setMaximumSize(QSize(139, 16777215));

        gridLayout_4->addWidget(pb_export_, 4, 0, 1, 2);


        verticalLayout_2->addWidget(groupBox_4);

        tool_panel2_->setWidget(dockWidgetContents_10);
        MainWindow->addDockWidget(Qt::LeftDockWidgetArea, tool_panel2_);

        mainToolBar->addAction(save_local_changes_);
        mainToolBar->addSeparator();
        mainToolBar->addAction(add_new_bbox_);
        mainToolBar->addAction(remove_bbox_);
        mainToolBar->addAction(action_show_bboxes_);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        save_local_changes_->setText(QCoreApplication::translate("MainWindow", "Save local changes", nullptr));
#if QT_CONFIG(shortcut)
        save_local_changes_->setShortcut(QCoreApplication::translate("MainWindow", "S", nullptr));
#endif // QT_CONFIG(shortcut)
        add_new_bbox_->setText(QCoreApplication::translate("MainWindow", "New BBox", nullptr));
#if QT_CONFIG(shortcut)
        add_new_bbox_->setShortcut(QCoreApplication::translate("MainWindow", "A", nullptr));
#endif // QT_CONFIG(shortcut)
        remove_bbox_->setText(QCoreApplication::translate("MainWindow", "Remove BBox", nullptr));
#if QT_CONFIG(shortcut)
        remove_bbox_->setShortcut(QCoreApplication::translate("MainWindow", "X", nullptr));
#endif // QT_CONFIG(shortcut)
        action_show_bboxes_->setText(QCoreApplication::translate("MainWindow", "Show ", nullptr));
#if QT_CONFIG(shortcut)
        action_show_bboxes_->setShortcut(QCoreApplication::translate("MainWindow", "H", nullptr));
#endif // QT_CONFIG(shortcut)
        toolButtonZoom100_->setText(QCoreApplication::translate("MainWindow", "100%", nullptr));
        tool_panel1_->setWindowTitle(QCoreApplication::translate("MainWindow", "Images", nullptr));
        groupBox_5->setTitle(QString());
        label_11->setText(QCoreApplication::translate("MainWindow", "Images folder:", nullptr));
        tb_load_image_folder_->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Annotation folder:", nullptr));
        tb_load_ann_folder_->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        label_14->setText(QCoreApplication::translate("MainWindow", "Ann. format:", nullptr));
        cbbox_ann_format_->setItemText(0, QCoreApplication::translate("MainWindow", ".csv", nullptr));
        cbbox_ann_format_->setItemText(1, QCoreApplication::translate("MainWindow", ".xml", nullptr));

        pb_load_img_ann_->setText(QCoreApplication::translate("MainWindow", "load", nullptr));
        groupBox_2->setTitle(QString());
        label_4->setText(QCoreApplication::translate("MainWindow", "w:", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "h:", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Label:", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "x:", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "y:", nullptr));
        combox_box_bbox_label_->setItemText(0, QCoreApplication::translate("MainWindow", "face", nullptr));
        combox_box_bbox_label_->setItemText(1, QCoreApplication::translate("MainWindow", "person", nullptr));
        combox_box_bbox_label_->setItemText(2, QCoreApplication::translate("MainWindow", "head", nullptr));
        combox_box_bbox_label_->setItemText(3, QCoreApplication::translate("MainWindow", "car", nullptr));
        combox_box_bbox_label_->setItemText(4, QCoreApplication::translate("MainWindow", "dog", nullptr));
        combox_box_bbox_label_->setItemText(5, QCoreApplication::translate("MainWindow", "cat", nullptr));

        push_button_change_bbox_->setText(QCoreApplication::translate("MainWindow", "Set", nullptr));
#if QT_CONFIG(accessibility)
        tool_panel2_->setAccessibleName(QString());
#endif // QT_CONFIG(accessibility)
        tool_panel2_->setWindowTitle(QCoreApplication::translate("MainWindow", "Exporter", nullptr));
        groupBox_4->setTitle(QString());
        combo_box_line_format_->setItemText(0, QCoreApplication::translate("MainWindow", "Native format", nullptr));
        combo_box_line_format_->setItemText(1, QCoreApplication::translate("MainWindow", "XML (VOC like format)", nullptr));
        combo_box_line_format_->setItemText(2, QCoreApplication::translate("MainWindow", "Single file", nullptr));

        add_cls_to_filter_list_->setText(QCoreApplication::translate("MainWindow", "Add", nullptr));
        remove_cls_to_filter_list_->setText(QCoreApplication::translate("MainWindow", "Remove", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Format:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = table_filter_list_->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("MainWindow", "Label", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = table_filter_list_->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("MainWindow", "Alias", nullptr));
        pb_export_->setText(QCoreApplication::translate("MainWindow", "Export", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
