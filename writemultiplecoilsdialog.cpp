#include "writemultiplecoilsdialog.hpp"

#include "ui_writemultiplecoilsdialog.h"

WriteMultipleCoilsDialog::WriteMultipleCoilsDialog(MainWindow *parent, quint16 startingAddress)
    : QDialog(parent), ui(new Ui::WriteMultipleCoilsDialog) {
  _setupUI(startingAddress);
}

WriteMultipleCoilsDialog::~WriteMultipleCoilsDialog() { delete ui; }

void WriteMultipleCoilsDialog::_setupUI(quint16 startingAddress) {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Write Multiple Coils");

  ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
  ui->inputAddress->setText(QString::number(startingAddress));

  ui->inputSlaveId->setText("1");
  ui->inputSlaveId->setValidator(new QIntValidator(0, 255, this));

  ui->inputCount->setText("1");
  ui->inputCount->setValidator(new QIntValidator(1, 2000, this));
  ui->inputCount->setFocus();

  _updateTableSettings();
}

void WriteMultipleCoilsDialog::_updateTableSettings() {
  ui->tableWidget->setRowCount(ui->inputCount->text().toUInt());
  for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
    QLineEdit *edit = new QLineEdit();
    edit->setText("0");
    edit->setAlignment(Qt::AlignRight);
    edit->setValidator(new QIntValidator(0, 11, this));
    ui->tableWidget->setCellWidget(i, 0, edit);
  }
}

QVector<quint16> WriteMultipleCoilsDialog::_getValues() {
  QVector<quint16> r;

  for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
    const auto text = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(i, 0))->text();
    r.push_back(text.isEmpty() ? 0 : (text == "1" ? 1 : 0));
  }

  return r;
}

void WriteMultipleCoilsDialog::on_btnSend_clicked() {
  if (ui->inputAddress->text().isEmpty() || ui->inputSlaveId->text().isEmpty()) {
    return;
  }

  const auto address = ui->inputAddress->text().toUShort();
  const auto slaveId = ui->inputSlaveId->text().toUShort();
  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();

  if (!modbus) {
    ui->labelStatus->setText("Failed - Modbus not connected");
    ui->labelStatus->setStyleSheet("QLabel { color : Crimson; }");
    return;
  }

  const auto reply = modbus->sendWriteRequest(QModbusDataUnit(QModbusDataUnit::Coils, address, _getValues()), slaveId);

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

void WriteMultipleCoilsDialog::on_btnCancel_clicked() { close(); }

void WriteMultipleCoilsDialog::on_inputCount_editingFinished() { _updateTableSettings(); }
