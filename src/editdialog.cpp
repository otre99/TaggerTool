#include "editdialog.h"

#include <QCompleter>

#include "ui_editdialog.h"
#include "utils.h"

EditDialog::EditDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EditDialog) {
  ui->setupUi(this);

  ui->labelsComboBox->addItems(Helper::currentLabels());
  ui->labelsComboBox->completer()->setCaseSensitivity(Qt::CaseSensitive);
  ui->labelsComboBox->setEditable(false);
  ui->groupBox1->setVisible(false);
}

EditDialog::~EditDialog() { delete ui; }

QString EditDialog::label() const { return ui->labelsComboBox->currentText(); }

void EditDialog::setLabel(const QString &lb) {
  ui->labelsComboBox->setCurrentText(lb);
}

void EditDialog::setOccludedTrancatedCrowded(bool occluded, bool truncated, bool crowded) {
    ui->checkBoxOccluded->setChecked(occluded);
    ui->checkBoxTruncated->setChecked(truncated);
    ui->checkBoxCrowded->setChecked(crowded);
    ui->groupBox1->setVisible(true);
}

bool  EditDialog::getOccluded() const {
    return ui->checkBoxOccluded->isChecked();
}

bool  EditDialog::getTruncated() const {
    return ui->checkBoxTruncated->isChecked();
}

bool  EditDialog::getCrowded() const {
    return ui->checkBoxCrowded->isChecked();
}

void EditDialog::on_toolButtonRemoveItem_clicked() {
  m_removedItem = true;
  accept();
}
