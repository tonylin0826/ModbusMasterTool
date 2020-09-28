#include "mainwindow.hpp"

#include <QAbstractSocket>
#include <QDebug>
#include <QIntValidator>
#include <QLabel>
#include <QObject>
#include <QRegularExpression>

#include "addmodbusregisterdialog.hpp"
#include "modbussubwindow.hpp"
#include "ui_mainwindow.h"
#include "writesingleregisterdialog.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      _ui(new Ui::MainWindow),
      _modbus(new Modbus::ModbusTcp(this, 1)),
      _pollTimer(new QTimer(this)) {
  _setupUI();

  QObject::connect(_modbus, &Modbus::ModbusTcp::errorOccurred, this,
                   [=](QAbstractSocket::SocketError error) { qWarning() << "socket error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::modbusErrorOccurred, this,
                   [=](Modbus::ModbusErrorCode error) { qWarning() << "modbus error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::disconnected, this, [=]() {
    _ui->labelConnectionStatus->setText("Disconnected");
    _ui->btnConnect->setText("Connect");
  });

  QObject::connect(_modbus, &Modbus::ModbusTcp::connected, this, [=]() {
    _ui->labelConnectionStatus->setText("Connected");
    _ui->btnConnect->setText("Disconnect");
  });

  _startPolling();
}

MainWindow::~MainWindow() {
  _modbus->disconnect();
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
                       [=](Modbus::RegisterType type, quint16 address, quint16 count) {
                         const auto sub = new ModbusSubWindow(this, {.type = type, .address = address, .count = count});

                         QObject::connect(sub, &ModbusSubWindow::registerClicked, this,
                                          [=](Modbus::RegisterType type, quint16 address) -> void {
                                            qDebug() << "register clicked" << type << address;

                                            if (type == Modbus::RegisterType::HoldingRegisters) {
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
  //  _ui->inputPort->setValidator(new QRegularExpressionValidator(IpRegex, this));

  _ui->mdiArea->installEventFilter(this);
  installEventFilter(this);
}

void MainWindow::_startPolling(int windowIndex) {
  const auto nextRound = [=]() { QTimer::singleShot(1000, this, [=]() { _startPolling(0); }); };
  if (!_modbus->isConnected()) {
    nextRound();
    return;
  }

  if (windowIndex >= _subwindows.size()) {
    nextRound();
    return;
  }

  const auto options = _subwindows[windowIndex]->options();

  std::function<bool(const quint16, const quint16, Modbus::ModbusReadCallback cb)> readFunc;

  switch (options.type) {
    case Modbus::RegisterType::Coils:
      readFunc = std::bind(&Modbus::ModbusTcp::readCoils, _modbus, std::placeholders::_1, std::placeholders::_2,
                           std::placeholders::_3);
      break;
    case Modbus::RegisterType::DiscreteInputs:
      readFunc = std::bind(&Modbus::ModbusTcp::readDiscreteInputs, _modbus, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
      break;
    case Modbus::RegisterType::InputRegisters:
      readFunc = std::bind(&Modbus::ModbusTcp::readInputRegisters, _modbus, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
      break;
    case Modbus::RegisterType::HoldingRegisters:
      readFunc = std::bind(&Modbus::ModbusTcp::readHoldingRegisters, _modbus, std::placeholders::_1,
                           std::placeholders::_2, std::placeholders::_3);
      break;
    default:
      nextRound();
      return;
  }

  const auto success = readFunc(options.address, options.count, [=](Modbus::ModbusReadResult result) {
    qDebug() << windowIndex << "read" << options.type << "success - " << result.success;

    if (result.success) {
      _subwindows[windowIndex]->updateValues(result.results);
    }

    _startPolling(windowIndex + 1);
  });

  if (!success) {
    qWarning() << windowIndex << "read" << options.type << "success - " << success;
    nextRound();
  }
}

void MainWindow::on_actiontest_triggered() {}

void MainWindow::on_btnConnect_clicked() {
  if (_modbus->isConnected()) {
    _modbus->disconnect();
    return;
  }

  qDebug() << _ui->inputPort->hasAcceptableInput() << " " << _ui->inputIp->hasAcceptableInput();
  if (!_ui->inputPort->hasAcceptableInput() || !_ui->inputIp->hasAcceptableInput()) {
    // TODO: show error message
    qWarning() << "invalid input";
    return;
  }

  _ui->btnConnect->setDisabled(true);
  _modbus->connect(_ui->inputIp->text(), _ui->inputPort->text().toUShort());
}
