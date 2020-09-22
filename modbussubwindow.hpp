#ifndef MODBUSSUBWINDOW_HPP
#define MODBUSSUBWINDOW_HPP

#include <QMdiSubWindow>
#include <QResizeEvent>

#include "modbus.hpp"

namespace Ui {
class ModbusSubWindow;
}

typedef struct {
  quint16 address;
  quint16 count;
  Modbus::RegisterType type;
} ModbusSubWindowOptions;

class ModbusSubWindow : public QMdiSubWindow {
  Q_OBJECT

 public:
  ModbusSubWindow(QWidget *parent = nullptr, Modbus::RegisterType type = Modbus::RegisterType::Coils,
                  quint16 address = 0, quint16 count = 10);

  ModbusSubWindow(QWidget *parent = nullptr,
                  ModbusSubWindowOptions options = {.address = 0, .count = 10, .type = Modbus::RegisterType::Coils});

  ~ModbusSubWindow();

  ModbusSubWindowOptions options();

  void updateOptions(ModbusSubWindowOptions option);

  void updateValues(QVector<QByteArray> values);

 signals:
  void closed(ModbusSubWindow *windowPtr);

 private:
  Ui::ModbusSubWindow *_ui;

  ModbusSubWindowOptions _options;

  void _setupUi();

 protected:
  void closeEvent(QCloseEvent *closeEvent) override;
};

#endif  // MODBUSSUBWINDOW_HPP
