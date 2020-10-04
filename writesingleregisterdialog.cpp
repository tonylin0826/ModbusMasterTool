#include "writesingleregisterdialog.hpp"

#include <QDebug>
#include <QIntValidator>
#include <QModbusReply>

#include "ui_writesingleregisterdialog.h"

WriteSingleRegisterDialog::WriteSingleRegisterDialog(MainWindow *parent, quint16 startingAddress)
    : QDialog(parent), ui(new Ui::WriteSingleRegisterDialog) {
  _setupUI(startingAddress);
}

WriteSingleRegisterDialog::~WriteSingleRegisterDialog() { delete ui; }

void WriteSingleRegisterDialog::_setupUI(quint16 startingAddress) {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Write Single Register");

  ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
  ui->inputAddress->setText(QString::number(startingAddress));

  ui->inputSlaveId->setText("1");
  ui->inputSlaveId->setValidator(new QIntValidator(0, 255, this));

  ui->inputValue->setText("0");
  ui->inputValue->setFocus();
}

void WriteSingleRegisterDialog::on_btnSend_clicked() {
  if (ui->inputAddress->text().isEmpty() || ui->inputValue->text().isEmpty() || ui->inputSlaveId->text().isEmpty()) {
    return;
  }

  const auto address = ui->inputAddress->text().toUShort();
  const auto value = ui->inputValue->text().toUShort();
  const auto slaveId = ui->inputSlaveId->text().toUShort();

  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();

  const auto reply = modbus->sendWriteRequest(
      QModbusDataUnit(QModbusDataUnit::HoldingRegisters, address, QVector<quint16>({value})), slaveId);

  if (!reply) {
    ui->labelStatus->setText("Failed - No reply");
    ui->labelStatus->setStyleSheet("QLabel { color : Crimson; }");
    return;
  }

  if (reply->isFinished()) {
    delete reply;
    return;
  }

  ui->labelStatus->setText("Sending");
  ui->labelStatus->setStyleSheet("QLabel { color : DodgerBlue; }");

  QObject::connect(reply, &QModbusReply::finished, [=]() {
    if (reply->error() != QModbusDevice::NoError) {
      ui->labelStatus->setText(QString("Failed - %1(code: %2)")
                                   .arg(reply->errorString(), QString::number(reply->rawResult().exceptionCode())));
      ui->labelStatus->setStyleSheet("QLabel { color : Crimson; }");
      delete reply;
      return;
    }

    ui->labelStatus->setText("Success");
    ui->labelStatus->setStyleSheet("QLabel { color : Chartreuse; }");
    delete reply;
  });
}

void WriteSingleRegisterDialog::on_btnCancel_clicked() { close(); }
