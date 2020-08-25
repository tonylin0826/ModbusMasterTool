#ifndef MODBUS_HPP
#define MODBUS_HPP

#include <QDataStream>
#include <QDebug>
#include <QHash>
#include <QHostAddress>
#include <QIODevice>
#include <QObject>
#include <QPair>
#include <QTcpSocket>
#include <QVector>
#include <bitset>
#include <chrono>
#include <functional>
#include <sstream>

namespace Modbus {

enum FunctionCode {
  ReadCoils = 0x01,
  ReadDiscreteInputs = 0x02,
  ReadHoldingRegisters = 0x03,
  ReadInputRegisters = 0x04,
  WriteSingleCoil = 0x05,
  WriteSingleRegister = 0x06,
  WriteMultipleCoils = 0x0F,
  WriteMultipleRegisters = 0x10
};

enum ModbusErrorCode { Unknown = 0, TransactionIdNotFound, SlaveIdNotMatched, InvalidResponseFunctionCode };

typedef struct {
  bool success;
  QString errorMessage;
  QVector<QByteArray> results;
} ModbusReadResult;

typedef struct {
  bool success;
  QString errorMessage;
} ModbusWriteResult;

typedef struct {
  std::function<void(quint16 transactionId, quint16 protocolId, quint8 unitId, QDataStream &stream)> responseHandler;
  std::function<void(quint16 transactionId, QDataStream &stream)> errorHandler;
} ModbusReadResponseHandler;

typedef std::function<void(ModbusReadResult result)> ModbusReadCallback;
typedef std::function<void(ModbusWriteResult result)> ModbusWriteCallback;

class ModbusTcp : public QObject {
  Q_OBJECT
 public:
  ModbusTcp(QObject *parent = nullptr, quint16 slaveId = 255);

  ~ModbusTcp();

  bool readHoldingRegisters(const quint16 address, const quint16 count, ModbusReadCallback cb);

  bool readInputRegisters(const quint16 address, const quint16 count, ModbusReadCallback cb);

  bool readCoils(const quint16 address, const quint16 count, ModbusReadCallback cb);

  bool readDiscreteInputs(const quint16 address, const quint16 count, ModbusReadCallback cb);

  bool writeSingleRegister(const quint16 address, const quint16 value, ModbusWriteCallback cb);

  bool writeSingleRegister(const quint16 address, const QByteArray &value, ModbusWriteCallback cb);

  bool writeSingleCoil(const quint16 address, const bool coilStatus, ModbusWriteCallback cb);

  bool writeMultipleRegisters(const quint16 address, const QVector<quint16> &values, ModbusWriteCallback cb);

  bool writeMultipleRegisters(const quint16 address, const QVector<QByteArray> &values, ModbusWriteCallback cb);

  bool writeMultipleCoils(const quint16 address, const QVector<bool> &coilStatuses, ModbusWriteCallback cb);

 private:
  QTcpSocket *_socket;
  QHash<quint16, ModbusReadCallback> _readCallbacks;
  QHash<quint16, ModbusWriteCallback> _writeCallbacks;
  QHash<FunctionCode, ModbusReadResponseHandler> _responseHandlers;

  quint16 _transactionId;
  quint8 _slaveId;

  std::chrono::milliseconds _responseTimeout;

  bool _read(FunctionCode code, const quint16 address, const quint16 count, ModbusReadCallback cb);

  void _handleReadResponseError(quint16 transactionId, QDataStream &stream, const std::string &type);

  void _handleWriteResponseError(quint16 transactionId, QDataStream &stream, const std::string &type);

  void _handleReadHoldingRegisterResponseError(quint16 transactionId, QDataStream &stream);

  void _handleReadHoldingRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                          QDataStream &stream);

  void _handleReadInputRegisterResponseError(quint16 transactionId, QDataStream &stream);

  void _handleReadInputRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId, QDataStream &stream);

  void _handleReadCoilsResponseError(quint16 transactionId, QDataStream &stream);

  void _handleReadCoilsResponse(quint16 transactionId, quint16 protocolId, quint8 unitId, QDataStream &stream);

  void _handleReadDiscreteInputsResponseError(quint16 transactionId, QDataStream &stream);

  void _handleReadDiscreteInputsResponse(quint16 transactionId, quint16 protocolId, quint8 unitId, QDataStream &stream);

  void _handleWriteSingleRegisterResponseError(quint16 transactionId, QDataStream &stream);

  void _handleWriteSingleRegisterResponse(quint16 transactionId, quint16 protocolId, quint8 unitId,
                                          QDataStream &stream);

  void _handleWriteSingleCoilResponseError(quint16 transactionId, QDataStream &stream);

  void _handleWriteMultipleRegistersResponseError(quint16 transactionId, QDataStream &stream);

  void _handleWriteMultipleCoilsResponseError(quint16 transactionId, QDataStream &stream);

 public slots:
  void connect(const QString &host, quint16 port) {
    if (_socket->state() == QAbstractSocket::SocketState::ConnectedState ||
        _socket->state() == QAbstractSocket::SocketState::ConnectingState) {
      return;
    }
    _socket->connectToHost(host, port);
  }

  void disconnect() { _socket->disconnectFromHost(); }

 private slots:
  void _readFromSocket();

  void _onConnected() { emit connected(); }

  void _onDisconnected() { emit disconnected(); }

  void _onErrorOccurred(QAbstractSocket::SocketError error) {
    qDebug() << "error " << error << " occured";
    emit errorOccurred(error);
  }

 signals:
  void connected();

  void disconnected();

  void errorOccurred(QAbstractSocket::SocketError e);

  void modbusErrorOccurred(ModbusErrorCode e);
};

}  // namespace Modbus

#endif  // MODBUS_HPP
