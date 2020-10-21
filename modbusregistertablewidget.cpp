#include "modbusregistertablewidget.hpp"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QPoint>
#include <typeinfo>

#include "writemutipleregistersdialog.hpp"

ModbusRegisterTableWidget::ModbusRegisterTableWidget(QWidget *parent) : QTableWidget(parent) { _setup(); }

void ModbusRegisterTableWidget::updateValues(const QVector<QByteArray> &values) {
  const auto rows = rowCount();
  if (rows != values.size()) {
    qDebug() << "updateValues size not match";
    return;
  }

  for (quint16 i = 0; i < rows;) {
    const auto tableItem = static_cast<ModbusRegisterTalbleWidgetItem *>(item(i, 1));
    const auto wordSize = tableItem->displayOptionWordSize();

    QByteArray b;
    for (quint16 w = 0; w < wordSize && (w + i) < rows; w++) {
      b += values[w + i];

      if (w != 0) {
        const auto tableItemTmp = static_cast<ModbusRegisterTalbleWidgetItem *>(item(w + i, 1));
        tableItemTmp->setValue("");
      }
    }

    tableItem->setValue(b);
    i += wordSize;
  }
}

void ModbusRegisterTableWidget::setItemValue(int row, int column, const QByteArray &value) {
  static_cast<ModbusRegisterTalbleWidgetItem *>(item(row, column))->setValue(value);
}

void ModbusRegisterTableWidget::setRegisterRange(quint16 address, quint16 count) {
  setRowCount(count);

  for (quint16 i = 0; i < count; i++) {
    auto item = new ModbusRegisterTalbleWidgetItem(ModbusRegisterDisplayOption::UnsignedIntegerLE);
    auto itemValue = new ModbusRegisterTalbleWidgetItem();
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    itemValue->setFlags(item->flags() ^ Qt::ItemIsEditable);
    itemValue->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(QString::number(address + i));
    itemValue->setValue(QByteArray(1, 0));
    setItem(i, 0, item);
    setItem(i, 1, itemValue);
  }
}

QSize ModbusRegisterTableWidget::realSize() {
  int w = verticalHeader()->width() + 8;                        // +4 seems to be needed
  for (int i = 0; i < columnCount(); i++) w += columnWidth(i);  // seems to include gridline (on my machine)
  int h = horizontalHeader()->height() * 2 + 8;
  for (int i = 0; i < rowCount(); i++) h += rowHeight(i);
  return QSize(w, h);
}

void ModbusRegisterTableWidget::_setup() {
  setContextMenuPolicy(Qt::CustomContextMenu);
  verticalHeader()->hide();

  setEditTriggers(QAbstractItemView::NoEditTriggers);

  // setup menu
  _menu = new QMenu(tr("Context Menu"), this);

  const auto display = new QMenu(tr("Display"), this);
  const auto actionDisplayUint = new ModbusDisplayAction(ModbusRegisterDisplayOption::UnsignedIntegerLE,
                                                         tr("Unsigned Integer (Little Endian)"), this);
  const auto actionDisplayInt =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::SignedIntegerLE, tr("Signed Integer (Little Endian)"), this);
  const auto actionDisplayUShort =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::UnsignedShortLE, tr("Unsigned Short (Little Endian)"), this);
  const auto actionDisplayShort =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::SignedShortLE, tr("Signed Short (Little Endian)"), this);
  const auto actionDisplayIEEEFloat =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::IEEEFloatLE, tr("IEEE Float (Little Endian)"), this);
  const auto actionDisplayIEEEDouble =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::IEEEDoubleLE, tr("IEEE Double (Little Endian)"), this);

  const auto actionDisplayUintBE = new ModbusDisplayAction(ModbusRegisterDisplayOption::UnsignedIntegerLE,
                                                           tr("Unsigned Integer (Big Endian)"), this);
  const auto actionDisplayIntBE =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::SignedIntegerBE, tr("Signed Integer (Big Endian)"), this);
  const auto actionDisplayUShortBE =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::UnsignedShortBE, tr("Unsigned Short (Big Endian)"), this);
  const auto actionDisplayShortBE =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::SignedShortBE, tr("Signed Short (Big Endian)"), this);
  const auto actionDisplayIEEEFloatBE =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::IEEEFloatBE, tr("IEEE Float (Big Endian)"), this);
  const auto actionDisplayIEEEDoubleBE =
      new ModbusDisplayAction(ModbusRegisterDisplayOption::IEEEDoubleBE, tr("IEEE Double (Big Endian)"), this);

  const auto actionDisplayHex = new ModbusDisplayAction(ModbusRegisterDisplayOption::HEX, tr("HEX"), this);
  const auto actionClose = new QAction(tr("Close"), this);

  const auto writeFunctionMenu = new QMenu(tr("Write Functions"), this);
  const auto actionWriteMultipleRegisters = new QAction(tr("Write Mutiple Registers"), this);
  const auto actionWriteMultipleCoils = new QAction(tr("Write Mutiple Coils"), this);
  writeFunctionMenu->addAction(actionWriteMultipleRegisters);
  writeFunctionMenu->addAction(actionWriteMultipleCoils);

  display->addAction(actionDisplayUint);
  display->addAction(actionDisplayInt);
  display->addAction(actionDisplayUShort);
  display->addAction(actionDisplayShort);
  display->addAction(actionDisplayIEEEFloat);
  display->addAction(actionDisplayIEEEDouble);

  display->addAction(actionDisplayUintBE);
  display->addAction(actionDisplayIntBE);
  display->addAction(actionDisplayUShortBE);
  display->addAction(actionDisplayShortBE);
  display->addAction(actionDisplayIEEEFloatBE);
  display->addAction(actionDisplayIEEEDoubleBE);

  display->addAction(actionDisplayHex);

  _menu->addMenu(display);
  _menu->addMenu(writeFunctionMenu);
  _menu->addAction(actionClose);

  QObject::connect(this, &QTableWidget::customContextMenuRequested, this, [=](const QPoint &pos) {
    QTableWidgetItem *item = itemAt(pos);
    if (!item) {
      return;
    }
    qDebug() << _menu;
    const auto ac = _menu->exec(mapToGlobal(pos));
    qDebug() << (ac == actionClose);

    if (ac == actionClose) {
      close();
      return;
    }

    if (ac == actionWriteMultipleRegisters) {
      emit writeMutipleRegisterActionClicked();
      return;
    }

    if (ac == actionWriteMultipleCoils) {
      emit writeMutipleCoilActionClicked();
      return;
    }

    if (ac == actionDisplayUint || ac == actionDisplayInt || ac == actionDisplayUShort || ac == actionDisplayShort ||
        ac == actionDisplayIEEEFloat || ac == actionDisplayIEEEDouble || ac == actionDisplayHex ||
        ac == actionDisplayUintBE || ac == actionDisplayIntBE || ac == actionDisplayUShortBE ||
        ac == actionDisplayShortBE || ac == actionDisplayIEEEFloatBE || ac == actionDisplayIEEEDoubleBE) {
      static_cast<ModbusRegisterTalbleWidgetItem *>(item)->setDisplayOption(
          static_cast<ModbusDisplayAction *>(ac)->displayOption());
      return;
    }
  });
}
