#include <QApplication>
#include <QFile>
#include <QDebug>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
  QFile qss(":/style.qss");
  qss.open(QFile::ReadOnly);

  QApplication a(argc, argv);
  qApp->setStyleSheet(qss.readAll());
  qss.close();

  QString s = "ffff";
  qDebug() << s.toUShort(nullptr, 16);

  MainWindow w;
  w.show();
  return a.exec();
}

