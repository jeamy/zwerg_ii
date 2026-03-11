#pragma once

#include "DwarfWebSocketClient.h"
#include "motor.pb.h"

#include <QObject>

class DwarfMotorController : public QObject {
  Q_OBJECT

public:
  enum class Axis { Azimuth = 1, Altitude = 2 };

  explicit DwarfMotorController(QObject *parent = nullptr);

  void setClient(DwarfWebSocketClient *client);

  void runMotor(Axis axis, bool directionRightOrUp, double speed,
                int speedRamping = 100, int resolutionLevel = 0);
  void stopMotor(Axis axis);

  // Joystick Control
  void startJoystick(double angle, double length, double speed);
  void startJoystickFixedAngle(double angle, double length, double speed);
  void stopJoystick();
  void dualCameraLinkage(int x, int y);

signals:
  void errorOccurred(const QString &message);
  void statusMessage(const QString &message);

private:
  DwarfWebSocketClient *m_client;

  quint32 moduleId() const;
  quint32 cmdRun() const;
  quint32 cmdStop() const;

  quint32 cmdJoystickStart() const;
  quint32 cmdJoystickFixedAngle() const;
  quint32 cmdJoystickStop() const;
  quint32 cmdDualCameraLinkage() const;
};
