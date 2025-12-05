#include "DwarfMotorController.h"
#include "ProtobufHelper.h"

#include <QDebug>
#include <algorithm>

using dwarf::ReqMotorRun;
using dwarf::ReqMotorStop;

namespace {
inline double clampDouble(double value, double minV, double maxV) {
  return std::max(minV, std::min(maxV, value));
}
} // namespace

DwarfMotorController::DwarfMotorController(QObject *parent)
    : QObject(parent), m_client(nullptr) {}

void DwarfMotorController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  qWarning() << "[DwarfMotorController] setClient called with"
             << (client ? "valid" : "null") << "client";
}

void DwarfMotorController::runMotor(Axis axis, bool directionRightOrUp,
                                    double speed, int speedRamping,
                                    int resolutionLevel) {
  qWarning() << "[DwarfMotorController] runMotor axis" << static_cast<int>(axis)
             << "direction" << directionRightOrUp << "speed" << speed;

  if (!m_client || !m_client->isConnected()) {
    qWarning()
        << "[DwarfMotorController] Cannot run motor, client not connected";
    emit errorOccurred("Motor client not connected");
    return;
  }

  speed = clampDouble(speed, 0.1, 30.0);
  speedRamping = std::max(0, std::min(1000, speedRamping));
  resolutionLevel = std::max(0, std::min(8, resolutionLevel));

  ReqMotorRun req;
  req.set_id(static_cast<int>(axis));
  req.set_speed(speed);
  req.set_direction(directionRightOrUp);
  req.set_speed_ramping(speedRamping);
  req.set_resolution_level(resolutionLevel);

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleId(), cmdRun(), data);
}

void DwarfMotorController::stopMotor(Axis axis) {
  qWarning() << "[DwarfMotorController] stopMotor axis"
             << static_cast<int>(axis);

  if (!m_client || !m_client->isConnected()) {
    qWarning()
        << "[DwarfMotorController] Cannot stop motor, client not connected";
    emit errorOccurred("Motor client not connected");
    return;
  }

  ReqMotorStop req;
  req.set_id(static_cast<int>(axis));

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleId(), cmdStop(), data);
}

quint32 DwarfMotorController::moduleId() const { return 6u; }

quint32 DwarfMotorController::cmdRun() const { return 14000u; }

quint32 DwarfMotorController::cmdStop() const { return 14002u; }

quint32 DwarfMotorController::cmdJoystickStart() const { return 14006u; }

quint32 DwarfMotorController::cmdJoystickFixedAngle() const { return 14007u; }

quint32 DwarfMotorController::cmdJoystickStop() const { return 14008u; }

void DwarfMotorController::startJoystick(double angle, double length,
                                         double speed) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Motor client not connected");
    return;
  }

  dwarf::ReqMotorServiceJoystick req;
  req.set_vector_angle(angle);
  req.set_vector_length(length);
  req.set_speed(speed);

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleId(), cmdJoystickStart(), data);
}

void DwarfMotorController::startJoystickFixedAngle(double angle, double length,
                                                   double speed) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Motor client not connected");
    return;
  }

  dwarf::ReqMotorServiceJoystickFixedAngle req;
  req.set_vector_angle(angle);
  req.set_vector_length(length);
  req.set_speed(speed);

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleId(), cmdJoystickFixedAngle(), data);
}

void DwarfMotorController::stopJoystick() {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Motor client not connected");
    return;
  }

  dwarf::ReqMotorServiceJoystickStop req;
  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleId(), cmdJoystickStop(), data);
}
