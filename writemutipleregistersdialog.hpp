#ifndef WRITEMUTIPLEREGISTERSDIALOG_HPP
#define WRITEMUTIPLEREGISTERSDIALOG_HPP

#include <QDialog>

#include "mainwindow.hpp"

namespace Ui {
class WriteMutipleRegistersDialog;
}

enum WriteFormat { Hex = 0 };

class WriteMutipleRegistersDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteMutipleRegistersDialog(MainWindow *parent = nullptr, quint16 startingAddress = 0);
  ~WriteMutipleRegistersDialog();

 private slots:

  void on_btnSend_clicked();

  void on_btnCancel_clicked();

  void on_inputCount_editingFinished();

 private:
  Ui::WriteMutipleRegistersDialog *ui;

  void _setupUI(quint16 startingAddress);

  QVector<quint16> _getValues();
};

#endif  // WRITEMUTIPLEREGISTERSDIALOG_HPP
