#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QList>
#include <QMainWindow>
#include <QMdiSubWindow>
#include <QModbusClient>
#include <QResizeEvent>
#include <QTimer>

#include "modbussubwindow.hpp"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

 public:
  MainWindow(QWidget *parent = nullptr);

  ~MainWindow();

  inline QModbusClient *modbus() { return _modbus; }

 protected:
  bool eventFilter(QObject *object, QEvent *event) override;

 private slots:
  void on_actionConnect_triggered();

  void on_actionDisconnect_triggered();

  void on_actionNew_triggered();

 private:
  Ui::MainWindow *_ui;
  QModbusClient *_modbus;
  QTimer *_pollTimer;

  QList<ModbusSubWindow *> _subwindows;

  void _setupUI();

  void _startPolling(int windowIndex = 0);

  void _showCreateNewRegisterWindow(const QPoint &clickPos);

  void _updateConnectionState(int state);
};
#endif  // MAINWINDOW_HPP
