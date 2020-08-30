#ifndef ADDMODBUSREGISTERDIALOG_HPP
#define ADDMODBUSREGISTERDIALOG_HPP

#include <QDialog>
#include <QPair>

#include "modbus.hpp"

namespace Ui {
class AddModbusRegisterDialog;
}

class AddModbusRegisterDialog : public QDialog {
  Q_OBJECT

 public:
  explicit AddModbusRegisterDialog(QWidget *parent = nullptr);
  ~AddModbusRegisterDialog();

 private slots:
  void on_btnCalcel_clicked();

  void on_btnOk_clicked();

  void on_selectType_currentIndexChanged(int index);

  void on_inputAddress_textChanged(const QString &arg1);

  void on_inputCount_textChanged(const QString &arg1);

 private:
  Ui::AddModbusRegisterDialog *_ui;

  void _setupUI();

  QPair<bool, QString> _isInputValid();

  void _onInputChanged();

 signals:
  void oked(Modbus::RegisterType type, quint16 address, quint16 count);
  void canceled();
};

#endif  // ADDMODBUSREGISTERDIALOG_HPP
