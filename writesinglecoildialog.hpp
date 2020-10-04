#ifndef WRITESINGLECOILDIALOG_HPP
#define WRITESINGLECOILDIALOG_HPP

#include <QButtonGroup>
#include <QDialog>

namespace Ui {
class WriteSingleCoilDialog;
}

class WriteSingleCoilDialog : public QDialog {
  Q_OBJECT

 public:
  explicit WriteSingleCoilDialog(QWidget *parent = nullptr, quint16 startingAddress = 0);

  ~WriteSingleCoilDialog();

 private slots:
  void on_btnCancel_clicked();

  void on_btnSend_clicked();

 private:
  Ui::WriteSingleCoilDialog *_ui;

  QButtonGroup *_onOffGroup;

  void _setupUI(quint16 startingAddress);
};

#endif  // WRITESINGLECOILDIALOG_HPP
