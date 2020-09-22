#include "modbussubwindow.hpp"

#include <QAction>
#include <QApplication>
#include <QIntValidator>
#include <QMenu>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QString>
#include <QStyle>
#include <QtDebug>
#include <QtMath>

#include "ui_modbussubwindow.h"

ModbusSubWindow::ModbusSubWindow(QWidget *parent, Modbus::RegisterType type, quint16 address, quint16 count)
    : ModbusSubWindow(parent, {.type = type, .address = address, .count = count}) {}

ModbusSubWindow::ModbusSubWindow(QWidget *parent, ModbusSubWindowOptions options)
    : QMdiSubWindow(parent), _ui(new Ui::ModbusSubWindow), _options(options) {
  _setupUi();

  _ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  qDebug() << "ModbusSubWindow";
  QObject::connect(_ui->tableWidget, &QTableWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
    qDebug() << "pos " << pos;
    QTableWidgetItem *item = _ui->tableWidget->itemAt(pos);
    if (item) {
      qDebug() << item->text();
    }
  });
}

ModbusSubWindow::~ModbusSubWindow() { delete _ui; }

ModbusSubWindowOptions ModbusSubWindow::options() { return _options; }

void ModbusSubWindow::updateOptions(ModbusSubWindowOptions option) {
  Q_UNUSED(option);
  // TODO: impl
}

void ModbusSubWindow::updateValues(QVector<QByteArray> values) {
  if (values.size() != _options.count) {
    qDebug() << "size not match - " << values.size() << ", " << _options.count;
    return;
  }

  _ui->tableWidget->updateValues(values);
}

void ModbusSubWindow::_setupUi() {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(Qt::SubWindow);

  _ui->setupUi(this);
  setWidget(_ui->frame);

  const QString titles[4] = {"Coils", "Discrete Inputs", "Input Registers", "Holding Registers"};
  setWindowTitle(titles[_options.type]);

  _ui->tableWidget->setRegisterRange(_options.address, _options.count);
}
