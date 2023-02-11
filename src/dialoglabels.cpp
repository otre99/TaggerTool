#include "dialoglabels.h"

#include "ui_dialoglabels.h"

DialogLabels::DialogLabels(QWidget *parent, const QString title)
    : QDialog(parent), ui(new Ui::DialogLabels) {
  ui->setupUi(this);
  ui->labelTitle->setText(title);
}

DialogLabels::~DialogLabels() { delete ui; }

QStringList DialogLabels::getLabels() const {
  QStringList l;
  for (int i = 0; i < ui->listWidgetTags->count(); ++i) {
    l.append(ui->listWidgetTags->item(i)->text());
  }
  return l;
}

void DialogLabels::setLabels(const QStringList &labels) {
  for (const auto &label : labels) {
    auto item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    item->setText(label);
    ui->listWidgetTags->addItem(item);
  }
}

void DialogLabels::on_tBAdd_clicked() {
  QString currTag = ui->lineEditLb->text();
  if (currTag.isEmpty())
    return;

  auto item = new QListWidgetItem();
  item->setFlags(item->flags() | Qt::ItemIsEditable);
  item->setText(currTag);
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    ui->listWidgetTags->insertItem(idx.row() + 1, item);
  } else {
    ui->listWidgetTags->addItem(item);
  }
  ui->lineEditLb->clear();
}

void DialogLabels::on_tBRemove_clicked() {
  auto idx = ui->listWidgetTags->currentIndex();
  if (idx.isValid()) {
    delete ui->listWidgetTags->takeItem(idx.row());
  }
}
