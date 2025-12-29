#include "AppConfig.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonValue>

AppConfig *AppConfig::s_instance = nullptr;

AppConfig *AppConfig::instance() {
  if (!s_instance) {
    s_instance = new AppConfig();
  }
  return s_instance;
}

AppConfig::AppConfig() {
  // Config lives next to the executable
  const QString appDir = QCoreApplication::applicationDirPath();
  m_configPath = QDir(appDir).filePath("config.json");
}

QString AppConfig::configFilePath() const { return m_configPath; }

bool AppConfig::load() {
  QMutexLocker locker(&m_mutex);

  QFile f(m_configPath);
  if (!f.exists()) {
    m_root = QJsonObject();
    return true;
  }

  if (!f.open(QIODevice::ReadOnly)) {
    return false;
  }

  const QByteArray data = f.readAll();
  f.close();

  QJsonParseError err;
  const QJsonDocument doc = QJsonDocument::fromJson(data, &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    m_root = QJsonObject();
    return false;
  }

  m_root = doc.object();
  return true;
}

bool AppConfig::save() {
  QMutexLocker locker(&m_mutex);

  QJsonDocument doc(m_root);
  const QByteArray data = doc.toJson(QJsonDocument::Indented);

  QFile f(m_configPath);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    return false;
  }

  const qint64 written = f.write(data);
  f.close();
  return written == data.size();
}

QVariant AppConfig::getValue(const QString &section, const QString &key,
                            const QVariant &defaultValue) const {
  QMutexLocker locker(&m_mutex);

  const QJsonValue secVal = m_root.value(section);
  if (!secVal.isObject()) {
    return defaultValue;
  }

  const QJsonObject secObj = secVal.toObject();
  if (!secObj.contains(key)) {
    return defaultValue;
  }

  return secObj.value(key).toVariant();
}

void AppConfig::setValue(const QString &section, const QString &key,
                         const QVariant &value) {
  QMutexLocker locker(&m_mutex);

  QJsonObject secObj;
  const QJsonValue secVal = m_root.value(section);
  if (secVal.isObject()) {
    secObj = secVal.toObject();
  }

  secObj.insert(key, QJsonValue::fromVariant(value));
  m_root.insert(section, secObj);
}

QJsonObject AppConfig::getSection(const QString &section) const {
  QMutexLocker locker(&m_mutex);

  const QJsonValue secVal = m_root.value(section);
  if (!secVal.isObject()) {
    return QJsonObject();
  }

  return secVal.toObject();
}

void AppConfig::setSection(const QString &section, const QJsonObject &data) {
  QMutexLocker locker(&m_mutex);
  m_root.insert(section, data);
}
