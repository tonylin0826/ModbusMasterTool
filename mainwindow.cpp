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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), _ui(new Ui::MainWindow), _modbus(new Modbus::ModbusTcp(this, 1)) {
  _setupUI();

  QObject::connect(_modbus, &Modbus::ModbusTcp::errorOccurred, this,
                   [=](QAbstractSocket::SocketError error) { qDebug() << "socket error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::modbusErrorOccurred, this,
                   [=](Modbus::ModbusErrorCode error) { qDebug() << "modbus error - " << error; });

  QObject::connect(_modbus, &Modbus::ModbusTcp::disconnected, this, [=]() {});

  QObject::connect(_modbus, &Modbus::ModbusTcp::connected, this, [=]() {});
}

MainWindow::~MainWindow() {
  _modbus->disconnect();
  delete _ui;
}

void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow *arg1) {}

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
                         _ui->mdiArea->addSubWindow(sub);

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

void MainWindow::on_actiontest_triggered() {}

void MainWindow::on_btnConnect_clicked() {
  if (_modbus->isConnected()) {
    _modbus->disconnect();
    return;
  }

  qDebug() << _ui->inputPort->hasAcceptableInput() << " " << _ui->inputIp->hasAcceptableInput();
  if (!_ui->inputPort->hasAcceptableInput() || !_ui->inputIp->hasAcceptableInput()) {
    // TODO: show error message
    qDebug() << "invalid input";
    return;
  }

  _modbus->connect(_ui->inputIp->text(), _ui->inputPort->text().toUShort());
}
