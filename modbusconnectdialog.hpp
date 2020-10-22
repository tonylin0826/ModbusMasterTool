#ifndef MODBUSCONNECTDIALOG_HPP
#define MODBUSCONNECTDIALOG_HPP

#include <QDialog>

#include "mainwindow.hpp"
namespace Ui {
class ModbusConnectDialog;
}

class ModbusConnectDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ModbusConnectDialog(MainWindow *parent = nullptr);

  ~ModbusConnectDialog();

 signals:
  void modbusDeviceGenerated(QModbusClient *modus);

 private slots:
  void on_btnConnect_clicked();

  void on_btnCancel_clicked();

  void on_selectProtocolType_currentIndexChanged(int index);

 private:
  Ui::ModbusConnectDialog *ui;

  void _setupUI();

  void _updateDisabledArea();

  void _connectRtu();

  void _connectTcp();
};

#endif  // MODBUSCONNECTDIALOG_HPP
