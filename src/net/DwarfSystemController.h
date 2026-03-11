#pragma once

#include <QObject>
#include <QString>

class QByteArray;
class DwarfWebSocketClient;

class DwarfSystemController : public QObject {
  Q_OBJECT

public:
  explicit DwarfSystemController(QObject *parent = nullptr);

  void setClient(DwarfWebSocketClient *client);

  void setTime(qint64 timestamp);
  void setTimezone(const QString &timezone);
  void setMtpMode(bool enabled);
  void setCpuMode(int mode);
  void openRgb();
  void closeRgb();
  void openPowerIndicator();
  void closePowerIndicator();
  void powerDown();
  void reboot();

public slots:
  void handleSystemMessage(quint32 cmd, const QByteArray &data);
  void handleRgbPowerMessage(quint32 cmd, const QByteArray &data);
  void handleNotification(quint32 cmd, const QByteArray &data);

signals:
  void errorOccurred(const QString &message);
  void statusMessage(const QString &message);
  void timeSynced();
  void timezoneChanged(const QString &timezone);
  void mtpModeChanged(bool enabled);
  void cpuModeChanged(int mode);
  void rgbStateChanged(bool enabled);
  void powerIndicatorStateChanged(bool enabled);
  void poweringOff();
  void rebooting();
  void poweredOff();

private:
  enum class PendingAction {
    None,
    SetTime,
    SetTimezone,
    SetMtpMode,
    SetCpuMode,
    OpenRgb,
    CloseRgb,
    OpenPowerIndicator,
    ClosePowerIndicator,
    PowerDown,
    Reboot,
  };

  void sendCommand(quint32 moduleId, quint32 cmd, const QByteArray &data,
                   PendingAction action);
  void handleResponse(quint32 cmd, const QByteArray &data, bool rgbPowerModule);
  void emitActionSuccess(PendingAction action);
  QString actionName(PendingAction action) const;

  DwarfWebSocketClient *m_client = nullptr;
  PendingAction m_pendingSystemAction = PendingAction::None;
  PendingAction m_pendingRgbPowerAction = PendingAction::None;
  QString m_lastTimezone;
  bool m_lastMtpMode = false;
  int m_lastCpuMode = 0;
};
