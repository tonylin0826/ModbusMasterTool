#include "modbus.hpp"

#include <QTimer>

QByteArray &operator<<(QByteArray &l, quint8 r) {
  l.append(r);
  return l;
}

QByteArray &operator<<(QByteArray &l, quint16 r) { return l << quint8(r >> 8) << quint8(r); }

QByteArray &operator<<(QByteArray &l, quint32 r) { return l << quint16(r >> 16) << quint16(r); }

Modbus::ModbusTcp::ModbusTcp(QObject *parent, quint16 slaveId)
    : QObject(parent), _socket(new QTcpSocket(this)), _transactionId(1), _slaveId(slaveId), _responseTimeout(3000) {
  QObject::connect(_socket, &QTcpSocket::readyRead, this, &ModbusTcp::_readFromSocket);
  QObject::connect(_socket, &QTcpSocket::connected, this, &ModbusTcp::_onConnected);
  QObject::connect(_socket, &QTcpSocket::disconnected, this, &ModbusTcp::_onDisconnected);
  QObject::connect(_socket,
                   static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::errorOccurred),
                   this, &ModbusTcp::_onErrorOccurred);

  _responseHandlers[FunctionCode::ReadHoldingRegisters] = {
      .responseHandler = std::bind(&ModbusTcp::_handleReadHoldingRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleReadHoldingRegisterResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::ReadInputRegisters] = {
      .responseHandler = std::bind(&ModbusTcp::_handleReadInputRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleReadInputRegisterResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::ReadCoils] = {
      .responseHandler = std::bind(&ModbusTcp::_handleReadCoilsResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler =
          std::bind(&ModbusTcp::_handleReadCoilsResponseError, this, std::placeholders::_1, std::placeholders::_2)};

  _responseHandlers[FunctionCode::ReadDiscreteInputs] = {
      .responseHandler = std::bind(&ModbusTcp::_handleReadDiscreteInputsResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleReadDiscreteInputsResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::WriteSingleRegister] = {
      .responseHandler = std::bind(&ModbusTcp::_handleWriteSingleRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleWriteSingleRegisterResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::WriteSingleCoil] = {
      .responseHandler = std::bind(&ModbusTcp::_handleWriteSingleRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleWriteSingleCoilResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::WriteMultipleRegisters] = {
      .responseHandler = std::bind(&ModbusTcp::_handleWriteSingleRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleWriteMultipleRegistersResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};

  _responseHandlers[FunctionCode::WriteMultipleCoils] = {
      .responseHandler = std::bind(&ModbusTcp::_handleWriteSingleRegisterResponse, this, std::placeholders::_1,
                                   std::placeholders::_2, std::placeholders::_3, std::placeholders::_4),
      .errorHandler = std::bind(&ModbusTcp::_handleWriteMultipleCoilsResponseError, this, std::placeholders::_1,
                                std::placeholders::_2)};
}

Modbus::ModbusTcp::~ModbusTcp() {
  if (_socket->isOpen()) {
    _socket->close();
  }

  _socket->deleteLater();
}

bool Modbus::ModbusTcp::_read(FunctionCode code, const quint16 address, const quint16 count,
                              Modbus::ModbusReadCallback cb) {
  quint16 maxCount = 125;
  if (code == FunctionCode::ReadCoils || code == FunctionCode::ReadDiscreteInputs) {
    maxCount = 2000;
  }

  if (count > maxCount) {
    throw std::runtime_error("read exceeds max count");
  }

  QByteArray buf;

  buf << _transactionId << static_cast<quint16>(0) << static_cast<quint16>(6) << _slaveId << static_cast<quint8>(code)
      << address << count;

  const auto success = _socket->write(buf) > 0;

  if (success) {
    _readCallbacks.insert(_transactionId, std::move(cb));
    _readCount.insert(_transactionId, count);

    QTimer::singleShot(_responseTimeout, [=]() {
      _readCallbacks.remove(_transactionId);
      _readCount.remove(_transactionId);
    });

    _transactionId++;
  }

  return success;
}

void Modbus::ModbusTcp::_handleReadResponseError(quint16 transactionId, QDataStream &stream, const std::string &type) {
  if (_readCallbacks.find(transactionId) == _readCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    emit modbusErrorOccurred(ModbusErrorCode::TransactionIdNotFound);
    return;
  }

  quint8 errorCode = 0;
  stream >> errorCode;

  std::stringstream ss;
  ss << "failed to read " << type << ", error code - " << errorCode;

  _readCallbacks.take(transactionId)({.success = false, .errorMessage = QString::fromStdString(ss.str())});
}

void Modbus::ModbusTcp::_handleWriteResponseError(quint16 transactionId, QDataStream &stream, const std::string &type) {
  if (_writeCallbacks.find(transactionId) == _writeCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    emit modbusErrorOccurred(ModbusErrorCode::TransactionIdNotFound);
    return;
  }

  quint8 errorCode = 0;
  stream >> errorCode;

  std::stringstream ss;
  ss << "failed to write " << type << ", error code - " << errorCode;

  _writeCallbacks.take(transactionId)({.success = false, .errorMessage = QString::fromStdString(ss.str())});
}

bool Modbus::ModbusTcp::readHoldingRegisters(const quint16 address, const quint16 count,
                                             Modbus::ModbusReadCallback cb) {
  return _read(FunctionCode::ReadHoldingRegisters, address, count, std::move(cb));
}

bool Modbus::ModbusTcp::readInputRegisters(const quint16 address, const quint16 count, Modbus::ModbusReadCallback cb) {
  return _read(FunctionCode::ReadInputRegisters, address, count, std::move(cb));
}

bool Modbus::ModbusTcp::readCoils(const quint16 address, const quint16 count, Modbus::ModbusReadCallback cb) {
  return _read(FunctionCode::ReadCoils, address, count, std::move(cb));
}

bool Modbus::ModbusTcp::readDiscreteInputs(const quint16 address, const quint16 count, Modbus::ModbusReadCallback cb) {
  return _read(FunctionCode::ReadDiscreteInputs, address, count, std::move(cb));
}

bool Modbus::ModbusTcp::writeSingleRegister(const quint16 address, const quint16 value, ModbusWriteCallback cb) {
  QByteArray arr;
  arr << value;

  return writeSingleRegister(address, arr, std::move(cb));
}

bool Modbus::ModbusTcp::writeSingleRegister(const quint16 address, const QByteArray &value, ModbusWriteCallback cb) {
  QByteArray buf;

  buf << _transactionId << static_cast<quint16>(0) << static_cast<quint16>(6) << _slaveId
      << static_cast<quint8>(FunctionCode::WriteSingleRegister) << address;

  buf.append(value);

  const auto success = _socket->write(buf) > 0;

  if (success) {
    _writeCallbacks.insert(_transactionId, std::move(cb));

    QTimer::singleShot(_responseTimeout, [=]() { _writeCallbacks.remove(_transactionId); });

    _transactionId++;
  }

  return success;
}

bool Modbus::ModbusTcp::writeSingleCoil(const quint16 address, const bool coilStatus, ModbusWriteCallback cb) {
  QByteArray buf;

  buf << _transactionId << static_cast<quint16>(0) << static_cast<quint16>(6) << _slaveId
      << static_cast<quint8>(FunctionCode::WriteSingleCoil) << address
      << static_cast<quint16>(coilStatus ? 0xff00 : 0x0000);

  const auto success = _socket->write(buf) > 0;

  if (success) {
    _writeCallbacks.insert(_transactionId, std::move(cb));

    QTimer::singleShot(_responseTimeout, [=]() { _writeCallbacks.remove(_transactionId); });

    _transactionId++;
  }

  return success;
}

bool Modbus::ModbusTcp::writeMultipleRegisters(const quint16 address, const QVector<quint16> &values,
                                               ModbusWriteCallback cb) {
  if (values.size() > 123) {
    throw std::runtime_error("write exceeds max count");
  }

  QVector<QByteArray> arr;
  for (const auto &v : values) {
    QByteArray bytes;
    bytes << v;

    arr.push_back(bytes);
  }

  return writeMultipleRegisters(address, arr, std::move(cb));
}

bool Modbus::ModbusTcp::writeMultipleRegisters(const quint16 address, const QVector<QByteArray> &values,
                                               ModbusWriteCallback cb) {
  if (values.size() > 123) {
    throw std::runtime_error("write exceeds max count");
  }

  QByteArray buf;

  buf << _transactionId << static_cast<quint16>(0) << static_cast<quint16>(values.size() * 2 + 7) << _slaveId
      << static_cast<quint8>(FunctionCode::WriteMultipleRegisters) << address << static_cast<quint16>(values.size())
      << static_cast<quint8>(values.size() * 2);

  for (const auto &v : values) {
    buf << static_cast<quint8>(v[0]) << static_cast<quint8>(v[1]);
  }

  const auto success = _socket->write(buf) > 0;

  if (success) {
    _writeCallbacks.insert(_transactionId, std::move(cb));

    QTimer::singleShot(_responseTimeout, [=]() { _writeCallbacks.remove(_transactionId); });

    _transactionId++;
  }

  return success;
}

bool Modbus::ModbusTcp::writeMultipleCoils(const quint16 address, const QVector<bool> &coilStatuses,
                                           ModbusWriteCallback cb) {
  if (coilStatuses.size() > 1968) {
    throw std::runtime_error("write exceeds max count");
  }

  const quint8 byteCount = static_cast<quint8>(coilStatuses.size() / 8 + ((coilStatuses.size() % 8) ? 1 : 0));

  QByteArray buf;
  buf << _transactionId << static_cast<quint16>(0) << static_cast<quint16>(byteCount + 7) << _slaveId
      << static_cast<quint8>(FunctionCode::WriteMultipleCoils) << address << static_cast<quint16>(coilStatuses.size())
      << byteCount;

  for (quint16 i = 0, j = 0; i < byteCount; i++) {
    quint8 byteToWrite = 0x00;
    quint8 bit = 0x01 & 0xFF;

    for (quint16 k = 0; k < 8 && j < coilStatuses.size(); k++) {
      byteToWrite |= (coilStatuses[j++] ? ((bit << k) & 0xFF) : 0x00);
    }

    buf << byteToWrite;
  }

  const auto success = _socket->write(buf) > 0;

  if (success) {
    _writeCallbacks.insert(_transactionId, std::move(cb));

    QTimer::singleShot(_responseTimeout, [=]() { _writeCallbacks.remove(_transactionId); });

    _transactionId++;
  }

  return success;
}

void Modbus::ModbusTcp::_handleReadHoldingRegisterResponseError(quint16 transactionId, QDataStream &stream) {
  _handleReadResponseError(transactionId, stream, "Holding Registers");
}

void Modbus::ModbusTcp::_handleReadHoldingRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                                           QDataStream &stream) {
  Q_UNUSED(protocolId);

  if (_readCallbacks.find(transactionId) == _readCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    return;
  }

  if (unitId != _slaveId) {
    qDebug() << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    std::stringstream ss;
    ss << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    _readCallbacks.take(transactionId)({.success = true, .errorMessage = QString::fromStdString(ss.str())});
    return;
  }

  quint8 byteCount = 0;
  stream >> byteCount;

  QVector<QByteArray> r;

  while (!stream.atEnd()) {
    char bs[2] = {0};
    stream.readRawData(bs, 2);
    r.append(QByteArray(reinterpret_cast<char *>(bs), 2));
  }

  _readCallbacks.take(transactionId)({.success = true, .results = r});
}

void Modbus::ModbusTcp::_handleReadInputRegisterResponseError(quint16 transactionId, QDataStream &stream) {
  _handleReadResponseError(transactionId, stream, "Input Registers");
}

void Modbus::ModbusTcp::_handleReadInputRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                                         QDataStream &stream) {
  Q_UNUSED(protocolId);

  if (_readCallbacks.find(transactionId) == _readCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    return;
  }

  if (unitId != _slaveId) {
    qDebug() << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    std::stringstream ss;
    ss << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    _readCallbacks.take(transactionId)({.success = true, .errorMessage = QString::fromStdString(ss.str())});
    return;
  }

  quint8 byteCount = 0;
  stream >> byteCount;

  QVector<QByteArray> r;

  while (!stream.atEnd()) {
    char bs[2] = {0};
    stream.readRawData(bs, 2);
    r.append(QByteArray(reinterpret_cast<char *>(bs), 2));
  }

  _readCallbacks.take(transactionId)({.success = true, .results = r});
}

void Modbus::ModbusTcp::_handleReadCoilsResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                                 QDataStream &stream) {
  Q_UNUSED(protocolId);

  if (_readCallbacks.find(transactionId) == _readCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    return;
  }

  if (unitId != _slaveId) {
    qDebug() << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    std::stringstream ss;
    ss << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    _readCallbacks.take(transactionId)({.success = true, .errorMessage = QString::fromStdString(ss.str())});
    return;
  }

  quint8 byteCount = 0;
  stream >> byteCount;

  const quint16 bitCount =
      _readCount.find(transactionId) == _readCount.end() ? 8 * byteCount : _readCount[transactionId];

  QVector<QByteArray> r;

  while (!stream.atEnd()) {
    quint8 b;
    stream >> b;

    std::bitset<8> bits(b);

    for (int i = 0; i < 8 && r.size() < bitCount; i++) {
      QByteArray ba;
      ba.append(static_cast<quint8>(bits[i] ? 0x01 : 0x00));
      r.append(ba);
    }
  }

  _readCallbacks.take(transactionId)({.success = true, .results = r});
}

void Modbus::ModbusTcp::_handleReadCoilsResponseError(quint16 transactionId, QDataStream &stream) {
  _handleReadResponseError(transactionId, stream, "Coils");
}

void Modbus::ModbusTcp::_handleReadDiscreteInputsResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                                          QDataStream &stream) {
  Q_UNUSED(protocolId);

  if (_readCallbacks.find(transactionId) == _readCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    emit modbusErrorOccurred(ModbusErrorCode::TransactionIdNotFound);
    return;
  }

  if (unitId != _slaveId) {
    qDebug() << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    std::stringstream ss;
    ss << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    _readCallbacks.take(transactionId)({.success = true, .errorMessage = QString::fromStdString(ss.str())});
    return;
  }

  quint8 byteCount = 0;
  stream >> byteCount;

  const quint16 bitCount =
      _readCount.find(transactionId) == _readCount.end() ? 8 * byteCount : _readCount[transactionId];

  QVector<QByteArray> r;

  while (!stream.atEnd()) {
    quint8 b;
    stream >> b;

    std::bitset<8> bits(b);

    for (int i = 0; i < 8 && r.size() < bitCount; i++) {
      QByteArray ba;
      ba.append(static_cast<quint8>(bits[i] ? 0x01 : 0x00));
      r.append(ba);
    }
  }

  _readCallbacks.take(transactionId)({.success = true, .results = r});
}

void Modbus::ModbusTcp::_handleReadDiscreteInputsResponseError(quint16 transactionId, QDataStream &stream) {
  _handleReadResponseError(transactionId, stream, "Discrete Inputs");
}

void Modbus::ModbusTcp::_handleWriteSingleRegisterResponseError(quint16 transactionId, QDataStream &stream) {
  _handleWriteResponseError(transactionId, stream, "Holding Register");
}

void Modbus::ModbusTcp::_handleWriteSingleCoilResponseError(quint16 transactionId, QDataStream &stream) {
  _handleWriteResponseError(transactionId, stream, "Coil");
}

void Modbus::ModbusTcp::_handleWriteMultipleRegistersResponseError(quint16 transactionId, QDataStream &stream) {
  _handleWriteResponseError(transactionId, stream, "Holding Register");
}

void Modbus::ModbusTcp::_handleWriteMultipleCoilsResponseError(quint16 transactionId, QDataStream &stream) {
  _handleWriteResponseError(transactionId, stream, "Coils");
}

void Modbus::ModbusTcp::_handleWriteSingleRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                                           QDataStream &stream) {
  Q_UNUSED(protocolId);
  Q_UNUSED(stream);

  if (_writeCallbacks.find(transactionId) == _writeCallbacks.end()) {
    qDebug() << "invalid response - transactionId " << transactionId << " not found";
    emit modbusErrorOccurred(ModbusErrorCode::TransactionIdNotFound);
    return;
  }

  if (unitId != _slaveId) {
    qDebug() << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    std::stringstream ss;
    ss << "invalid response - unitId " << unitId << " not the same with - " << _slaveId;
    _writeCallbacks.take(transactionId)({.success = true, .errorMessage = QString::fromStdString(ss.str())});
    return;
  }

  _writeCallbacks.take(transactionId)({.success = true});
}

void Modbus::ModbusTcp::_readFromSocket() {
  const auto readBytes = _socket->readAll();
  quint16 transactionId = 0, protocolId = 0, length = 0;
  quint8 unitId = 0, functionCode = 0;
  QDataStream in(readBytes);

  in.setVersion(QDataStream::Qt_5_12);

  in >> transactionId >> protocolId >> length >> unitId >> functionCode;

  //  qDebug() << readBytes.size();
  //  qDebug() << readBytes.toHex();
  //  qDebug() << "transactionId => " << transactionId;
  //  qDebug() << "protocolId => " << protocolId;
  //  qDebug() << "length => " << length;
  //  qDebug() << "unitId => " << unitId;

  const bool exceptionOccured = std::bitset<8>(functionCode)[7];

  functionCode = exceptionOccured ? functionCode - 0x80 : functionCode;

  if (_responseHandlers.find(static_cast<FunctionCode>(functionCode)) == _responseHandlers.end()) {
    _readCallbacks.remove(transactionId);
    _readCount.remove(transactionId);
    emit modbusErrorOccurred(ModbusErrorCode::InvalidResponseFunctionCode);
    qDebug() << "invalid response function code - " << functionCode;
    return;
  }

  if (exceptionOccured) {
    _responseHandlers[static_cast<FunctionCode>(functionCode)].errorHandler(transactionId, in);
    _readCount.remove(transactionId);
    return;
  }

  _responseHandlers[static_cast<FunctionCode>(functionCode)].responseHandler(transactionId, protocolId, unitId, in);
  _readCount.remove(transactionId);
}
