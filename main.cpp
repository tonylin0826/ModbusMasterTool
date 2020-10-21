#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QSerialPortInfo>

#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
  QFile qss(":/style.qss");
  qss.open(QFile::ReadOnly);

  QApplication a(argc, argv);
  qApp->setStyleSheet(qss.readAll());
  qss.close();

  const auto ports = QSerialPortInfo::availablePorts();
  for (const auto &port : ports) {
    qDebug() << port.portName();
  }

  MainWindow w;
  w.show();
  return a.exec();
}
