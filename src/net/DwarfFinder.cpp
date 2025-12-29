#include "DwarfFinder.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkInterface>
#include <QTimer>

DwarfFinder::DwarfFinder(QObject *parent)
    : QObject(parent), m_isScanning(false),
      m_maxConcurrentScans(40) { // Initialize m_maxConcurrentScans
  m_nam = new QNetworkAccessManager(this);
}

void DwarfFinder::startScan(const QString &targetSubnet) {
  if (m_isScanning) {
    qDebug() << "Scan already in progress, ignoring request";
    return;
  }

  qDebug() << "Starting scan for subnet:" << targetSubnet;
  m_isScanning = true;
  m_ipsToScan.clear();
  m_scanQueue.clear();

  if (!targetSubnet.isEmpty()) {
    scanSubnet(targetSubnet);
  } else {
    // Find all local subnets
    QStringList subnets = getLocalSubnets();
    qDebug() << "Auto-detected subnets:" << subnets;
    for (const QString &subnet : subnets) {
      scanSubnet(subnet);
    }
  }

  qDebug() << "Scan queue size:" << m_scanQueue.size();
  m_ipsToScan.clear();
  for (const QString &ip : m_scanQueue) {
    m_ipsToScan.append(ip);
  }

  processQueue();

  // If no subnets found or added, finish immediately
  if (m_activeSockets.isEmpty() && m_scanQueue.isEmpty()) {
    qDebug() << "No IPs to scan, finishing immediately";
    m_isScanning = false;
    emit scanFinished();
  }
}

QStringList DwarfFinder::getLocalSubnets() {
  QStringList subnets;
  const QList<QNetworkInterface> interfaces =
      QNetworkInterface::allInterfaces();
  for (const QNetworkInterface &interface : interfaces) {
    if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
        !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

      for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
        if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
          QString ip = entry.ip().toString();
          QString subnet = ip.section('.', 0, 2);
          if (!subnets.contains(subnet)) {
            subnets.append(subnet);
          }
        }
      }
    }
  }
  return subnets;
}

void DwarfFinder::stopScan() {
  m_isScanning = false;
  m_scanQueue.clear(); // Clear the scan queue
  // Cleanup active sockets
  for (QTcpSocket *socket : m_activeSockets) {
    socket->abort();
    socket->deleteLater();
  }
  m_activeSockets.clear();
  emit scanFinished();
}

void DwarfFinder::scanSubnet(const QString &subnet) {
  // Scan .1 to .254
  for (int i = 1; i < 255; ++i) {
    QString ip = QString("%1.%2").arg(subnet).arg(i);
    m_scanQueue.append(ip); // Add IP to the queue instead of checking directly
  }
}

void DwarfFinder::processQueue() {
  if (!m_isScanning)
    return;

  while (m_activeSockets.size() < m_maxConcurrentScans &&
         !m_scanQueue.isEmpty()) {
    QString ip = m_scanQueue.takeFirst();
    checkIp(ip);
  }

  // Emit progress
  int total = m_ipsToScan.size();
  if (total > 0) {
    int remaining = m_scanQueue.size() + m_activeSockets.size();
    int scanned = total - remaining;
    int percent = (scanned * 100) / total;
    emit scanProgress(percent);
  }

  // If no active sockets and no more IPs in queue, scanning is complete
  if (m_activeSockets.isEmpty() && m_scanQueue.isEmpty()) {
    qDebug() << "Scan complete";
    m_isScanning = false;
    emit scanFinished();
  }
}

void DwarfFinder::checkIp(const QString &ip) {
  QTcpSocket *socket = new QTcpSocket(this);
  socket->setProperty("ip", ip);

  m_activeSockets.append(socket);

  connect(socket, &QTcpSocket::connected, this,
          &DwarfFinder::onSocketConnected);
  connect(socket, &QTcpSocket::errorOccurred, this,
          &DwarfFinder::onSocketError);

  // Timeout timer
  QTimer *timer = new QTimer(socket);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, this,
          [this, socket]() { handleSocketTimeout(socket); });

  socket->connectToHost(ip, 8082);
  timer->start(2000); // 2s timeout
}

void DwarfFinder::onSocketConnected() {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (!socket)
    return;

  QString ip = socket->property("ip").toString();
  qDebug() << "Port 8082 open on:" << ip;
  socket->disconnectFromHost();

  // Port 8082 is open, now check device info
  getDeviceInfo(ip);

  m_activeSockets.removeAll(socket);
  socket->deleteLater();

  processQueue();
}

void DwarfFinder::handleSocketTimeout(QTcpSocket *socket) {
  if (!socket)
    return;

  if (!m_isScanning)
    return;

  QString ip = socket->property("ip").toString();
  qDebug() << "Socket timeout for" << ip;

  m_activeSockets.removeAll(socket);
  socket->abort();
  socket->deleteLater();

  processQueue();
}

void DwarfFinder::onSocketError(QAbstractSocket::SocketError error) {
  QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
  if (!socket)
    return;

  QString ip = socket->property("ip").toString();
  // Only log non-timeout errors to reduce noise
  if (error != QAbstractSocket::SocketTimeoutError &&
      error != QAbstractSocket::ConnectionRefusedError) {
    qDebug() << "Socket error for" << ip << ":" << error;
  }

  m_activeSockets.removeAll(socket);
  socket->deleteLater();

  processQueue();
}

void DwarfFinder::checkNextIp() {
  // Deprecated, replaced by processQueue
}

void DwarfFinder::getDeviceInfo(const QString &ip) {
  QUrl url(QString("http://%1:8082/getdeviceInfo").arg(ip));
  QNetworkRequest request(url);
  request.setTransferTimeout(1000); // 1s timeout for HTTP

  QNetworkReply *reply = m_nam->get(request);
  reply->setProperty("ip", ip);

  // Create a timeout timer for this request
  QTimer *timer = new QTimer(reply);
  timer->setSingleShot(true);
  timer->setInterval(1500); // 1.5s total timeout

  connect(timer, &QTimer::timeout, this, [this, reply]() {
    if (reply->isRunning()) {
      reply->abort();
    }
  });

  connect(reply, &QNetworkReply::finished, this, [this, reply, timer]() {
    timer->stop();
    onDeviceInfoReceived(reply);
  });

  timer->start();
}

void DwarfFinder::onDeviceInfoReceived(QNetworkReply *reply) {
  QString ip = reply->property("ip").toString();
  DwarfDeviceInfo info;
  info.ip = ip;
  info.name = "DWARF II";
  info.version.clear();

  if (reply->error() == QNetworkReply::NoError) {
    QByteArray data = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isNull() && doc.isObject()) {
      const QJsonObject root = doc.object();
      const QJsonObject dataObj = root.value(QStringLiteral("data")).toObject();

      auto pickString = [&](const QJsonObject &obj,
                            const QStringList &keys) -> QString {
        for (const QString &k : keys) {
          const QJsonValue v = obj.value(k);
          if (v.isString()) {
            const QString s = v.toString().trimmed();
            if (!s.isEmpty())
              return s;
          }
        }
        return QString();
      };

      // Device name
      QString name = pickString(root,
                                {QStringLiteral("name"), QStringLiteral("deviceName"),
                                 QStringLiteral("productName"), QStringLiteral("model")});
      if (name.isEmpty())
        name = pickString(dataObj,
                          {QStringLiteral("name"), QStringLiteral("deviceName"),
                           QStringLiteral("productName"), QStringLiteral("model")});
      if (!name.isEmpty())
        info.name = name;

      // Firmware / version
      QString ver = pickString(root,
                               {QStringLiteral("version"), QStringLiteral("fwVersion"),
                                QStringLiteral("firmware"), QStringLiteral("firmwareVersion"),
                                QStringLiteral("deviceVersion"), QStringLiteral("romVersion")});
      if (ver.isEmpty())
        ver = pickString(dataObj,
                         {QStringLiteral("version"), QStringLiteral("fwVersion"),
                          QStringLiteral("firmware"), QStringLiteral("firmwareVersion"),
                          QStringLiteral("deviceVersion"), QStringLiteral("romVersion")});
      info.version = ver;
    }

    emit deviceFound(info);
  } else {
    // Even if getdeviceinfo fails, the port 8082 was open, so it's likely a
    // DWARF.
    info.name = "DWARF II (Unverified)";
    emit deviceFound(info);
  }

  reply->deleteLater();
}
