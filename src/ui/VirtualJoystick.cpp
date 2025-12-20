#include "VirtualJoystick.h"
#include <QPainter>
#include <QMouseEvent>
#include <QtMath>
#include <QDebug>

VirtualJoystick::VirtualJoystick(QWidget *parent) : QWidget(parent) {
    setMinimumSize(150, 150);
    // Transparent background
    setAttribute(Qt::WA_TranslucentBackground);
}

void VirtualJoystick::setSpeed(double speed) {
    // Can be used to scale output if needed
}

void VirtualJoystick::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    m_center = QPointF(width() / 2.0, height() / 2.0);
    if (!m_dragging) {
        m_handlePos = m_center;
    }

    // Draw Base
    QColor ringColor(39, 174, 96, 80);
    QColor baseColor(0, 20, 10, 180);
    
    // Outer background
    p.setPen(Qt::NoPen);
    p.setBrush(baseColor);
    p.drawEllipse(m_center, m_baseRadius, m_baseRadius);

    // Concentric rings (Tech look)
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(ringColor, 1));
    p.drawEllipse(m_center, m_baseRadius, m_baseRadius);
    p.drawEllipse(m_center, m_baseRadius * 0.7, m_baseRadius * 0.7);
    p.drawEllipse(m_center, m_baseRadius * 0.4, m_baseRadius * 0.4);

    // Draw Handle
    QRadialGradient gradient(m_handlePos, m_handleRadius);
    gradient.setColorAt(0, QColor(39, 174, 96, 200));
    gradient.setColorAt(1, QColor(39, 174, 96, 0)); 

    p.setPen(Qt::NoPen);
    p.setBrush(gradient);
    p.drawEllipse(m_handlePos, m_handleRadius, m_handleRadius);
    
    // Draw handle border
    p.setPen(QPen(QColor(39, 174, 96, 255), 2));
    p.setBrush(Qt::NoBrush);
    // Make the physical handle ring smaller than the glow
    p.drawEllipse(m_handlePos, m_handleRadius * 0.6, m_handleRadius * 0.6);
}

void VirtualJoystick::mousePressEvent(QMouseEvent *event) {
    QPointF diff = event->position() - m_center;
    double dist = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    
    if (dist <= m_baseRadius) {
        m_dragging = true;
        updateHandle(event->position());
    }
}

void VirtualJoystick::mouseMoveEvent(QMouseEvent *event) {
    if (m_dragging) {
        updateHandle(event->position());
    }
}

void VirtualJoystick::mouseReleaseEvent(QMouseEvent *) {
    m_dragging = false;
    m_handlePos = m_center;
    update();
    emit joystickReleased();
}

void VirtualJoystick::resizeEvent(QResizeEvent *) {
    m_center = QPointF(width() / 2.0, height() / 2.0);
    if (!m_dragging) {
        m_handlePos = m_center;
    }
    update();
}

void VirtualJoystick::updateHandle(const QPointF &pos) {
    QPointF diff = pos - m_center;
    double dist = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    double angleRad = std::atan2(diff.y(), diff.x()); // Y is down, so +Y is down
    
    // Clamp to max distance
    if (dist > m_maxDistance) {
        diff.setX(std::cos(angleRad) * m_maxDistance);
        diff.setY(std::sin(angleRad) * m_maxDistance);
        dist = m_maxDistance;
    }
    
    m_handlePos = m_center + diff;
    update();
    
    // Calculate angle in degrees (0-360)
    // Dwarf API typically expects: 0=Right, 90=Up, 180=Left, 270=Down?
    // Or standard math: 0=Right, 90=Down (screen coordinates).
    // Let's standard math degrees: atan2 returns radians.
    // Degrees = rad * 180 / PI.
    double degrees = qRadiansToDegrees(angleRad);
    if (degrees < 0) degrees += 360.0;
    
    // Dwarf Joystick likely expects angle 0-360 starting from somewhere.
    // Usually standard math angle: 0 is East (Right), 90 is South (Down), etc.
    // But verify: Dwarf App usually has Up=90?
    // Let's invert Y for "Up" calculation if needed.
    // Screen coords: Right (+x), Down (+y).
    // So 0 deg = Right. 90 deg = Down. 270 deg = Up.
    
    // Dwarf API "angle" parameter: usually 0-360.
    // Let's emit standard screen angle for now, controller can adapt if needed.
    
    double strength = dist / m_maxDistance; // 0.0 to 1.0
    
    emit joystickMoved(degrees, strength);
}
