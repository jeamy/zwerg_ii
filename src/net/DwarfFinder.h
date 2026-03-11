#pragma once

#include <QHostAddress>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QTcpSocket>

struct DwarfDeviceInfo {
  QString ip;
  QString name;
  QString version;
};

class DwarfFinder : public QObject {
  Q_OBJECT

public:
  explicit DwarfFinder(QObject *parent = nullptr);
  void startScan(const QString &targetSubnet = QString());
  void stopScan();
  QStringList getLocalSubnets();

signals:
  void deviceFound(const DwarfDeviceInfo &info);
  void scanFinished();
  void scanProgress(int percent);

private slots:
  void onSocketConnected();
  void onSocketError(QAbstractSocket::SocketError socketError);
  void onDeviceInfoReceived(QNetworkReply *reply);
  void onFirmwareVersionReceived(QNetworkReply *reply);

private:
  void checkNextIp();
  void checkIp(const QString &ip);
  void getDeviceInfo(const QString &ip);
  void requestFirmwareVersion(const QString &ip, const QString &name);
  void handleSocketTimeout(QTcpSocket *socket);

  QList<QString> m_ipsToScan;
  int m_currentIpIndex;
  int m_totalIps;
  bool m_isScanning;

  // We can use multiple sockets for parallel scanning to be faster
  // For simplicity, let's start with sequential or small batches.
  // The bash script did parallel. Sequential 254 IPs with 1s timeout is 4
  // minutes. Too slow. We need parallel scanning.

  void scanSubnet(const QString &subnet);
  void processQueue();

  QNetworkAccessManager *m_nam;
  QList<QTcpSocket *> m_activeSockets;
  QList<QString> m_scanQueue;
  const int m_maxConcurrentScans = 20;
};
