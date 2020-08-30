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
    : QMdiSubWindow(parent),
      _ui(new Ui::ModbusSubWindow),
      _options({.type = type, .address = address, .count = count}) {
  _setupUi();
}

ModbusSubWindow::ModbusSubWindow(QWidget *parent, ModbusSubWindowOptions options)
    : QMdiSubWindow(parent), _ui(new Ui::ModbusSubWindow), _options(options) {
  _setupUi();
}

ModbusSubWindow::~ModbusSubWindow() { delete _ui; }

void ModbusSubWindow::_setupUi() {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowFlags(Qt::SubWindow);

  _ui->setupUi(this);
  setWidget(_ui->frame);

  const QString titles[4] = {"Coils", "Discrete Inputs", "Input Registers", "Holding Registers"};
  setWindowTitle(titles[_options.type]);

  _ui->tableWidget->setRowCount(_options.count);
  _ui->tableWidget->verticalHeader()->hide();

  for (quint16 i = 0; i < _options.count; i++) {
    auto item = new QTableWidgetItem(QString::number(_options.address + i));
    auto itemValue = new QTableWidgetItem(QString::number(0));
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    itemValue->setFlags(item->flags() ^ Qt::ItemIsEditable);
    itemValue->setTextAlignment(Qt::AlignRight);
    item->setTextAlignment(Qt::AlignCenter);
    _ui->tableWidget->setItem(i, 0, item);
    _ui->tableWidget->setItem(i, 1, itemValue);
  }
}
