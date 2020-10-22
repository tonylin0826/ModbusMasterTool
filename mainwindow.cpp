#include "mainwindow.hpp"

#include <QAbstractSocket>
#include <QDebug>
#include <QIntValidator>
#include <QLabel>
#include <QModbusTcpClient>
#include <QObject>
#include <QRegularExpression>

#include "addmodbusregisterdialog.hpp"
#include "modbusconnectdialog.hpp"
#include "modbussubwindow.hpp"
#include "ui_mainwindow.h"
#include "writemultiplecoilsdialog.hpp"
#include "writemutipleregistersdialog.hpp"
#include "writesinglecoildialog.hpp"
#include "writesingleregisterdialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _modbus(nullptr), _pollTimer(new QTimer(this)) {
  _setupUI();

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
    QObject::connect(&action1, &QAction::triggered, this, [=]() { _showCreateNewRegisterWindow(clickPos); });

    contextMenu.addAction(&action1);
    contextMenu.exec(_ui->mdiArea->mapToGlobal(clickPos));
    return true;
  }

  return false;
}

void MainWindow::_setupUI() {
  _ui->setupUi(this);

  _updateConnectionState(QModbusDevice::State::UnconnectedState);

  _ui->mdiArea->installEventFilter(this);
  _ui->mdiArea->setBackground(QBrush(QColor(60, 63, 65)));
  installEventFilter(this);
}

void MainWindow::_startPolling(int windowIndex) {
  const auto nextRound = [=]() { QTimer::singleShot(1000, this, [=]() { _startPolling(0); }); };
  const auto nextWindow = [=]() { QTimer::singleShot(0, this, [=]() { _startPolling(windowIndex + 1); }); };

  if (_modbus == nullptr) {
    qWarning() << "_startPolling modbus not ready";
    nextRound();
    return;
  }

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

  const auto reply =
      _modbus->sendReadRequest(QModbusDataUnit(options.type, options.address, options.count), options.slaveId);
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
      delete reply;
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

    delete reply;
  });
}

void MainWindow::_showCreateNewRegisterWindow(const QPoint &clickPos) {
  const auto dialog = new AddModbusRegisterDialog(this);
  QObject::connect(dialog, &AddModbusRegisterDialog::oked, this,
                   [=](QModbusDataUnit::RegisterType type, quint16 address, quint16 count, quint8 slaveId) {
                     const auto sub = new ModbusSubWindow(
                         this, {.type = type, .slaveId = slaveId, .address = address, .count = count});

                     QObject::connect(sub, &ModbusSubWindow::registerClicked, this,
                                      [=](QModbusDataUnit::RegisterType type, quint16 address) -> void {
                                        qDebug() << "register clicked" << type << address;

                                        if (type == QModbusDataUnit::RegisterType::HoldingRegisters) {
                                          const auto dialog = new WriteSingleRegisterDialog(this, address);
                                          dialog->exec();
                                          return;
                                        }

                                        if (type == QModbusDataUnit::RegisterType::Coils) {
                                          const auto dialog = new WriteSingleCoilDialog(this, address);
                                          dialog->exec();
                                          return;
                                        }
                                      });

                     QObject::connect(sub, &ModbusSubWindow::writeMutipleRegisterActionClicked, this, [=]() -> void {
                       const auto dialog = new WriteMutipleRegistersDialog(this, 0);
                       dialog->exec();
                     });

                     QObject::connect(sub, &ModbusSubWindow::writeMutipleCoilActionClicked, this, [=]() -> void {
                       const auto dialog = new WriteMultipleCoilsDialog(this, 0);
                       dialog->exec();
                     });

                     QObject::connect(sub, &ModbusSubWindow::closed, this,
                                      [=](ModbusSubWindow *ptr) -> void { _subwindows.removeOne(ptr); });

                     _ui->mdiArea->addSubWindow(sub);
                     _subwindows.push_back(sub);

                     sub->setGeometry(clickPos.x(), clickPos.y(), sub->width(), sub->height());
                     sub->show();
                   });
  dialog->show();
}

void MainWindow::_updateConnectionState(int state) {
  qDebug() << "state" << state;
  switch (state) {
    case QModbusDevice::State::ConnectedState:
      _ui->labelConnectionStatus->setText("Connected");
      _ui->actionConnect->setDisabled(true);
      _ui->actionDisconnect->setDisabled(false);
      break;
    case QModbusDevice::State::UnconnectedState:
      _ui->labelConnectionStatus->setText("Disconnected");
      _ui->actionConnect->setDisabled(false);
      _ui->actionDisconnect->setDisabled(true);
      break;
    case QModbusDevice::State::ConnectingState:
      _ui->labelConnectionStatus->setText("Connecting");
      _ui->actionConnect->setDisabled(true);
      _ui->actionDisconnect->setDisabled(false);
      break;
    default:
      _ui->labelConnectionStatus->setText("Disconnected");
      _ui->actionConnect->setDisabled(false);
      _ui->actionDisconnect->setDisabled(true);
      break;
  }
}

void MainWindow::on_actionConnect_triggered() {
  const auto dialog = new ModbusConnectDialog(this);
  QObject::connect(dialog, &ModbusConnectDialog::modbusDeviceGenerated, this, [=](QModbusClient *modbus) {
    _modbus = modbus;

    QObject::connect(_modbus, &QModbusClient::errorOccurred, this,
                     [=](QModbusDevice::Error e) { qDebug() << e << _modbus->errorString(); });

    QObject::connect(_modbus, &QModbusClient::stateChanged, this, &MainWindow::_updateConnectionState);
  });
  dialog->exec();
}

void MainWindow::on_actionDisconnect_triggered() {
  if (!_modbus) {
    return;
  }

  _modbus->disconnectDevice();

  delete _modbus;
  _modbus = nullptr;
}

void MainWindow::on_actionNew_triggered() { _showCreateNewRegisterWindow(QPoint(0, 0)); }
