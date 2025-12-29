#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QString>
#include <QVariant>

class AppConfig {
public:
  static AppConfig *instance();

  bool load();
  bool save();

  QVariant getValue(const QString &section, const QString &key,
                    const QVariant &defaultValue = QVariant()) const;
  void setValue(const QString &section, const QString &key, const QVariant &value);

  QJsonObject getSection(const QString &section) const;
  void setSection(const QString &section, const QJsonObject &data);

  QString configFilePath() const;

private:
  AppConfig();

  static AppConfig *s_instance;

  QString m_configPath;
  mutable QMutex m_mutex;
  QJsonObject m_root;
};
