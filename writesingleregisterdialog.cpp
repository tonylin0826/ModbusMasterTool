#include "writesingleregisterdialog.hpp"

#include <QDebug>
#include <QIntValidator>
#include <QModbusReply>

#include "ui_writesingleregisterdialog.h"

WriteSingleRegisterDialog::WriteSingleRegisterDialog(MainWindow *parent)
    : QDialog(parent), ui(new Ui::WriteSingleRegisterDialog) {
  _setupUI();
}

WriteSingleRegisterDialog::~WriteSingleRegisterDialog() { delete ui; }

void WriteSingleRegisterDialog::_setupUI() {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Write Single Register");

  ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
}

void WriteSingleRegisterDialog::on_btnSend_clicked() {
  if (ui->inputAddress->text().isEmpty() || ui->inputValue->text().isEmpty()) {
    return;
  }

  const auto address = ui->inputAddress->text().toUShort();
  const auto value = ui->inputValue->text().toUShort();

  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();

  const auto reply = modbus->sendWriteRequest(
      QModbusDataUnit(QModbusDataUnit::HoldingRegisters, address, QVector<quint16>({value})), 1);

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
