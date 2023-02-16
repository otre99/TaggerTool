#include "dialogexporter.h"
#include "heavytaskthread.h"
#include "ui_dialogexporter.h"
#include <QListWidgetItem>

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
    ui->groupBox->setEnabled(true);
    for (const auto &lb : m_heavyTaskThread->uniqueLabels) {
      auto item = new QListWidgetItem();
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(Qt::Checked);
      item->setText(lb);
      ui->listWidget->addItem(item);
    }
  }
}
