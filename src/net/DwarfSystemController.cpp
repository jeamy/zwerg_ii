#include "DwarfSystemController.h"

#include "DwarfWebSocketClient.h"
#include "ProtobufHelper.h"
#include "base.pb.h"
#include "notify.pb.h"
#include "system.pb.h"

#include <QByteArray>
#include <QDebug>

namespace SystemModuleCmd {
constexpr quint32 MODULE_ID = 4;
constexpr quint32 SET_TIME = 13000;
constexpr quint32 SET_TIMEZONE = 13001;
constexpr quint32 SET_MTP_MODE = 13002;
constexpr quint32 SET_CPU_MODE = 13003;
}

namespace RgbPowerModuleCmd {
constexpr quint32 MODULE_ID = 5;
constexpr quint32 OPEN_RGB = 13500;
constexpr quint32 CLOSE_RGB = 13501;
constexpr quint32 POWER_DOWN = 13502;
constexpr quint32 OPEN_POWER_IND = 13503;
constexpr quint32 CLOSE_POWER_IND = 13504;
constexpr quint32 REBOOT = 13505;
}

namespace NotifyCmd {
constexpr quint32 RGB_STATE = 15221;
constexpr quint32 POWER_IND_STATE = 15222;
constexpr quint32 MTP_STATE = 15224;
constexpr quint32 CPU_MODE = 15227;
constexpr quint32 POWER_OFF = 15229;
}

DwarfSystemController::DwarfSystemController(QObject *parent)
    : QObject(parent) {}

void DwarfSystemController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  if (!m_client) {
    m_pendingSystemAction = PendingAction::None;
    m_pendingRgbPowerAction = PendingAction::None;
  }
}

void DwarfSystemController::setTime(qint64 timestamp) {
  dwarf::ReqSetTime req;
  req.set_timestamp(static_cast<quint64>(timestamp));
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(SystemModuleCmd::MODULE_ID, SystemModuleCmd::SET_TIME, data,
              PendingAction::SetTime);
}

void DwarfSystemController::setTimezone(const QString &timezone) {
  dwarf::ReqSetTimezone req;
  req.set_timezone(timezone.toStdString());
  m_lastTimezone = timezone;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(SystemModuleCmd::MODULE_ID, SystemModuleCmd::SET_TIMEZONE, data,
              PendingAction::SetTimezone);
}

void DwarfSystemController::setMtpMode(bool enabled) {
  dwarf::ReqSetMtpMode req;
  req.set_mode(enabled ? 1 : 0);
  m_lastMtpMode = enabled;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(SystemModuleCmd::MODULE_ID, SystemModuleCmd::SET_MTP_MODE, data,
              PendingAction::SetMtpMode);
}

void DwarfSystemController::setCpuMode(int mode) {
  dwarf::ReqSetCpuMode req;
  req.set_mode(mode);
  m_lastCpuMode = mode;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(SystemModuleCmd::MODULE_ID, SystemModuleCmd::SET_CPU_MODE, data,
              PendingAction::SetCpuMode);
}

void DwarfSystemController::openRgb() {
  dwarf::ReqOpenRgb req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::OPEN_RGB, data,
              PendingAction::OpenRgb);
}

void DwarfSystemController::closeRgb() {
  dwarf::ReqCloseRgb req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::CLOSE_RGB, data,
              PendingAction::CloseRgb);
}

void DwarfSystemController::openPowerIndicator() {
  dwarf::ReqOpenPowerInd req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::OPEN_POWER_IND,
              data, PendingAction::OpenPowerIndicator);
}

void DwarfSystemController::closePowerIndicator() {
  dwarf::ReqClosePowerInd req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::CLOSE_POWER_IND,
              data, PendingAction::ClosePowerIndicator);
}

void DwarfSystemController::powerDown() {
  dwarf::ReqPowerDown req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::POWER_DOWN, data,
              PendingAction::PowerDown);
}

void DwarfSystemController::reboot() {
  dwarf::ReqReboot req;
  const QByteArray data = ProtobufHelper::serialize(req);
  sendCommand(RgbPowerModuleCmd::MODULE_ID, RgbPowerModuleCmd::REBOOT, data,
              PendingAction::Reboot);
}

void DwarfSystemController::handleSystemMessage(quint32 cmd,
                                                const QByteArray &data) {
  handleResponse(cmd, data, false);
}

void DwarfSystemController::handleRgbPowerMessage(quint32 cmd,
                                                  const QByteArray &data) {
  handleResponse(cmd, data, true);
}

void DwarfSystemController::handleNotification(quint32 cmd,
                                               const QByteArray &data) {
  switch (cmd) {
  case NotifyCmd::RGB_STATE: {
    dwarf::ResNotifyRgbState res;
    if (!ProtobufHelper::parse(data, res))
      return;
    emit rgbStateChanged(res.state() != 0);
    break;
  }
  case NotifyCmd::POWER_IND_STATE: {
    dwarf::ResNotifyPowerIndState res;
    if (!ProtobufHelper::parse(data, res))
      return;
    emit powerIndicatorStateChanged(res.state() != 0);
    break;
  }
  case NotifyCmd::MTP_STATE: {
    dwarf::ResNotifyMTPState res;
    if (!ProtobufHelper::parse(data, res))
      return;
    emit mtpModeChanged(res.mode() != 0);
    break;
  }
  case NotifyCmd::CPU_MODE: {
    dwarf::ResNotifyCPUMode res;
    if (!ProtobufHelper::parse(data, res))
      return;
    emit cpuModeChanged(res.mode());
    break;
  }
  case NotifyCmd::POWER_OFF:
    emit poweredOff();
    break;
  default:
    break;
  }
}

void DwarfSystemController::sendCommand(quint32 moduleId, quint32 cmd,
                                        const QByteArray &data,
                                        PendingAction action) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred(tr("System client not connected"));
    return;
  }

  if (moduleId == SystemModuleCmd::MODULE_ID)
    m_pendingSystemAction = action;
  else
    m_pendingRgbPowerAction = action;

  m_client->sendCommand(moduleId, cmd, data);
}

void DwarfSystemController::handleResponse(quint32 cmd, const QByteArray &data,
                                           bool rgbPowerModule) {
  dwarf::ComResponse res;
  if (!ProtobufHelper::parse(data, res)) {
    qWarning() << "[DwarfSystemController] Failed to parse response cmd" << cmd
               << "module" << (rgbPowerModule ? 5 : 4)
               << "payload" << data.toHex();
    return;
  }

  PendingAction &pending =
      rgbPowerModule ? m_pendingRgbPowerAction : m_pendingSystemAction;
  const PendingAction action = pending;
  pending = PendingAction::None;

  if (res.code() != 0) {
    emit errorOccurred(tr("%1 failed (code %2)")
                           .arg(actionName(action))
                           .arg(res.code()));
    return;
  }

  emitActionSuccess(action);
}

void DwarfSystemController::emitActionSuccess(PendingAction action) {
  switch (action) {
  case PendingAction::SetTime:
    emit statusMessage(tr("Device time synced"));
    emit timeSynced();
    break;
  case PendingAction::SetTimezone:
    emit statusMessage(tr("Timezone updated"));
    emit timezoneChanged(m_lastTimezone);
    break;
  case PendingAction::SetMtpMode:
    emit statusMessage(tr("MTP mode updated"));
    emit mtpModeChanged(m_lastMtpMode);
    break;
  case PendingAction::SetCpuMode:
    emit statusMessage(tr("CPU mode updated"));
    emit cpuModeChanged(m_lastCpuMode);
    break;
  case PendingAction::OpenRgb:
    emit statusMessage(tr("RGB ring enabled"));
    emit rgbStateChanged(true);
    break;
  case PendingAction::CloseRgb:
    emit statusMessage(tr("RGB ring disabled"));
    emit rgbStateChanged(false);
    break;
  case PendingAction::OpenPowerIndicator:
    emit statusMessage(tr("Power indicator enabled"));
    emit powerIndicatorStateChanged(true);
    break;
  case PendingAction::ClosePowerIndicator:
    emit statusMessage(tr("Power indicator disabled"));
    emit powerIndicatorStateChanged(false);
    break;
  case PendingAction::PowerDown:
    emit statusMessage(tr("Shutdown requested"));
    emit poweringOff();
    break;
  case PendingAction::Reboot:
    emit statusMessage(tr("Reboot requested"));
    emit rebooting();
    break;
  case PendingAction::None:
    break;
  }
}

QString DwarfSystemController::actionName(PendingAction action) const {
  switch (action) {
  case PendingAction::SetTime:
    return tr("Set time");
  case PendingAction::SetTimezone:
    return tr("Set timezone");
  case PendingAction::SetMtpMode:
    return tr("Set MTP mode");
  case PendingAction::SetCpuMode:
    return tr("Set CPU mode");
  case PendingAction::OpenRgb:
    return tr("Enable RGB ring");
  case PendingAction::CloseRgb:
    return tr("Disable RGB ring");
  case PendingAction::OpenPowerIndicator:
    return tr("Enable power indicator");
  case PendingAction::ClosePowerIndicator:
    return tr("Disable power indicator");
  case PendingAction::PowerDown:
    return tr("Shutdown");
  case PendingAction::Reboot:
    return tr("Reboot");
  case PendingAction::None:
    return tr("System action");
  }

  return tr("System action");
}
