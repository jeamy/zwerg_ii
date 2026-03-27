#include "DwarfHttpClient.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QNetworkRequest>
#include <QUrl>
#include <functional>
#include <memory>

namespace {
QString responseMessage(const QJsonObject &obj) {
  const QString msg = obj.value(QStringLiteral("msg")).toString();
  if (!msg.isEmpty())
    return msg;
  return obj.value(QStringLiteral("message")).toString();
}
}

DwarfHttpClient::DwarfHttpClient(const QString &ip, QObject *parent)
    : QObject(parent), m_manager(new QNetworkAccessManager(this)), m_ip(ip) {}

void DwarfHttpClient::fetchMediaList() {
  QUrl url(
      QString("http://%1:%2/album/list/mediaInfos").arg(m_ip).arg(HTTP_PORT));
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/json"));

  QJsonObject payload;
  payload.insert(QStringLiteral("mediaType"), 0);
  payload.insert(QStringLiteral("pageIndex"), 0);
  payload.insert(QStringLiteral("pageSize"), 0);
  QJsonDocument doc(payload);

  QNetworkReply *reply = m_manager->post(request, doc.toJson());

  connect(reply, &QNetworkReply::finished, this, [this, reply]() {
    if (reply->error() == QNetworkReply::NoError) {
      QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
      emit mediaListReceived(document);
    } else {
      emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
  });
}

void DwarfHttpClient::fetchDefaultParamsConfig() {
  auto fetchOnPort = std::make_shared<std::function<void(int, bool)>>();
  *fetchOnPort = [this, fetchOnPort](int port, bool allowFallback) {
    QUrl url(QString("http://%1:%2/getDefaultParamsConfig").arg(m_ip).arg(port));
    QNetworkRequest request(url);

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, port, allowFallback, fetchOnPort]() {
              if (reply->error() == QNetworkReply::NoError) {
                const QByteArray raw = reply->readAll();
                const QJsonDocument document = QJsonDocument::fromJson(raw);
                if (!document.isNull()) {
                  bool looksEmpty = false;
                  if (document.isObject()) {
                    const QJsonObject obj = document.object();
                    const QJsonObject dataObj =
                        obj.value(QStringLiteral("data")).toObject();
                    const QJsonArray cameras =
                        dataObj.value(QStringLiteral("cameras")).toArray();
                    const QJsonArray featureParams =
                        dataObj.value(QStringLiteral("featureParams")).toArray();
                    looksEmpty = cameras.isEmpty() && featureParams.isEmpty();
                  }

                  if (looksEmpty && allowFallback && port == HTTP_PORT) {
                    qWarning()
                        << "[HttpClient] getDefaultParamsConfig returned empty arrays on" << port
                        << "- retrying on 8080";
                    reply->deleteLater();
                    (*fetchOnPort)(8080, false);
                    return;
                  }

                  emit defaultParamsConfigReceived(document);
                } else {
                  emit errorOccurred(tr("Invalid params config JSON"));
                }
              } else {
                emit errorOccurred(reply->errorString());
              }
              reply->deleteLater();
            });
  };

  (*fetchOnPort)(HTTP_PORT, true);
}

void DwarfHttpClient::deleteMedia(const QString &filePath) {
  auto sendDeleteRequest =
      std::make_shared<std::function<void(bool legacyFallbackAllowed)>>();

  *sendDeleteRequest = [this, filePath, sendDeleteRequest](bool legacyFallbackAllowed) {
    const bool useLegacyEndpoint = !legacyFallbackAllowed;
    const QString endpoint = useLegacyEndpoint ? QStringLiteral("/sdcard/deleteFile")
                                               : QStringLiteral("/album/delete");
    QUrl url(QString("http://%1:%2%3").arg(m_ip).arg(HTTP_PORT).arg(endpoint));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QStringLiteral("application/json"));

    QJsonObject payload;
    if (useLegacyEndpoint) {
      payload.insert(QStringLiteral("filePath"), filePath);
    } else {
      QJsonObject item;
      item.insert(QStringLiteral("mediaType"), 0);
      item.insert(QStringLiteral("filePath"), filePath);
      item.insert(QStringLiteral("fileName"), QFileInfo(filePath).fileName());

      QJsonArray datas;
      datas.append(item);
      payload.insert(QStringLiteral("datas"), datas);
    }

    const QJsonDocument doc(payload);
    qDebug() << "[HttpClient] Deleting media:" << filePath
             << "endpoint:" << endpoint;
    qDebug() << "[HttpClient] Delete request:" << doc.toJson(QJsonDocument::Compact);

    QNetworkReply *reply = m_manager->post(request, doc.toJson());
    connect(reply, &QNetworkReply::finished, this,
            [this, reply, filePath, sendDeleteRequest, legacyFallbackAllowed]() {
              const QByteArray responseData = reply->readAll();
              const int httpCode =
                  reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
              qDebug() << "[HttpClient] Delete response code:" << httpCode;
              qDebug() << "[HttpClient] Delete response:" << responseData;

              auto retryLegacy = [reply, sendDeleteRequest, legacyFallbackAllowed]() {
                if (!legacyFallbackAllowed)
                  return false;
                const int httpCode =
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                const QString networkError = reply->errorString();
                const bool missingEndpoint =
                    reply->error() == QNetworkReply::ContentNotFoundError ||
                    httpCode == 404 ||
                    networkError.contains(QStringLiteral("404")) ||
                    networkError.contains(QStringLiteral("not found"),
                                          Qt::CaseInsensitive);
                return missingEndpoint;
              };

              if (reply->error() == QNetworkReply::NoError) {
                const QJsonDocument response = QJsonDocument::fromJson(responseData);
                const QJsonObject obj = response.object();
                const int code = obj.value(QStringLiteral("code")).toInt(-1);
                const QString msg = obj.value(QStringLiteral("msg")).toString();

                if (code == 0 || msg.contains(QStringLiteral("success"),
                                              Qt::CaseInsensitive)) {
                  emit mediaDeleted(filePath);
                } else if (legacyFallbackAllowed &&
                           (msg.contains(QStringLiteral("not implemented"),
                                         Qt::CaseInsensitive) ||
                            msg.contains(QStringLiteral("not found"),
                                         Qt::CaseInsensitive) ||
                            msg.contains(QStringLiteral("404")))) {
                  qWarning() << "[HttpClient] /album/delete unavailable, retrying legacy delete";
                  reply->deleteLater();
                  (*sendDeleteRequest)(false);
                  return;
                } else if (msg.contains(QStringLiteral("not implemented"),
                                        Qt::CaseInsensitive)) {
                  emit deleteError(
                      filePath,
                      tr("Delete not supported by DWARF firmware. Please delete files using the DWARF app."));
                } else {
                  emit deleteError(filePath,
                                   msg.isEmpty() ? tr("Unknown error") : msg);
                }
              } else if (retryLegacy()) {
                qWarning() << "[HttpClient] /album/delete HTTP endpoint missing, retrying legacy delete";
                reply->deleteLater();
                (*sendDeleteRequest)(false);
                return;
              } else {
                emit deleteError(filePath, reply->errorString());
              }

              reply->deleteLater();
            });
  };

  (*sendDeleteRequest)(true);
}

void DwarfHttpClient::setDeviceName(const QString &newName,
                                    const QString &oldName) {
  const QString trimmedName = newName.trimmed();
  if (trimmedName.isEmpty()) {
    emit deviceSettingError(ChangeName, tr("Device name must not be empty"));
    return;
  }

  QUrl url(QString("http://%1:%2/setDeviceNameAndPsd").arg(m_ip).arg(HTTP_PORT));
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/json"));

  QJsonObject payload;
  payload.insert(QStringLiteral("mode"), ChangeName);
  payload.insert(QStringLiteral("oldValue"), oldName);
  payload.insert(QStringLiteral("newValue"), trimmedName);

  QNetworkReply *reply =
      m_manager->post(request, QJsonDocument(payload).toJson());
  connect(reply, &QNetworkReply::finished, this,
          [this, reply, trimmedName]() {
            const QByteArray responseData = reply->readAll();
            const QJsonDocument response = QJsonDocument::fromJson(responseData);
            const QJsonObject obj = response.object();
            const int code = obj.value(QStringLiteral("code")).toInt(-1);
            const QString message = responseMessage(obj);

            if (reply->error() == QNetworkReply::NoError && code == 0) {
              const QJsonObject data = obj.value(QStringLiteral("data")).toObject();
              const QString appliedName =
                  data.value(QStringLiteral("deviceName")).toString(trimmedName);
              emit deviceSettingChanged(ChangeName, appliedName);
            } else {
              emit deviceSettingError(ChangeName,
                                      message.isEmpty() ? reply->errorString()
                                                        : message);
            }
            reply->deleteLater();
          });
}

void DwarfHttpClient::setDevicePassword(const QString &oldPassword,
                                        const QString &newPassword) {
  if (oldPassword.isEmpty()) {
    emit deviceSettingError(ChangePassword,
                            tr("Current password must not be empty"));
    return;
  }
  if (newPassword.isEmpty()) {
    emit deviceSettingError(ChangePassword,
                            tr("New password must not be empty"));
    return;
  }

  QUrl url(QString("http://%1:%2/setDeviceNameAndPsd").arg(m_ip).arg(HTTP_PORT));
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/json"));

  QJsonObject payload;
  payload.insert(QStringLiteral("mode"), ChangePassword);
  payload.insert(QStringLiteral("oldValue"), oldPassword);
  payload.insert(QStringLiteral("newValue"), newPassword);

  QNetworkReply *reply =
      m_manager->post(request, QJsonDocument(payload).toJson());
  connect(reply, &QNetworkReply::finished, this,
          [this, reply, newPassword]() {
            const QByteArray responseData = reply->readAll();
            const QJsonDocument response = QJsonDocument::fromJson(responseData);
            const QJsonObject obj = response.object();
            const int code = obj.value(QStringLiteral("code")).toInt(-1);
            const QString message = responseMessage(obj);

            if (reply->error() == QNetworkReply::NoError && code == 0) {
              const QJsonObject data = obj.value(QStringLiteral("data")).toObject();
              const QString appliedPassword =
                  data.value(QStringLiteral("devicePwd")).toString(newPassword);
              emit deviceSettingChanged(ChangePassword, appliedPassword);
            } else {
              emit deviceSettingError(ChangePassword,
                                      message.isEmpty() ? reply->errorString()
                                                        : message);
            }
            reply->deleteLater();
          });
}

void DwarfHttpClient::uploadFirmware(const QString &filePath) {
  QFile firmwareFile(filePath);
  if (!firmwareFile.open(QIODevice::ReadOnly)) {
    emit firmwareUploadFinished(filePath, false, -1,
                                tr("Firmware file could not be opened"));
    return;
  }

  const QByteArray md5 =
      QCryptographicHash::hash(firmwareFile.readAll(), QCryptographicHash::Md5)
          .toHex();
  firmwareFile.close();

  auto sendUploadRequest =
      std::make_shared<std::function<void(bool preferDocFieldName)>>();

  *sendUploadRequest = [this, filePath, md5,
                        sendUploadRequest](bool preferDocFieldName) {
    auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    auto *file = new QFile(filePath, multiPart);
    if (!file->open(QIODevice::ReadOnly)) {
      emit firmwareUploadFinished(filePath, false, -1,
                                  tr("Firmware file could not be opened"));
      multiPart->deleteLater();
      return;
    }

    const QFileInfo info(filePath);
    const QString fieldName = preferDocFieldName
                                  ? QStringLiteral("fiwmwareFileName")
                                  : QStringLiteral("firmwareFileName");

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                       QStringLiteral("application/octet-stream"));
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QStringLiteral("form-data; name=\"%1\"; filename=\"%2\"")
            .arg(fieldName, info.fileName()));
    filePart.setBodyDevice(file);

    QHttpPart md5Part;
    md5Part.setHeader(QNetworkRequest::ContentDispositionHeader,
                      QStringLiteral("form-data; name=\"md5\""));
    md5Part.setBody(md5);

    multiPart->append(filePart);
    multiPart->append(md5Part);

    QUrl url(QString("http://%1:%2/uploadFirmware").arg(m_ip).arg(HTTP_PORT));
    QNetworkRequest request(url);
    QNetworkReply *reply = m_manager->post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, filePath, preferDocFieldName,
             sendUploadRequest]() {
              const QByteArray responseData = reply->readAll();
              const QJsonDocument response = QJsonDocument::fromJson(responseData);
              const QJsonObject obj = response.object();
              const int code = obj.value(QStringLiteral("code")).toInt(-999);
              const QString message = responseMessage(obj);

              const bool invalidParam =
                  code == -2 ||
                  message.contains(QStringLiteral("invalid"),
                                   Qt::CaseInsensitive) ||
                  message.contains(QStringLiteral("parameter"),
                                   Qt::CaseInsensitive);

              if (reply->error() == QNetworkReply::NoError && code == 0) {
                emit firmwareUploadFinished(
                    filePath, true, code,
                    message.isEmpty() ? tr("Firmware uploaded successfully")
                                      : message);
              } else if (preferDocFieldName && invalidParam) {
                qWarning() << "[HttpClient] uploadFirmware rejected documented field name, retrying canonical field";
                reply->deleteLater();
                (*sendUploadRequest)(false);
                return;
              } else {
                const QString fallbackMessage =
                    message.isEmpty() ? reply->errorString() : message;
                emit firmwareUploadFinished(filePath, false, code,
                                            fallbackMessage);
              }

              reply->deleteLater();
            });
  };

  (*sendUploadRequest)(true);
}
