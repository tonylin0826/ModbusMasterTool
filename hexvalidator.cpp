#include "hexvalidator.hpp"

#include <QDebug>

HexValidator::HexValidator(int hexSize, QObject *parent) : _hexSize(hexSize) { qDebug() << "HexValidator"; }

void HexValidator::fixup(QString &input) const {
  input = input.rightJustified(_hexSize, '0');
  qDebug() << "changed to " << input;
}

QValidator::State HexValidator::validate(QString &input, int &pos) const { return QValidator::State::Acceptable; }
