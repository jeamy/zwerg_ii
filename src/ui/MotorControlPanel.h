#pragma once

#include <QWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QElapsedTimer>

class DwarfMotorController;
class VirtualJoystick;
class QTimer;

class MotorControlPanel : public QWidget {
    Q_OBJECT

public:
    explicit MotorControlPanel(QWidget *parent = nullptr);

    void setMotorController(DwarfMotorController *controller);

signals:
    void speedChanged(int speedIndex);

private:
    void setupUi();

    DwarfMotorController *m_controller = nullptr;

    // UI Elements
    QPushButton *m_upButton = nullptr;
    QPushButton *m_downButton = nullptr;
    QPushButton *m_leftButton = nullptr;
    QPushButton *m_rightButton = nullptr;
    QSlider *m_speedSlider = nullptr;
    QLabel *m_speedLabel = nullptr;

    VirtualJoystick *m_joystick = nullptr;
    QTimer *m_joystickSendTimer = nullptr;
    double m_pendingJoystickAngle = 0.0;
    double m_pendingJoystickStrength = 0.0;
    bool m_hasPendingJoystick = false;
    QElapsedTimer m_lastJoystickSend;

    // Speed table: 0.1, 1.0, 5.0, 10.0, 30.0
    static const double s_speedTable[5];

private slots:
    void onUpPressed();
    void onUpReleased();
    void onDownPressed();
    void onDownReleased();
    void onLeftPressed();
    void onLeftReleased();
    void onRightPressed();
    void onRightReleased();
    void onSpeedChanged(int value);

    void onJoystickMoved(double angle, double strength);
    void onJoystickReleased();
    void sendPendingJoystick();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPoint m_dragOffset;
};
