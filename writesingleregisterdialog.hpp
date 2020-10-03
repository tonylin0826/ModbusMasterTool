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
  explicit WriteSingleRegisterDialog(MainWindow *parent = nullptr);
  ~WriteSingleRegisterDialog();

 private slots:
  void on_btnSend_clicked();

 private:
  Ui::WriteSingleRegisterDialog *ui;

  void _setupUI();
};

#endif  // WRITESINGLEREGISTERDIALOG_HPP
