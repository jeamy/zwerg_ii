#pragma once

#include <QWidget>
#include <QPointF>

class VirtualJoystick : public QWidget {
    Q_OBJECT

public:
    explicit VirtualJoystick(QWidget *parent = nullptr);

    void setSpeed(double speed); // Max speed multiplier

signals:
    // angle in degrees (0-360), strength (0.0-1.0)
    void joystickMoved(double angle, double strength);
    void joystickReleased();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPointF m_center;
    QPointF m_handlePos;
    bool m_dragging = false;
    double m_baseRadius = 60.0;
    double m_handleRadius = 25.0;
    double m_maxDistance = 60.0;
    
    void updateHandle(const QPointF &pos);
};
