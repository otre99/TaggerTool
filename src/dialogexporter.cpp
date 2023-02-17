#include "dialogexporter.h"
#include "heavytaskthread.h"
#include "ui_dialogexporter.h"
#include <QListWidgetItem>
#include <QFileDialog>

const char DialogExporter::COLLECT_ANNS[] = "Collecting annotations: ";

DialogExporter::DialogExporter(QWidget *parent,
                               HeavyTaskThread *heavyTaskThread)
    : QDialog(parent), m_heavyTaskThread{heavyTaskThread},
      ui(new Ui::DialogExporter) {
  ui->setupUi(this);
  ui->progressBar->setRange(0, 100);
  m_currProgressBarText = "Collecting annotations: ";
  ui->labelProgressBarText->setText(m_currProgressBarText);
  ui->groupBox->setEnabled(false);
}

void DialogExporter::closeEvent(QCloseEvent *event) {
    m_heavyTaskThread->killTaskAndWait();
    QDialog::closeEvent(event);
}

DialogExporter::~DialogExporter() { delete ui; }

void DialogExporter::taskStarted() {
  ui->progressBar->setVisible(true);
  ui->labelProgressBarText->setVisible(true);
  ui->progressBar->setValue(0);
}

void DialogExporter::updateProgress(int p) { ui->progressBar->setValue(p); }

void DialogExporter::taskFinished(bool ok) {
  ui->progressBar->setVisible(false);
  ui->labelProgressBarText->setVisible(false);
  if (!ok)
    return;
  if (ui->labelProgressBarText->text() == COLLECT_ANNS) {
    for (const auto &lb : m_heavyTaskThread->uniqueLabels) {
      auto item = new QListWidgetItem();
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(Qt::Checked);
      item->setText(lb);
      ui->listWidget->addItem(item);
    }
  }
  ui->groupBox->setEnabled(true);
}

void DialogExporter::on_pBExportCoco_clicked()
{
    const QString cocoJsonFile = QFileDialog::getSaveFileName(this,"Output JSON COCO annotation file");
    if (cocoJsonFile.isEmpty()) return;
    getSelectedLabels();
    m_heavyTaskThread->outputDirOrFile=cocoJsonFile;
    m_heavyTaskThread->startTask(HeavyTaskThread::ExportCOCOAnnotations);
    ui->labelProgressBarText->setText("Exporting to COCO forma: ");
    ui->groupBox->setEnabled(false);
}


void DialogExporter::on_pBExportPascal_clicked()
{

}

void DialogExporter::getSelectedLabels(){
    m_heavyTaskThread->uniqueLabels.clear();
    for (int row = 0; row < ui->listWidget->count(); ++row){
        m_heavyTaskThread->uniqueLabels.insert(ui->listWidget->item(row)->text());
    }
}

