#ifndef WRITEMULTIPLECOILSDIALOG_HPP
#define WRITEMULTIPLECOILSDIALOG_HPP

#include <QDialog>

#include "mainwindow.hpp"

namespace Ui {
class WriteMultipleCoilsDialog;
}

class WriteMultipleCoilsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteMultipleCoilsDialog(MainWindow *parent = nullptr, quint16 startingAddress = 0);
  ~WriteMultipleCoilsDialog();

 private slots:
  void on_btnSend_clicked();

  void on_btnCancel_clicked();

  void on_inputCount_editingFinished();

 private:
  Ui::WriteMultipleCoilsDialog *ui;

  void _setupUI(quint16 startingAddress);

  void _updateTableSettings();

  QVector<quint16> _getValues();
};

#endif  // WRITEMULTIPLECOILSDIALOG_HPP
