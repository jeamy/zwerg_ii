#pragma once

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class DwarfHttpClient : public QObject {
  Q_OBJECT

public:
  enum DeviceSettingMode {
    ChangePassword = 0,
    ChangeName = 1,
  };

  explicit DwarfHttpClient(const QString &ip, QObject *parent = nullptr);
  void fetchMediaList();
  void fetchDefaultParamsConfig();
  void deleteMedia(const QString &filePath);
  void setDeviceName(const QString &newName,
                     const QString &oldName = QString());
  void setDevicePassword(const QString &oldPassword,
                         const QString &newPassword);
  void uploadFirmware(const QString &filePath);

signals:
  void mediaListReceived(const QJsonDocument &document);
  void defaultParamsConfigReceived(const QJsonDocument &document);
  void mediaDeleted(const QString &filePath);
  void deleteError(const QString &filePath, const QString &error);
  void deviceSettingChanged(int mode, const QString &appliedValue);
  void deviceSettingError(int mode, const QString &error);
  void firmwareUploadFinished(const QString &filePath, bool success, int code,
                              const QString &message);
  void errorOccurred(const QString &error);

private:
  QNetworkAccessManager *m_manager;
  QString m_ip;
  static constexpr int HTTP_PORT = 8082;
};
