#include "modbusconnectdialog.hpp"

#include <QDebug>

#include "ui_modbusconnectdialog.h"

ModbusConnectDialog::ModbusConnectDialog(QWidget *parent) : QDialog(parent), ui(new Ui::ModbusConnectDialog) {
  _setupUI();
}

ModbusConnectDialog::~ModbusConnectDialog() {
  qDebug() << "delete";
  delete ui;
}

void ModbusConnectDialog::_setupUI() {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
}
