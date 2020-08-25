#include "mainwindow.hpp"

#include <QAbstractSocket>
#include <QDebug>
#include <QObject>

#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _modbus(new Modbus::ModbusTcp(this, 1)), _modbusConnected(false) {
  _ui->setupUi(this);

  QObject::connect(_modbus, &Modbus::ModbusTcp::errorOccurred, this,
                   [=](QAbstractSocket::SocketError error) { qDebug() << "socket error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::modbusErrorOccurred, this,
                   [=](Modbus::ModbusErrorCode error) { qDebug() << "modbus error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::disconnected, this, [=]() {
    _modbusConnected = false;
    qDebug() << "slave disconnected";
  });

  QObject::connect(_modbus, &Modbus::ModbusTcp::connected, this, [=]() {
    _modbusConnected = true;
    qDebug() << "slave connected, starting to read";
    _modbus->readHoldingRegisters(0, 100, [=](Modbus::ModbusReadResult r) {
      qDebug() << "read success " << r.success << " - " << r.errorMessage;

      for (const auto &reg : r.results) {
        qDebug() << reg.toHex();
      }
    });
  });
}

MainWindow::~MainWindow() {
  _modbus->disconnect();
  delete _ui;
}

void MainWindow::on_pushButton_clicked() {
  if (_modbusConnected) {
    _modbus->readHoldingRegisters(0, 100, [=](Modbus::ModbusReadResult r) {
      qDebug() << "read success " << r.success << " - " << r.errorMessage;

      for (const auto &reg : r.results) {
        qDebug() << reg.toHex();
      }
    });
    return;
  }

  _modbus->connect("10.211.55.3", 502);
}
