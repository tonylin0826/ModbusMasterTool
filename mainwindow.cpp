#include "mainwindow.hpp"

#include <QAbstractSocket>
#include <QDebug>
#include <QObject>

#include "modbussubwindow.hpp"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _modbus(new Modbus::ModbusTcp(this, 1)), _modbusConnected(false) {
  _setupUI();

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

void MainWindow::resizeEvent(QResizeEvent *event) {
  _ui->mdiArea->setGeometry(0, 22, event->size().width(), event->size().height());
}

void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow *arg1) {}

void MainWindow::_setupUI() {
  _ui->setupUi(this);

  _ui->mdiArea->setGeometry(0, 22, width(), height());

  const auto sub = new ModbusSubWindow(this);
  _ui->mdiArea->addSubWindow(sub);

  sub->show();
}
