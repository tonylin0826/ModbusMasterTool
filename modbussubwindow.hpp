#ifndef MODBUSSUBWINDOW_HPP
#define MODBUSSUBWINDOW_HPP

#include <QMdiSubWindow>
#include <QModbusDataUnit>
#include <QResizeEvent>

namespace Ui {
class ModbusSubWindow;
}

typedef struct {
  QModbusDataUnit::RegisterType type;
  quint16 address;
  quint16 count;
} ModbusSubWindowOptions;

class ModbusSubWindow : public QMdiSubWindow {
  Q_OBJECT

 public:
  ModbusSubWindow(QWidget *parent = nullptr, QModbusDataUnit::RegisterType type = QModbusDataUnit::RegisterType::Coils,
                  quint16 address = 0, quint16 count = 10);

  ModbusSubWindow(QWidget *parent = nullptr,
                  ModbusSubWindowOptions options = {
                      .type = QModbusDataUnit::RegisterType::Coils, .address = 0, .count = 10});

  ~ModbusSubWindow();

  ModbusSubWindowOptions options();

  void updateOptions(ModbusSubWindowOptions option);

  void updateValues(QVector<QByteArray> values);

 signals:
  void closed(ModbusSubWindow *windowPtr);

  void registerClicked(QModbusDataUnit::RegisterType type, quint16 address);

 private:
  Ui::ModbusSubWindow *_ui;

  ModbusSubWindowOptions _options;

  void _setupUi();

 protected:
  void closeEvent(QCloseEvent *closeEvent) override;
};

#endif  // MODBUSSUBWINDOW_HPP
