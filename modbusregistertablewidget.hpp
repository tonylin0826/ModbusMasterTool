#ifndef MODBUSREGISTERTABLEWIDGET_HPP
#define MODBUSREGISTERTABLEWIDGET_HPP

#include <QAction>
#include <QByteArray>
#include <QDebug>
#include <QMenu>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtEndian>

enum ModbusRegisterDisplayOption {
  UnsignedIntegerLE,
  SignedIntegerLE,
  UnsignedShortLE,
  SignedShortLE,
  IEEEFloatLE,
  IEEEDoubleLE,
  UnsignedIntegerBE,
  SignedIntegerBE,
  UnsignedShortBE,
  SignedShortBE,
  IEEEFloatBE,
  IEEEDoubleBE,
  HEX
};

class ModbusRegisterTalbleWidgetItem : public QTableWidgetItem {
 public:
  ModbusRegisterTalbleWidgetItem(ModbusRegisterDisplayOption option = ModbusRegisterDisplayOption::HEX)
      : QTableWidgetItem(), _displayOption(option) {}

  void setDisplayOption(ModbusRegisterDisplayOption option) { _displayOption = option; }

  void setValue(const QByteArray &value) {
    switch (_displayOption) {
      case ModbusRegisterDisplayOption::UnsignedShortLE:
        setText(QString::number(qFromLittleEndian<quint16>(value.data())));
        break;
      case ModbusRegisterDisplayOption::SignedShortLE:
        setText(QString::number(qFromLittleEndian<qint16>(value.data())));
        break;
      case ModbusRegisterDisplayOption::UnsignedIntegerLE:
        setText(QString::number(qFromLittleEndian<quint32>(value.data())));
        break;
      case ModbusRegisterDisplayOption::SignedIntegerLE:
        setText(QString::number(qFromLittleEndian<qint32>(value.data())));
        break;
      case ModbusRegisterDisplayOption::IEEEFloatLE:
        setText(QString::number(qFromLittleEndian<float>(value.data())));
        break;
      case ModbusRegisterDisplayOption::IEEEDoubleLE:
        setText(QString::number(qFromLittleEndian<double>(value.data())));
        break;

      case ModbusRegisterDisplayOption::UnsignedShortBE:
        setText(QString::number(qFromBigEndian<quint16>(value.data())));
        break;
      case ModbusRegisterDisplayOption::SignedShortBE:
        setText(QString::number(qFromBigEndian<qint16>(value.data())));
        break;
      case ModbusRegisterDisplayOption::UnsignedIntegerBE:
        setText(QString::number(qFromBigEndian<quint32>(value.data())));
        break;
      case ModbusRegisterDisplayOption::SignedIntegerBE:
        setText(QString::number(qFromBigEndian<qint32>(value.data())));
        break;
      case ModbusRegisterDisplayOption::IEEEFloatBE:
        setText(QString::number(qFromBigEndian<float>(value.data())));
        break;
      case ModbusRegisterDisplayOption::IEEEDoubleBE:
        setText(QString::number(qFromBigEndian<double>(value.data())));
        break;
      case ModbusRegisterDisplayOption::HEX:
      default:
        setText(value.toHex());
    }
  }

  quint16 displayOptionWordSize() {
    switch (_displayOption) {
      case ModbusRegisterDisplayOption::UnsignedIntegerBE:
      case ModbusRegisterDisplayOption::SignedIntegerBE:
      case ModbusRegisterDisplayOption::UnsignedIntegerLE:
      case ModbusRegisterDisplayOption::SignedIntegerLE:
      case ModbusRegisterDisplayOption::IEEEFloatLE:
      case ModbusRegisterDisplayOption::IEEEFloatBE:
        return 2;
      case ModbusRegisterDisplayOption::IEEEDoubleLE:
      case ModbusRegisterDisplayOption::IEEEDoubleBE:
        return 4;
      default:
        return 1;
    }
  }

 private:
  ModbusRegisterDisplayOption _displayOption;
};

class ModbusDisplayAction : public QAction {
  Q_OBJECT
 public:
  ModbusDisplayAction(ModbusRegisterDisplayOption display, const QString &text, QObject *parent = nullptr)
      : QAction(text, parent), _displayOption(display) {}

  ModbusRegisterDisplayOption displayOption() { return _displayOption; }

 private:
  ModbusRegisterDisplayOption _displayOption;
};

class ModbusRegisterTableWidget : public QTableWidget {
  Q_OBJECT
 public:
  ModbusRegisterTableWidget(QWidget *parent = nullptr);

  void updateValues(const QVector<QByteArray> &values);

  void setItemValue(int row, int column, const QByteArray &value);

  void setRegisterRange(quint16 address, quint16 count);

  QSize realSize();

 signals:
  void registerClicked();

  void writeMutipleRegisterActionClicked();

  void writeMutipleCoilActionClicked();

 private:
  QMenu *_menu;

  void _setup();
};

#endif  // MODBUSREGISTERTABLEWIDGET_HPP
