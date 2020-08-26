#ifndef MODBUSSUBWINDOW_HPP
#define MODBUSSUBWINDOW_HPP

#include <QMdiSubWindow>

class ModbusSubWindow : public QMdiSubWindow {
  Q_OBJECT
 public:
  explicit ModbusSubWindow(QWidget *parent = nullptr);

 signals:
};

#endif  // MODBUSSUBWINDOW_HPP
