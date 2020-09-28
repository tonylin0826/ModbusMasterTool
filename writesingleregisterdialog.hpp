#ifndef WRITESINGLEREGISTERDIALOG_HPP
#define WRITESINGLEREGISTERDIALOG_HPP

#include <QDialog>

namespace Ui {
class WriteSingleRegisterDialog;
}

class WriteSingleRegisterDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteSingleRegisterDialog(QWidget *parent = nullptr);
  ~WriteSingleRegisterDialog();

 private:
  Ui::WriteSingleRegisterDialog *ui;

  void _setupUI();
};

#endif  // WRITESINGLEREGISTERDIALOG_HPP
