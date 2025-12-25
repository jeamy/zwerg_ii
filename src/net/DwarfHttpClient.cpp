#include "DwarfHttpClient.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>
#include <functional>
#include <memory>

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
  // DWARF II HTTP API endpoint for deleting media
  // Try the /sdcard/deleteFile endpoint which is more likely to exist
  QUrl url(QString("http://%1:%2/sdcard/deleteFile").arg(m_ip).arg(HTTP_PORT));
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QStringLiteral("application/json"));

  // Try with filePath directly
  QJsonObject payload;
  payload.insert(QStringLiteral("filePath"), filePath);
  QJsonDocument doc(payload);

  qDebug() << "[HttpClient] Deleting media:" << filePath;
  qDebug() << "[HttpClient] URL:" << url.toString();
  qDebug() << "[HttpClient] Request:" << doc.toJson();

  QNetworkReply *reply = m_manager->post(request, doc.toJson());

  connect(reply, &QNetworkReply::finished, this, [this, reply, filePath]() {
    QByteArray responseData = reply->readAll();
    qDebug() << "[HttpClient] Delete response code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[HttpClient] Delete response:" << responseData;

    if (reply->error() == QNetworkReply::NoError) {
      QJsonDocument response = QJsonDocument::fromJson(responseData);
      QJsonObject obj = response.object();
      int code = obj.value(QStringLiteral("code")).toInt(-1);
      QString msg = obj.value(QStringLiteral("msg")).toString();

      if (code == 0 || msg.contains("success", Qt::CaseInsensitive)) {
        emit mediaDeleted(filePath);
      } else if (msg.contains("not implemented", Qt::CaseInsensitive)) {
        // Try alternative: maybe we need to use WebSocket command
        qWarning() << "[HttpClient] Delete API not implemented on this DWARF firmware";
        emit deleteError(filePath, tr("Delete not supported by DWARF firmware. Please delete files using the DWARF app."));
      } else {
        emit deleteError(filePath, msg.isEmpty() ? tr("Unknown error") : msg);
      }
    } else {
      emit deleteError(filePath, reply->errorString());
    }
    reply->deleteLater();
  });
}
