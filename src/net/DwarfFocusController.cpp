#include "DwarfFocusController.h"
#include "ProtobufHelper.h"
#include "base.pb.h"
#include "notify.pb.h"

#include <QDebug>

using dwarf::ReqNormalAutoFocus;
using dwarf::ReqManualSingleStepFocus;
using dwarf::ReqStartManualContinuFocus;
using dwarf::ReqStopManualContinuFocus;
using dwarf::ReqAstroAutoFocus;
using dwarf::ReqStopAstroAutoFocus;

DwarfFocusController::DwarfFocusController(QObject *parent)
    : QObject(parent), m_client(nullptr) {}

void DwarfFocusController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  qWarning() << "[DwarfFocusController] setClient called with"
             << (client ? "valid" : "null") << "client";
}

void DwarfFocusController::autoFocusNormal() {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfFocusController] Cannot start auto focus, client not connected";
    emit errorOccurred("Focus client not connected");
    return;
  }

  ReqNormalAutoFocus req;
  req.set_mode(0);
  req.set_center_x(0);
  req.set_center_y(0);

  sendCommand(cmdAutoFocus(), ProtobufHelper::serialize(req),
              tr("Cannot start auto focus"));
}

void DwarfFocusController::manualStepNear() {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfFocusController] Cannot do manual focus near, client not connected";
    emit errorOccurred("Focus client not connected");
    return;
  }

  ReqManualSingleStepFocus req;
  req.set_direction(1);
  sendCommand(cmdManualSingleStep(), ProtobufHelper::serialize(req),
              tr("Cannot focus near"));
}

void DwarfFocusController::manualStepFar() {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfFocusController] Cannot do manual focus far, client not connected";
    emit errorOccurred("Focus client not connected");
    return;
  }

  ReqManualSingleStepFocus req;
  req.set_direction(0);
  sendCommand(cmdManualSingleStep(), ProtobufHelper::serialize(req),
              tr("Cannot focus far"));
}

quint32 DwarfFocusController::moduleId() const { return 8u; }

quint32 DwarfFocusController::cmdAutoFocus() const { return 15000u; }

quint32 DwarfFocusController::cmdManualSingleStep() const { return 15001u; }

quint32 DwarfFocusController::cmdManualContinuousStart() const { return 15002u; }

quint32 DwarfFocusController::cmdManualContinuousStop() const { return 15003u; }

quint32 DwarfFocusController::cmdAstroAutoFocusStart() const { return 15004u; }

quint32 DwarfFocusController::cmdAstroAutoFocusStop() const { return 15005u; }

void DwarfFocusController::startManualContinuousNear() {
  ReqStartManualContinuFocus req;
  req.set_direction(1);
  sendCommand(cmdManualContinuousStart(), ProtobufHelper::serialize(req),
              tr("Cannot start continuous near focus"));
}

void DwarfFocusController::startManualContinuousFar() {
  ReqStartManualContinuFocus req;
  req.set_direction(0);
  sendCommand(cmdManualContinuousStart(), ProtobufHelper::serialize(req),
              tr("Cannot start continuous far focus"));
}

void DwarfFocusController::stopManualContinuousFocus() {
  ReqStopManualContinuFocus req;
  sendCommand(cmdManualContinuousStop(), ProtobufHelper::serialize(req),
              tr("Cannot stop continuous focus"));
}

void DwarfFocusController::startAstroAutoFocus(bool fastMode) {
  ReqAstroAutoFocus req;
  req.set_mode(fastMode ? 1u : 0u);
  sendCommand(cmdAstroAutoFocusStart(), ProtobufHelper::serialize(req),
              tr("Cannot start astro autofocus"));
}

void DwarfFocusController::stopAstroAutoFocus() {
  ReqStopAstroAutoFocus req;
  sendCommand(cmdAstroAutoFocusStop(), ProtobufHelper::serialize(req),
              tr("Cannot stop astro autofocus"));
}

void DwarfFocusController::handleFocusMessage(quint32 cmd,
                                              const QByteArray &data) {
  dwarf::ComResponse res;
  if (!ProtobufHelper::parse(data, res)) {
    emit errorOccurred(tr("Focus response parse failed for cmd %1").arg(cmd));
    return;
  }

  if (res.code() != 0) {
    emit errorOccurred(
        tr("Focus command %1 failed (code %2)").arg(cmd).arg(res.code()));
    return;
  }

  switch (cmd) {
  case 15000u:
    emit statusMessage(tr("Auto focus started"));
    break;
  case 15001u:
    emit statusMessage(tr("Single-step focus sent"));
    break;
  case 15002u:
    emit statusMessage(tr("Continuous focus started"));
    break;
  case 15003u:
    emit statusMessage(tr("Continuous focus stopped"));
    break;
  case 15004u:
    emit statusMessage(tr("Astro autofocus started"));
    break;
  case 15005u:
    emit statusMessage(tr("Astro autofocus stopped"));
    break;
  default:
    break;
  }
}

void DwarfFocusController::handleNotification(quint32 cmd,
                                              const QByteArray &data) {
  if (cmd != 15257u)
    return;

  dwarf::ResNotifyFocus res;
  if (!ProtobufHelper::parse(data, res))
    return;
  emit focusPositionChanged(res.position());
}

void DwarfFocusController::sendCommand(quint32 cmd, const QByteArray &data,
                                       const QString &errorContext) {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfFocusController]" << errorContext
               << "- client not connected";
    emit errorOccurred(tr("Focus client not connected"));
    return;
  }

  m_client->sendCommand(moduleId(), cmd, data);
}
