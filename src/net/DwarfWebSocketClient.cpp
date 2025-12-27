#include "DwarfWebSocketClient.h"
#include "ProtobufHelper.h"
#include <QDebug>
#include <QUuid>

DwarfWebSocketClient::DwarfWebSocketClient(const QString &ip, QObject *parent)
    : QObject(parent), m_ip(ip),
      m_clientId(QUuid::createUuid().toString(QUuid::WithoutBraces)) {
  connect(&m_webSocket, &QWebSocket::connected, this,
          &DwarfWebSocketClient::onConnected);
  connect(&m_webSocket, &QWebSocket::disconnected, this,
          &DwarfWebSocketClient::onDisconnected);
  connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this,
          &DwarfWebSocketClient::onBinaryMessageReceived);
  connect(&m_webSocket, &QWebSocket::textMessageReceived, this,
          &DwarfWebSocketClient::onTextMessageReceived);
  connect(
      &m_webSocket,
      QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
      this, &DwarfWebSocketClient::onError);

  connect(&m_pingTimer, &QTimer::timeout, this,
          &DwarfWebSocketClient::sendPing);
  m_pingTimer.setInterval(5000);
}

DwarfWebSocketClient::~DwarfWebSocketClient() { disconnect(); }

void DwarfWebSocketClient::connectToDevice() {
  if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
    qDebug() << "Already connected";
    return;
  }

  QString url = QString("ws://%1:%2").arg(m_ip).arg(WEBSOCKET_PORT);
  qDebug() << "Connecting to:" << url;
  m_webSocket.open(QUrl(url));
}

void DwarfWebSocketClient::disconnect() {
  m_pingTimer.stop();
  if (m_webSocket.state() == QAbstractSocket::ConnectedState) {
    m_webSocket.close();
  }
}

bool DwarfWebSocketClient::isConnected() const {
  return m_webSocket.state() == QAbstractSocket::ConnectedState;
}

void DwarfWebSocketClient::sendCommand(uint32_t moduleId, uint32_t cmd,
                                       const QByteArray &data) {
  if (!isConnected()) {
    qWarning() << "[DwarfWebSocketClient] Cannot send command: not connected"
               << "module" << moduleId << "cmd" << cmd << "data size" << data.size();
    return;
  }

  QByteArray packet = createPacket(moduleId, cmd, data);
  qint64 bytesSent = m_webSocket.sendBinaryMessage(packet);
  qWarning() << "[DwarfWebSocketClient] Sent command - Module:" << moduleId 
             << "Cmd:" << cmd << "PacketSize:" << packet.size() 
             << "BytesSent:" << bytesSent
             << "PayloadHex:" << data.toHex();
}

void DwarfWebSocketClient::sendTextCommand(const QString &text) {
  if (!isConnected()) {
    qWarning() << "[DwarfWebSocketClient] Cannot send text command: not connected. Text:" << text;
    return;
  }
  qint64 bytesSent = m_webSocket.sendTextMessage(text);
  qWarning() << "[DwarfWebSocketClient] Sent text command BytesSent:" << bytesSent
             << "Text:" << text;
}

void DwarfWebSocketClient::onConnected() {
  qDebug() << "WebSocket connected to" << m_ip;
  m_pingTimer.start();
  emit connected();
}

void DwarfWebSocketClient::onDisconnected() {
  qDebug() << "WebSocket disconnected";
  m_pingTimer.stop();
  emit disconnected();
}

void DwarfWebSocketClient::onBinaryMessageReceived(const QByteArray &message) {
  dwarf::WsPacket packet;
  if (!packet.ParseFromArray(message.data(), message.size())) {
    qWarning() << "Failed to parse WsPacket";
    return;
  }

  qWarning() << "[DwarfWebSocketClient] Received - Module:" << packet.module_id()
             << "Cmd:" << packet.cmd() << "Type:" << packet.type()
             << "Data size:" << packet.data().size();

  QByteArray data(packet.data().data(), packet.data().size());
  emit messageReceived(packet.module_id(), packet.cmd(), data);
}

void DwarfWebSocketClient::onTextMessageReceived(const QString &message) {
  // DWARF API 4.6: Server returns "pong" in response to "ping"
  if (message == QStringLiteral("pong")) {
    qDebug() << "[DwarfWebSocketClient] Received heartbeat pong";
  } else {
    qDebug() << "[DwarfWebSocketClient] Received text message:" << message;
  }
}

void DwarfWebSocketClient::onError(QAbstractSocket::SocketError error) {
  QString errorString = m_webSocket.errorString();
  qWarning() << "WebSocket error:" << error << errorString;
  emit errorOccurred(errorString);
}

void DwarfWebSocketClient::sendPing() {
  if (!isConnected())
    return;

  // DWARF API 4.6: Client sends "ping", server returns "pong"
  m_webSocket.sendTextMessage(QStringLiteral("ping"));
  qDebug() << "[DwarfWebSocketClient] Sent heartbeat ping";
}

QByteArray DwarfWebSocketClient::createPacket(uint32_t moduleId, uint32_t cmd,
                                              const QByteArray &data) {
  dwarf::WsPacket packet;
  packet.set_major_version(MAJOR_VERSION);
  packet.set_minor_version(MINOR_VERSION);
  packet.set_device_id(DEVICE_ID);
  packet.set_module_id(moduleId);
  packet.set_cmd(cmd);
  packet.set_data(data.data(), data.size());
  packet.set_client_id(m_clientId.toStdString());

  return ProtobufHelper::serialize(packet);
}
