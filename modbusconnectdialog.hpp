#ifndef MODBUSCONNECTDIALOG_HPP
#define MODBUSCONNECTDIALOG_HPP

#include <QDialog>

namespace Ui {
class ModbusConnectDialog;
}

class ModbusConnectDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ModbusConnectDialog(QWidget *parent = nullptr);

  ~ModbusConnectDialog();

 private:
  Ui::ModbusConnectDialog *ui;

  void _setupUI();
};

#endif  // MODBUSCONNECTDIALOG_HPP
