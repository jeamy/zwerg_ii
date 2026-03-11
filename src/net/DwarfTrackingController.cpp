#include "DwarfTrackingController.h"

#include "DwarfWebSocketClient.h"
#include "ProtobufHelper.h"
#include "base.pb.h"
#include "notify.pb.h"
#include "tracking.pb.h"

#include <QByteArray>

namespace TrackCmd {
constexpr quint32 MODULE_ID = 7;
constexpr quint32 START_TRACK = 14800;
constexpr quint32 STOP_TRACK = 14801;
constexpr quint32 START_SENTRY = 14802;
constexpr quint32 STOP_SENTRY = 14803;
constexpr quint32 START_MOT = 14804;
constexpr quint32 MOT_TRACK_ONE = 14805;
constexpr quint32 START_UFO = 14806;
constexpr quint32 STOP_UFO = 14807;
constexpr quint32 MOT_WIDE_TRACK_ONE = 14808;
constexpr quint32 WIDE_TELE_TRACK_SWITCH = 14809;
constexpr quint32 UFO_HAND_AUTO_MODE = 14810;
}

namespace TrackNotify {
constexpr quint32 TRACK_RESULT = 15225;
constexpr quint32 SENTRY_STATE = 15231;
constexpr quint32 SENTRY_TRACK_RESULT = 15232;
constexpr quint32 MULTI_TRACK_RESULT = 15238;
constexpr quint32 UFO_STATE = 15240;
}

DwarfTrackingController::DwarfTrackingController(QObject *parent)
    : QObject(parent) {}

void DwarfTrackingController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  if (!m_client)
    clearResults();
}

void DwarfTrackingController::startTrack(const QRect &box) {
  dwarf::ReqStartTrack req;
  req.set_x(box.x());
  req.set_y(box.y());
  req.set_w(box.width());
  req.set_h(box.height());
  sendCommand(TrackCmd::START_TRACK, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::stopTrack() {
  dwarf::ReqStopTrack req;
  sendCommand(TrackCmd::STOP_TRACK, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::startSentryMode(int mode) {
  dwarf::ReqStartSentryMode req;
  req.set_mode(mode);
  sendCommand(TrackCmd::START_SENTRY, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::stopSentryMode() {
  dwarf::ReqStopSentryMode req;
  sendCommand(TrackCmd::STOP_SENTRY, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::startMot() {
  dwarf::ReqMOTStart req;
  sendCommand(TrackCmd::START_MOT, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::motTrackOne(int id) {
  dwarf::ReqMOTTrackOne req;
  req.set_id(id);
  sendCommand(TrackCmd::MOT_TRACK_ONE, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::wideMotTrackOne(int id) {
  dwarf::ReqMOTTrackOne req;
  req.set_id(id);
  sendCommand(TrackCmd::MOT_WIDE_TRACK_ONE, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::startUfoMode(int mode) {
  dwarf::ReqStartSentryMode req;
  req.set_mode(mode);
  sendCommand(TrackCmd::START_UFO, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::stopUfoMode() {
  dwarf::ReqStopSentryMode req;
  sendCommand(TrackCmd::STOP_UFO, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::setWideTeleTrackSwitch(int mode) {
  dwarf::ReqWideTeleTrackSwitch req;
  req.set_mode(mode);
  sendCommand(TrackCmd::WIDE_TELE_TRACK_SWITCH, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::setUfoHandAutoMode(int mode) {
  dwarf::ReqUfoHandAutoMode req;
  req.set_mode(mode);
  sendCommand(TrackCmd::UFO_HAND_AUTO_MODE, ProtobufHelper::serialize(req));
}

void DwarfTrackingController::handleTrackMessage(quint32 cmd,
                                                 const QByteArray &data) {
  handleTrackResponse(cmd, data);
}

void DwarfTrackingController::handleNotification(quint32 cmd,
                                                 const QByteArray &data) {
  switch (cmd) {
  case TrackNotify::TRACK_RESULT:
  case TrackNotify::SENTRY_TRACK_RESULT: {
    dwarf::ResNotifyTrackResult res;
    if (!ProtobufHelper::parse(data, res))
      return;
    m_currentTrackRect = QRect(res.x(), res.y(), res.w(), res.h());
    m_currentTrackId = (cmd == TrackNotify::TRACK_RESULT ||
                        cmd == TrackNotify::SENTRY_TRACK_RESULT)
                           ? -1
                           : res.id();
    emit trackResultChanged();
    break;
  }
  case TrackNotify::MULTI_TRACK_RESULT: {
    dwarf::ResNotifyMultiTrackResult res;
    if (!ProtobufHelper::parse(data, res))
      return;
    m_multiTrackResults.clear();
    m_multiTrackResults.reserve(res.results_size());
    for (int i = 0; i < res.results_size(); ++i) {
      const auto &it = res.results(i);
      TrackResult result;
      result.rect = QRect(it.x(), it.y(), it.w(), it.h());
      result.id = it.id();
      m_multiTrackResults.push_back(result);
    }
    emit multiTrackResultsChanged();
    break;
  }
  case TrackNotify::SENTRY_STATE: {
    dwarf::ResNotifyStateSentryMode res;
    if (!ProtobufHelper::parse(data, res))
      return;
    m_sentryState = res.state();
    emit sentryStateChanged(m_sentryState);
    break;
  }
  case TrackNotify::UFO_STATE: {
    dwarf::ResNotifyStateSentryMode res;
    if (!ProtobufHelper::parse(data, res))
      return;
    m_ufoState = res.state();
    emit ufoStateChanged(m_ufoState);
    break;
  }
  default:
    break;
  }
}

void DwarfTrackingController::sendCommand(quint32 cmd, const QByteArray &data) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred(tr("Tracking client not connected"));
    return;
  }
  m_client->sendCommand(TrackCmd::MODULE_ID, cmd, data);
}

void DwarfTrackingController::handleTrackResponse(quint32 cmd,
                                                  const QByteArray &data) {
  dwarf::ComResponse res;
  if (!ProtobufHelper::parse(data, res)) {
    emit errorOccurred(tr("Tracking response parse failed for cmd %1").arg(cmd));
    return;
  }

  if (res.code() != 0) {
    emit errorOccurred(tr("Tracking command %1 failed (code %2)")
                           .arg(cmd)
                           .arg(res.code()));
    return;
  }

  switch (cmd) {
  case TrackCmd::START_TRACK:
    emit statusMessage(tr("Tracking started"));
    emit trackSelectionStarted();
    break;
  case TrackCmd::STOP_TRACK:
    clearResults();
    emit statusMessage(tr("Tracking stopped"));
    emit trackStopped();
    break;
  case TrackCmd::START_SENTRY:
    emit statusMessage(tr("Sentry mode started"));
    break;
  case TrackCmd::STOP_SENTRY:
    clearResults();
    m_sentryState = 0;
    emit sentryStateChanged(m_sentryState);
    emit statusMessage(tr("Sentry mode stopped"));
    break;
  case TrackCmd::START_MOT:
    m_motActive = true;
    emit motModeChanged(true);
    emit statusMessage(tr("Multi-object tracking started"));
    break;
  case TrackCmd::MOT_TRACK_ONE:
    emit statusMessage(tr("Tracking MOT target"));
    break;
  case TrackCmd::START_UFO:
    emit statusMessage(tr("UFO mode started"));
    break;
  case TrackCmd::STOP_UFO:
    clearResults();
    m_ufoState = 0;
    emit ufoStateChanged(m_ufoState);
    emit statusMessage(tr("UFO mode stopped"));
    break;
  case TrackCmd::MOT_WIDE_TRACK_ONE:
    emit statusMessage(tr("Tracking wide MOT target"));
    break;
  case TrackCmd::WIDE_TELE_TRACK_SWITCH:
    emit statusMessage(tr("Tracking source switched"));
    break;
  case TrackCmd::UFO_HAND_AUTO_MODE:
    emit statusMessage(tr("UFO hand/auto mode updated"));
    break;
  default:
    break;
  }
}

void DwarfTrackingController::clearResults() {
  m_currentTrackRect = QRect();
  m_currentTrackId = -1;
  m_multiTrackResults.clear();
  emit trackResultChanged();
  emit multiTrackResultsChanged();
}
