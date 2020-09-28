#include "writesingleregisterdialog.hpp"

#include "ui_writesingleregisterdialog.h"

WriteSingleRegisterDialog::WriteSingleRegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::WriteSingleRegisterDialog) {}

WriteSingleRegisterDialog::~WriteSingleRegisterDialog() { delete ui; }

void WriteSingleRegisterDialog::_setupUI() {
  setAttribute(Qt::WA_DeleteOnClose);
  ui->setupUi(this);
  setWindowTitle("Write Single Register");
}
