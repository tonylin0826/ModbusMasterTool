#include "mainwindow.hpp"

#include <QAbstractSocket>
#include <QDebug>
#include <QIntValidator>
#include <QLabel>
#include <QModbusTcpClient>
#include <QObject>
#include <QRegularExpression>

#include "addmodbusregisterdialog.hpp"
#include "modbussubwindow.hpp"
#include "ui_mainwindow.h"
#include "writesingleregisterdialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _modbus(new QModbusTcpClient(this)), _pollTimer(new QTimer(this)) {
  _setupUI();

  QObject::connect(_modbus, &QModbusClient::errorOccurred, this,
                   [=](QModbusDevice::Error e) { qDebug() << e << _modbus->errorString(); });

  QObject::connect(_modbus, &QModbusClient::stateChanged, [=](int state) {
    qDebug() << "state" << state;
    switch (state) {
      case QModbusDevice::State::ConnectedState:
        _ui->labelConnectionStatus->setText("Connected");
        _ui->btnConnect->setText("Disconnect");
        break;
      case QModbusDevice::State::UnconnectedState:
        _ui->labelConnectionStatus->setText("Disconnected");
        _ui->btnConnect->setText("Connect");
        break;
      case QModbusDevice::State::ConnectingState:
        _ui->labelConnectionStatus->setText("Connecting");
        _ui->btnConnect->setText("Disconnect");
        break;
      default:
        _ui->labelConnectionStatus->setText("Disconnected");
        _ui->btnConnect->setText("Connect");
        break;
    }
  });

  _startPolling();
}

MainWindow::~MainWindow() {
  _modbus->disconnectDevice();
  delete _ui;
}

bool MainWindow::eventFilter(QObject *target, QEvent *event) {
  if (target == _ui->mdiArea && event->type() == QEvent::MouseButtonPress) {
    const auto mouseEvent = static_cast<QMouseEvent *>(event);
    if (mouseEvent->button() != Qt::MouseButton::RightButton) {
      return false;
    }

    const auto clickPos = mouseEvent->pos();

    QMenu contextMenu(tr("Context menu"), this);
    QAction action1("New register window", this);
    QObject::connect(&action1, &QAction::triggered, this, [=]() {
      const auto dialog = new AddModbusRegisterDialog(this);
      QObject::connect(dialog, &AddModbusRegisterDialog::oked, this,
                       [=](QModbusDataUnit::RegisterType type, quint16 address, quint16 count) {
                         const auto sub = new ModbusSubWindow(this, {.type = type, .address = address, .count = count});

                         QObject::connect(sub, &ModbusSubWindow::registerClicked, this,
                                          [=](QModbusDataUnit::RegisterType type, quint16 address) -> void {
                                            qDebug() << "register clicked" << type << address;

                                            if (type == QModbusDataUnit::RegisterType::HoldingRegisters) {
                                              const auto dialog = new WriteSingleRegisterDialog(this);

                                              dialog->exec();
                                            }
                                          });

                         QObject::connect(sub, &ModbusSubWindow::closed, this,
                                          [=](ModbusSubWindow *ptr) -> void { _subwindows.removeOne(ptr); });

                         _ui->mdiArea->addSubWindow(sub);
                         _subwindows.push_back(sub);

                         sub->setGeometry(clickPos.x(), clickPos.y(), sub->width(), sub->height());
                         sub->show();
                       });
      dialog->show();
    });

    contextMenu.addAction(&action1);
    contextMenu.exec(_ui->mdiArea->mapToGlobal(clickPos));
    return true;
  }

  return false;
}

void MainWindow::_setupUI() {
  _ui->setupUi(this);

  _ui->inputPort->setText("502");
  _ui->inputPort->setValidator(new QIntValidator(0, 65535, this));

  QString IpRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  QRegularExpression IpRegex("^" + IpRange + "(\\." + IpRange + ")" + "(\\." + IpRange + ")" + "(\\." + IpRange + ")$");

  _ui->inputIp->setText("0.0.0.0");
  _ui->inputIp->setText("10.211.55.3");
  //  _ui->inputPort->setValidator(new QRegularExpressionValidator(IpRegex, this));

  _ui->mdiArea->installEventFilter(this);
  installEventFilter(this);
}

void MainWindow::_startPolling(int windowIndex) {
  const auto nextRound = [=]() { QTimer::singleShot(1000, this, [=]() { _startPolling(0); }); };
  const auto nextWindow = [=]() { QTimer::singleShot(0, this, [=]() { _startPolling(windowIndex + 1); }); };

  if (_modbus->state() != QModbusClient::State::ConnectedState) {
    qWarning() << "_startPolling not connected";
    nextRound();
    return;
  }

  if (windowIndex >= _subwindows.size()) {
    qWarning() << "_startPolling last one";
    nextRound();
    return;
  }

  const auto options = _subwindows[windowIndex]->options();

  const auto *reply = _modbus->sendReadRequest(QModbusDataUnit(options.type, options.address, options.count), 1);
  if (!reply) {
    qWarning() << windowIndex << "read" << options.type << "failed";
    nextWindow();
    return;
  }

  if (reply->isFinished()) {
    delete reply;
    nextWindow();
    return;
  }

  QObject::connect(reply, &QModbusReply::finished, [=]() {
    if (reply->error() != QModbusDevice::NoError) {
      qDebug() << "Read response error:" << reply->errorString() << "(code:" << reply->rawResult().exceptionCode()
               << ")";
      nextWindow();
      return;
    }

    const QModbusDataUnit unit = reply->result();
    QVector<QByteArray> bytes;
    foreach (const quint16 &elem, reply->result().values()) {
      QByteArray ba;
      ba.append(quint8(elem >> 8)).append(quint8(elem));
      bytes.append(ba);
    }

    _subwindows[windowIndex]->updateValues(bytes);

    nextWindow();
  });
}

void MainWindow::on_actiontest_triggered() {}

void MainWindow::on_btnConnect_clicked() {
  if (_modbus->state() == QModbusClient::ConnectedState || _modbus->state() == QModbusClient::ConnectingState) {
    return;
  }

  qDebug() << _ui->inputPort->hasAcceptableInput() << " " << _ui->inputIp->hasAcceptableInput();
  if (!_ui->inputPort->hasAcceptableInput() || !_ui->inputIp->hasAcceptableInput()) {
    // TODO: show error message
    qWarning() << "invalid input";
    return;
  }

  _ui->btnConnect->setDisabled(true);

  _modbus->setConnectionParameter(QModbusDevice::NetworkPortParameter, QVariant(_ui->inputPort->text().toUShort()));
  _modbus->setConnectionParameter(QModbusDevice::NetworkAddressParameter, QVariant(_ui->inputIp->text()));

  _modbus->setTimeout(3000);
  _modbus->setNumberOfRetries(3);

  _modbus->connectDevice();
}
