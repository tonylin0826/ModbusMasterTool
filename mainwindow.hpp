#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QMdiSubWindow>
#include <QResizeEvent>

#include "modbus.hpp"

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

 protected:
  bool eventFilter(QObject *object, QEvent *event) override;
 private slots:
  void on_mdiArea_subWindowActivated(QMdiSubWindow *arg1);

  void on_actiontest_triggered();

  void on_btnConnect_clicked();

 private:
  Ui::MainWindow *_ui;
  Modbus::ModbusTcp *_modbus;

  void _setupUI();
};
#endif  // MAINWINDOW_HPP
