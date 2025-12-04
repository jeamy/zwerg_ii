#pragma once

#include "base.pb.h"
#include <QObject>
#include <QTimer>
#include <QWebSocket>
#include <memory>

class DwarfWebSocketClient : public QObject {
  Q_OBJECT

public:
  explicit DwarfWebSocketClient(const QString &ip, QObject *parent = nullptr);
  ~DwarfWebSocketClient();

  void connectToDevice();
  void disconnect();
  bool isConnected() const;

  // Send command to DWARF II
  void sendCommand(uint32_t moduleId, uint32_t cmd,
                   const QByteArray &data = QByteArray());

signals:
  void connected();
  void disconnected();
  void messageReceived(uint32_t moduleId, uint32_t cmd, const QByteArray &data);
  void errorOccurred(const QString &error);

private slots:
  void onConnected();
  void onDisconnected();
  void onBinaryMessageReceived(const QByteArray &message);
  void onTextMessageReceived(const QString &message);
  void onError(QAbstractSocket::SocketError error);
  void sendPing();

private:
  QByteArray createPacket(uint32_t moduleId, uint32_t cmd,
                          const QByteArray &data);

  QWebSocket m_webSocket;
  QString m_ip;
  QTimer m_pingTimer;
  QString m_clientId;

  static constexpr int WEBSOCKET_PORT = 9900;
  static constexpr uint32_t MAJOR_VERSION = 2;
  static constexpr uint32_t MINOR_VERSION = 0;
  static constexpr uint32_t DEVICE_ID = 1; // DWARF II
};
