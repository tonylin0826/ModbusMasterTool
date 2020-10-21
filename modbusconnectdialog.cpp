#include "modbusconnectdialog.hpp"

#include <QDebug>
#include <QSerialPortInfo>

#include "ui_modbusconnectdialog.h"

ModbusConnectDialog::ModbusConnectDialog(MainWindow *parent) : QDialog(parent), ui(new Ui::ModbusConnectDialog) {
  _setupUI();
}

ModbusConnectDialog::~ModbusConnectDialog() {
  qDebug() << "delete";
  delete ui;
}

void ModbusConnectDialog::_setupUI() {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Modbus Connection Settings");

  ui->inputPort->setText("502");
  ui->inputPort->setValidator(new QIntValidator(0, 65535, this));

  QString IpRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  QRegularExpression IpRegex("^" + IpRange + "(\\." + IpRange + ")" + "(\\." + IpRange + ")" + "(\\." + IpRange + ")$");

  ui->inputIpAddress->setText("0.0.0.0");
  ui->inputIpAddress->setText("10.211.55.3");

  const auto ports = QSerialPortInfo::availablePorts();

  for (const auto &port : ports) {
    ui->selectSerialDevice->addItem(port.portName());
  }

  _updateDisabledArea();
}

void ModbusConnectDialog::_updateDisabledArea() {
  const auto selectedProtocol = ui->selectProtocolType->currentIndex();
  if (selectedProtocol == 0) {
    ui->groupSerial->setDisabled(true);
    ui->groupTcp->setDisabled(false);
    return;
  }

  ui->groupSerial->setDisabled(false);
  ui->groupTcp->setDisabled(true);
}

void ModbusConnectDialog::_connectRtu() {
  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();
  if (modbus->state() == QModbusClient::ConnectedState || modbus->state() == QModbusClient::ConnectingState) {
    modbus->disconnectDevice();
    return;
  }

  if (modbus->state() == QModbusClient::ClosingState) {
    return;
  }

  ui->btnConnect->setDisabled(true);

  const int databits[2] = {7, 8};
  const int parities[2] = {2, 3};
  const int stopbits[2] = {1, 2};
  const int baudrates[8] = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

  modbus->setConnectionParameter(QModbusDevice::SerialPortNameParameter, ui->selectSerialDevice->currentText());
  modbus->setConnectionParameter(QModbusDevice::SerialParityParameter, parities[ui->selectParityBit->currentIndex()]);
  modbus->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, baudrates[ui->selectBaudrate->currentIndex()]);
  modbus->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, databits[ui->selectDataBit->currentIndex()]);
  modbus->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, stopbits[ui->selectStopBit->currentIndex()]);

  modbus->setTimeout(3000);
  modbus->setNumberOfRetries(0);

  modbus->connectDevice();

  close();
}

void ModbusConnectDialog::_connectTcp() {
  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();
  if (modbus->state() == QModbusClient::ConnectedState || modbus->state() == QModbusClient::ConnectingState) {
    modbus->disconnectDevice();
    return;
  }

  if (modbus->state() == QModbusClient::ClosingState) {
    return;
  }

  if (!ui->inputPort->hasAcceptableInput() || !ui->inputIpAddress->hasAcceptableInput()) {
    // TODO: show error message
    qWarning() << "invalid input";
    return;
  }

  ui->btnConnect->setDisabled(true);

  modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(ui->inputPort->text().toUShort()));
  modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(ui->inputIpAddress->text()));

  modbus->setTimeout(3000);
  modbus->setNumberOfRetries(0);

  modbus->connectDevice();

  close();
}

void ModbusConnectDialog::on_btnConnect_clicked() {
  return ui->selectProtocolType->currentIndex() == 0 ? _connectTcp() : _connectRtu();
}

void ModbusConnectDialog::on_btnCancel_clicked() { close(); }

void ModbusConnectDialog::on_selectProtocolType_currentIndexChanged(int index) {
  Q_UNUSED(index);
  _updateDisabledArea();
}
