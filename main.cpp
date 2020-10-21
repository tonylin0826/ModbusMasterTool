#include <QApplication>
#include <QDebug>
#include <QFile>

#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
  QFile qss(":/style.qss");
  qss.open(QFile::ReadOnly);

  QApplication a(argc, argv);
  qApp->setStyleSheet(qss.readAll());
  qss.close();

  MainWindow w;
  w.show();
  return a.exec();
}
