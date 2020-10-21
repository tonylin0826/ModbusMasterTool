#ifndef WRITEMUTIPLEREGISTERSDIALOG_HPP
#define WRITEMUTIPLEREGISTERSDIALOG_HPP

#include <QDialog>

#include "mainwindow.hpp"

namespace Ui {
class WriteMutipleRegistersDialog;
}

enum WriteFormat { Hex = 0, Uint16, Int16 };

class WriteMutipleRegistersDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteMutipleRegistersDialog(MainWindow *parent = nullptr, quint16 startingAddress = 0);
  ~WriteMutipleRegistersDialog();

 private slots:

  void on_btnSend_clicked();

  void on_btnCancel_clicked();

  void on_inputCount_editingFinished();

  void on_selectFormat_currentIndexChanged(int index);

private:
  Ui::WriteMutipleRegistersDialog *ui;

  void _setupUI(quint16 startingAddress);

  void _updateTableSettings();

  QVector<quint16> _getValues();

  WriteFormat _format;
};

#endif  // WRITEMUTIPLEREGISTERSDIALOG_HPP
