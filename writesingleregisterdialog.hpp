#ifndef WRITESINGLEREGISTERDIALOG_HPP
#define WRITESINGLEREGISTERDIALOG_HPP

#include <QDialog>

#include "mainwindow.hpp"

namespace Ui {
class WriteSingleRegisterDialog;
}

class WriteSingleRegisterDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteSingleRegisterDialog(MainWindow *parent = nullptr, quint16 startingAddress = 0);
  ~WriteSingleRegisterDialog();

 private slots:
  void on_btnSend_clicked();

  void on_btnCancel_clicked();

 private:
  Ui::WriteSingleRegisterDialog *ui;

  void _setupUI(quint16 startingAddress);
};

#endif  // WRITESINGLEREGISTERDIALOG_HPP
