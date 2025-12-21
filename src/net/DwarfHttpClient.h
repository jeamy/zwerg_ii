#pragma once

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class DwarfHttpClient : public QObject {
  Q_OBJECT

public:
  explicit DwarfHttpClient(const QString &ip, QObject *parent = nullptr);
  void fetchMediaList();
  void fetchDefaultParamsConfig();
  void deleteMedia(const QString &filePath);

signals:
  void mediaListReceived(const QJsonDocument &document);
  void defaultParamsConfigReceived(const QJsonDocument &document);
  void mediaDeleted(const QString &filePath);
  void deleteError(const QString &filePath, const QString &error);
  void errorOccurred(const QString &error);

private:
  QNetworkAccessManager *m_manager;
  QString m_ip;
  static constexpr int HTTP_PORT = 8082;
};
