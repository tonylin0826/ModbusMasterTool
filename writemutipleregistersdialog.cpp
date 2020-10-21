#include "writemutipleregistersdialog.hpp"

#include <QDebug>
#include <QModbusDataUnit>
#include <QModbusReply>

#include "hexvalidator.hpp"
#include "ui_writemutipleregistersdialog.h"

WriteMutipleRegistersDialog::WriteMutipleRegistersDialog(MainWindow *parent, quint16 startingAddress)
    : QDialog(parent), ui(new Ui::WriteMutipleRegistersDialog), _format(Hex) {
  _setupUI(startingAddress);
}

WriteMutipleRegistersDialog::~WriteMutipleRegistersDialog() { delete ui; }

void WriteMutipleRegistersDialog::_setupUI(quint16 startingAddress) {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Write Multiple Registers");

  ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
  ui->inputAddress->setText(QString::number(startingAddress));

  ui->inputSlaveId->setText("1");
  ui->inputSlaveId->setValidator(new QIntValidator(0, 255, this));

  ui->inputCount->setText("1");
  ui->inputCount->setValidator(new QIntValidator(1, 123, this));
  ui->inputCount->setFocus();

  _updateTableSettings();
}

void WriteMutipleRegistersDialog::_updateTableSettings() {
  ui->tableWidget->setRowCount(ui->inputCount->text().toUInt());

  switch (_format) {
    case WriteFormat::Hex:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QLineEdit *edit = new QLineEdit();
        edit->setText("0000");
        edit->setAlignment(Qt::AlignRight);
        edit->setInputMask("HHHH");
        ui->tableWidget->setCellWidget(i, 0, edit);
      }
      break;
    case WriteFormat::Int16:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QLineEdit *edit = new QLineEdit();
        edit->setText("0");
        edit->setAlignment(Qt::AlignRight);
        edit->setValidator(new QIntValidator(-32768, 32767, this));
        ui->tableWidget->setCellWidget(i, 0, edit);
      }
      break;
    case WriteFormat::Uint16:
    default:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QLineEdit *edit = new QLineEdit();
        edit->setText("0");
        edit->setAlignment(Qt::AlignRight);
        edit->setValidator(new QIntValidator(0, 65535, this));
        ui->tableWidget->setCellWidget(i, 0, edit);
      }
      break;
  }
}

QVector<quint16> WriteMutipleRegistersDialog::_getValues() {
  QVector<quint16> r;

  switch (_format) {
    case WriteFormat::Hex:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        const auto text = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(i, 0))->text();
        bool ok = false;
        quint16 value = text.toUShort(&ok, 16);
        r.push_back(text.isEmpty() ? 0 : (ok ? value : 0));
      }
      break;
    case WriteFormat::Int16:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        const auto text = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(i, 0))->text();
        r.push_back(text.isEmpty() ? 0 : text.toShort());
      }
      break;
    case WriteFormat::Uint16:
    default:
      for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        const auto text = qobject_cast<QLineEdit *>(ui->tableWidget->cellWidget(i, 0))->text();
        r.push_back(text.isEmpty() ? 0 : text.toUShort());
      }
      break;
  }

  return r;
}

void WriteMutipleRegistersDialog::on_btnSend_clicked() {
  if (ui->inputAddress->text().isEmpty() || ui->inputSlaveId->text().isEmpty()) {
    return;
  }

  const auto address = ui->inputAddress->text().toUShort();
  const auto slaveId = ui->inputSlaveId->text().toUShort();
  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();

  const auto reply =
      modbus->sendWriteRequest(QModbusDataUnit(QModbusDataUnit::HoldingRegisters, address, _getValues()), slaveId);

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

void WriteMutipleRegistersDialog::on_btnCancel_clicked() { close(); }

void WriteMutipleRegistersDialog::on_inputCount_editingFinished() { _updateTableSettings(); }

void WriteMutipleRegistersDialog::on_selectFormat_currentIndexChanged(int index) {
  _format = static_cast<WriteFormat>(index);
  _updateTableSettings();
}
