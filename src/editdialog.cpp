#include "editdialog.h"

#include <QCompleter>

#include "ui_editdialog.h"
#include "utils.h"

EditDialog::EditDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::EditDialog) {
  ui->setupUi(this);

  ui->labelsComboBox->addItems(Helper::currentLabels());
  ui->labelsComboBox->completer()->setCaseSensitivity(Qt::CaseSensitive);
}

EditDialog::~EditDialog() { delete ui; }

QString EditDialog::label() const { return ui->labelsComboBox->currentText(); }

void EditDialog::setLabel(const QString &lb) {
  ui->labelsComboBox->setCurrentText(lb);
}
