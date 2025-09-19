#include "editdialog.h"

#include <QCompleter>
#include <QMessageBox>

#include "labeltreemodel.h"
#include "ui_editdialog.h"
#include <QPushButton>

EditDialog::EditDialog(Helper::CustomItemType anntype, const QString &label,
                       const QString &dsc, QWidget *parent)
    : QDialog(parent), ui(new Ui::EditDialog) {
  ui->setupUi(this);

  auto labels = Helper::labelTreeModel->labels(anntype);
  ui->labelsComboBox->addItems(labels);
  ui->labelsComboBox->completer()->setCaseSensitivity(Qt::CaseSensitive);
  ui->labelsComboBox->setEditable(true);
  ui->groupBox1->setVisible(false);

  m_annType = anntype;
  setDescription(dsc);
  m_oldLabel = label;
  setLabel(label.isEmpty() ? labels[0] : label);

  ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(!m_oldLabel.isEmpty());
}

EditDialog::~EditDialog() { delete ui; }

QString EditDialog::label() const {
  return ui->labelsComboBox->currentText().trimmed();
}

QString EditDialog::description() const {
  return ui->textEditDescription->document()->toPlainText();
}

void EditDialog::setLabel(const QString &lb) {
  ui->labelsComboBox->setCurrentText(lb);
}

void EditDialog::setDescription(const QString &dsc) {
  ui->textEditDescription->setText(dsc);
}

void EditDialog::setOccludedTrancatedCrowded(bool occluded, bool truncated,
                                             bool crowded) {
  ui->checkBoxOccluded->setChecked(occluded);
  ui->checkBoxTruncated->setChecked(truncated);
  ui->checkBoxCrowded->setChecked(crowded);
  ui->groupBox1->setVisible(true);
}

bool EditDialog::getOccluded() const {
  return ui->checkBoxOccluded->isChecked();
}

bool EditDialog::getTruncated() const {
  return ui->checkBoxTruncated->isChecked();
}

bool EditDialog::getCrowded() const { return ui->checkBoxCrowded->isChecked(); }

void EditDialog::accept() {
  const QString lb = label();
  if (lb.trimmed().isEmpty()) {
    QMessageBox::warning(
        this, "Non valid label",
        "You need to specify a valid label");
    return;
  }

  if (label() != m_oldLabel) {
    Helper::labelTreeModel->addNewLabel(m_annType, label());
  }
  QDialog::accept();
}

void EditDialog::on_toolButtonRemoveItem_clicked() {
  m_removedItem = true;
  QDialog::accept();
}
