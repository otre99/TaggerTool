#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QMap>

#include "annimgmanager.h"
#include "import_export/export_options.h"

class QAbstractButton;
class QComboBox;
class QLabel;

namespace Ui {
class ExportDialog;
}

/**
 * @brief Settings window shown before an export runs.
 *
 * The dialog scans the project once so it can show how many annotations of
 * each type and label exist, offers a per type conversion (a closed polygon
 * exported as its bounding box, a circle as a polygon, ...), a label filter
 * with class renaming, geometry filters and the options specific to the
 * selected format. Choices are remembered per format through QSettings.
 */
class ExportDialog : public QDialog {
  Q_OBJECT

 public:
  ExportDialog(ExportOptions::Format format, AnnImgManager *mgr,
               QWidget *parent = nullptr);
  ~ExportDialog() override;

  /// Settings as configured by the user. Valid once exec() returned Accepted.
  ExportOptions options() const;

  /// Human readable name of @p format, used in titles and messages.
  static QString formatName(ExportOptions::Format format);
  /// QSettings sub-group the settings of @p format are remembered under.
  static QString settingsGroup(ExportOptions::Format format);

 private slots:
  void on_toolButtonBrowse_clicked();
  void on_pushButtonPreview_clicked();
  void on_pushButtonCheckAll_clicked();
  void on_pushButtonCheckNone_clicked();
  void on_pushButtonResetNames_clicked();
  void on_lineEditLabelSearch_textChanged(const QString &text);
  void on_checkBoxFilterLabels_toggled(bool on);
  void on_lineEditOutput_textChanged(const QString &text);
  void on_comboBoxYoloLayout_currentIndexChanged(int index);
  void on_buttonBox_clicked(QAbstractButton *button);

 private:
  /// Handling values the selected format can represent for @p type.
  QVector<ExportOptions::Handling> allowedHandling(
      Helper::CustomItemType type) const;
  void populateCombos();
  void populateFormatPage();
  void scanProject();
  void fillLabelTable();
  void applyOptions(const ExportOptions &opt);
  void setAllChecked(bool checked);
  void updateOkState();
  QComboBox *comboFor(Helper::CustomItemType type) const;
  QLabel *countLabelFor(Helper::CustomItemType type) const;

  Ui::ExportDialog *ui;
  ExportOptions::Format m_format;
  AnnImgManager *m_mgr;
  QMap<int, int> m_typeCounts;      ///< Helper::CustomItemType -> count
  QMap<QString, int> m_labelCounts;  ///< label -> count
};

#endif  // EXPORTDIALOG_H
