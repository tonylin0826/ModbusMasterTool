#include "addmodbusregisterdialog.hpp"

#include <QDebug>
#include <QMessageBox>

#include "ui_addmodbusregisterdialog.h"

AddModbusRegisterDialog::AddModbusRegisterDialog(QWidget *parent)
    : QDialog(parent), _ui(new Ui::AddModbusRegisterDialog) {
  _ui->setupUi(this);
  _setupUI();
  setAttribute(Qt::WA_DeleteOnClose);
}

AddModbusRegisterDialog::~AddModbusRegisterDialog() { delete _ui; }

void AddModbusRegisterDialog::on_btnCalcel_clicked() {
  emit canceled();
  close();
}

void AddModbusRegisterDialog::on_btnOk_clicked() {
  const auto validPair = _isInputValid();

  if (!validPair.first) {
    // TODO: show error;
    QMessageBox::warning(this, "Warning", validPair.second, QMessageBox::Ok);
    qDebug() << validPair.second;
    return;
  }

  qDebug() << _ui->inputCount->text();
  qDebug() << _ui->inputAddress->text();
  qDebug() << _ui->selectType->currentIndex();

  const quint16 count = _ui->inputCount->text().toUInt();
  const quint16 address = _ui->inputAddress->text().toUInt();

  emit oked(static_cast<Modbus::RegisterType>(_ui->selectType->currentIndex()), address, count);
  close();
}

void AddModbusRegisterDialog::_setupUI() {
  // TODO: check if it is edit or new
  setWindowTitle("New Register Window");
  _ui->inputCount->setValidator(new QIntValidator(0, 65535, this));
  _ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
}

QPair<bool, QString> AddModbusRegisterDialog::_isInputValid() {
  const auto type = static_cast<Modbus::RegisterType>(_ui->selectType->currentIndex());

  const quint16 counts[4] = {2000, 2000, 125, 125};

  const quint16 address = _ui->inputAddress->text().toUInt();
  const quint16 maxCount =
      ((address + counts[type]) > 65535) ? (counts[type] - (address + counts[type] - 65535)) : counts[type];

  if (!_ui->inputAddress->hasAcceptableInput()) {
    return {false, "Address should be between 0 - 65535"};
  }

  if (!_ui->inputCount->hasAcceptableInput()) {
    return {false, QString("Count should be between 0 - %1").arg(QString::number(maxCount))};
  }

  return {true, ""};
}

void AddModbusRegisterDialog::_onInputChanged() {
  const auto type = static_cast<Modbus::RegisterType>(_ui->selectType->currentIndex());

  const quint16 counts[4] = {2000, 2000, 125, 125};

  const quint16 address = _ui->inputAddress->text().toUInt();
  const quint16 maxCount =
      ((address + counts[type]) > 65535) ? (counts[type] - (address + counts[type] - 65535)) : counts[type];

  qDebug() << maxCount;
  _ui->inputCount->setValidator(new QIntValidator(0, maxCount, this));
}

void AddModbusRegisterDialog::on_selectType_currentIndexChanged(int index) {
  Q_UNUSED(index);
  _onInputChanged();
}

void AddModbusRegisterDialog::on_inputAddress_textChanged(const QString &addressStr) {
  Q_UNUSED(addressStr);
  _onInputChanged();
}

void AddModbusRegisterDialog::on_inputCount_textChanged(const QString &count) {
  Q_UNUSED(count);
  _onInputChanged();
}