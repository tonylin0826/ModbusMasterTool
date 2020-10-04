#include "writesinglecoildialog.hpp"

#include <QIntValidator>

#include "mainwindow.hpp"
#include "ui_writesinglecoildialog.h"

WriteSingleCoilDialog::WriteSingleCoilDialog(QWidget *parent, quint16 startingAddress)
    : QDialog(parent), _ui(new Ui::WriteSingleCoilDialog), _onOffGroup(new QButtonGroup(this)) {
  _setupUI(startingAddress);
}

WriteSingleCoilDialog::~WriteSingleCoilDialog() { delete _ui; }

void WriteSingleCoilDialog::on_btnCancel_clicked() { close(); }

void WriteSingleCoilDialog::on_btnSend_clicked() {
  if (_ui->inputAddress->text().isEmpty() || _ui->inputSlaveId->text().isEmpty()) {
    return;
  }

  const auto address = _ui->inputAddress->text().toUShort();
  const auto slaveId = _ui->inputSlaveId->text().toUShort();
  const auto on = _onOffGroup->checkedId() == 1;
  const auto modbus = qobject_cast<MainWindow *>(parentWidget())->modbus();

  const auto reply = modbus->sendWriteRequest(
      QModbusDataUnit(QModbusDataUnit::RegisterType::Coils, address, QVector<quint16>({on})), slaveId);

  if (!reply) {
    _ui->labelStatus->setText("Failed - No reply");
    _ui->labelStatus->setStyleSheet("QLabel { color : Crimson; }");
    return;
  }

  if (reply->isFinished()) {
    delete reply;
    return;
  }

  _ui->labelStatus->setText("Sending");
  _ui->labelStatus->setStyleSheet("QLabel { color : DodgerBlue; }");

  QObject::connect(reply, &QModbusReply::finished, [=]() {
    if (reply->error() != QModbusDevice::NoError) {
      _ui->labelStatus->setText(QString("Failed - %1(code: %2)")
                                    .arg(reply->errorString(), QString::number(reply->rawResult().exceptionCode())));
      _ui->labelStatus->setStyleSheet("QLabel { color : Crimson; }");
      delete reply;
      return;
    }

    _ui->labelStatus->setText("Success");
    _ui->labelStatus->setStyleSheet("QLabel { color : Chartreuse; }");
    delete reply;
  });
}

void WriteSingleCoilDialog::_setupUI(quint16 startingAddress) {
  setAttribute(Qt::WA_DeleteOnClose);
  _ui->setupUi(this);
  setWindowTitle("Write Single Coil");

  _ui->inputAddress->setValidator(new QIntValidator(0, 65535, this));
  _ui->inputAddress->setText(QString::number(startingAddress));

  _ui->inputSlaveId->setText("1");
  _ui->inputSlaveId->setValidator(new QIntValidator(0, 255, this));

  _onOffGroup->addButton(_ui->btnOff, 0);
  _onOffGroup->addButton(_ui->btnOn, 1);

  _ui->btnOn->setChecked(true);
}
