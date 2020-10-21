#ifndef HEXVALIDATOR_HPP
#define HEXVALIDATOR_HPP

#include <QObject>
#include <QValidator>

class HexValidator : public QValidator {
  Q_OBJECT
 public:
  HexValidator(int hexSize = 2, QObject *parent = nullptr);

  virtual void fixup(QString &input) const;

  virtual QValidator::State validate(QString &input, int &pos) const;

 private:
  int _hexSize;
};

#endif  // HEXVALIDATOR_HPP
