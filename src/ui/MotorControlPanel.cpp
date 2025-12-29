#include "MotorControlPanel.h"
#include "../net/DwarfMotorController.h"
#include "../net/DwarfFocusController.h"

#include "VirtualJoystick.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGraphicsDropShadowEffect>
#include <QLabel>
#include <QCursor>
#include <QMouseEvent>
#include <QStackedLayout>
#include <QStyle>
#include <QTimer>
#include <QStyleOptionGroupBox>
#include <algorithm>

const double MotorControlPanel::s_speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};

namespace {
inline double speedIndexToLengthScale(int idx) {
    // The DWARF joystick API tends to feel like speed is dominated by vector_length.
    // Use a perceptible mapping so the slider has a clear effect.
    static const double kScale[5] = {0.02, 0.05, 0.15, 0.35, 1.00};
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    return kScale[idx];
}

inline double speedIndexToJoystickSpeedParam(int idx) {
    // The joystick protobuf "speed" is likely a normalized factor (0..1).
    // We expose user-facing deg/s, but normalize here to avoid everything >= 1 behaving as max.
    if (idx < 0) idx = 0;
    if (idx > 4) idx = 4;
    static const double kSpeedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
    const double maxDegPerSec = kSpeedTable[4];
    const double selected = kSpeedTable[idx];
    return std::min(1.0, std::max(0.0, selected / maxDegPerSec));
}
}

MotorControlPanel::MotorControlPanel(QWidget *parent) : QWidget(parent) {
    setupUi();
}

void MotorControlPanel::setMotorController(DwarfMotorController *controller) {
    m_controller = controller;
}

void MotorControlPanel::setFocusController(DwarfFocusController *controller) {
    m_focusController = controller;
}

void MotorControlPanel::setClientMode(bool enabled) {
    bool controlsEnabled = !enabled;
    if (m_upButton) m_upButton->setEnabled(controlsEnabled);
    if (m_downButton) m_downButton->setEnabled(controlsEnabled);
    if (m_leftButton) m_leftButton->setEnabled(controlsEnabled);
    if (m_rightButton) m_rightButton->setEnabled(controlsEnabled);
    if (m_joystick) m_joystick->setEnabled(controlsEnabled);
    if (m_focusFarButton) m_focusFarButton->setEnabled(controlsEnabled);
    if (m_focusNearButton) m_focusNearButton->setEnabled(controlsEnabled);
    if (m_autoFocusButton) m_autoFocusButton->setEnabled(controlsEnabled);
}

void MotorControlPanel::setupUi() {
    setObjectName("motorControlPanel");
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(8);

    // Motor Control Group
    m_motorGroup = new QGroupBox(tr("Motor Control"), this);
    m_motorGroup->setObjectName("motorControlGroup");
    QVBoxLayout *motorLayout = new QVBoxLayout(m_motorGroup);
    motorLayout->setContentsMargins(10, 0, 10, 10);
    motorLayout->setSpacing(8);

    auto *groupGlow = new QGraphicsDropShadowEffect(m_motorGroup);
    groupGlow->setBlurRadius(32);
    groupGlow->setOffset(0, 0);
    groupGlow->setColor(QColor(39, 174, 96, 140));
    m_motorGroup->setGraphicsEffect(groupGlow);

    QWidget *slewArea = new QWidget(m_motorGroup);
    slewArea->setObjectName("motorSlewArea");
    slewArea->setFixedSize(190, 190);
    QGridLayout *slewLayout = new QGridLayout(slewArea);
    slewLayout->setContentsMargins(0, 0, 0, 0);
    slewLayout->setSpacing(0);

    slewLayout->setRowMinimumHeight(0, 40);
    slewLayout->setRowMinimumHeight(1, 40);
    slewLayout->setRowMinimumHeight(2, 40);
    slewLayout->setColumnMinimumWidth(0, 40);
    slewLayout->setColumnMinimumWidth(1, 40);
    slewLayout->setColumnMinimumWidth(2, 40);

    auto makeArrowButton = [&](const QString &text, const char *objectName) {
        QPushButton *btn = new QPushButton(slewArea);
        btn->setObjectName(objectName);
        btn->setText(text);
        btn->setFlat(true);
        btn->setFocusPolicy(Qt::NoFocus);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setFixedSize(34, 34);
        auto *glow = new QGraphicsDropShadowEffect(btn);
        glow->setBlurRadius(40);
        glow->setOffset(0, 0);
        glow->setColor(QColor(39, 174, 96, 190));
        btn->setGraphicsEffect(glow);
        return btn;
    };

    m_upButton = makeArrowButton(QString::fromUtf8("▲"), "motorArrowUpButton");
    m_downButton = makeArrowButton(QString::fromUtf8("▼"), "motorArrowDownButton");
    m_leftButton = makeArrowButton(QString::fromUtf8("◀"), "motorArrowLeftButton");
    m_rightButton = makeArrowButton(QString::fromUtf8("▶"), "motorArrowRightButton");

    QWidget *center = new QWidget(slewArea);
    center->setObjectName("motorSlewCenter");
    QStackedLayout *centerStack = new QStackedLayout(center);
    centerStack->setStackingMode(QStackedLayout::StackAll);
    centerStack->setContentsMargins(0, 0, 0, 0);

    m_joystick = new VirtualJoystick(center);
    m_joystick->setObjectName("motorJoystick");
    m_joystick->setFixedSize(110, 110);

    QLabel *slewText = new QLabel(tr("SLEW"), center);
    slewText->setObjectName("motorSlewText");
    slewText->setAlignment(Qt::AlignCenter);
    slewText->setAttribute(Qt::WA_TransparentForMouseEvents);

    auto *slewGlow = new QGraphicsDropShadowEffect(slewText);
    slewGlow->setBlurRadius(30);
    slewGlow->setOffset(0, 0);
    slewGlow->setColor(QColor(39, 174, 96, 160));
    slewText->setGraphicsEffect(slewGlow);

    centerStack->addWidget(m_joystick);
    centerStack->addWidget(slewText);
    centerStack->setAlignment(slewText, Qt::AlignCenter);

    slewLayout->setRowStretch(0, 0);
    slewLayout->setRowStretch(1, 1);
    slewLayout->setRowStretch(2, 0);
    slewLayout->setColumnStretch(0, 0);
    slewLayout->setColumnStretch(1, 1);
    slewLayout->setColumnStretch(2, 0);

    slewLayout->addWidget(m_upButton, 0, 1, Qt::AlignCenter);
    slewLayout->addWidget(m_leftButton, 1, 0, Qt::AlignCenter);
    slewLayout->addWidget(center, 1, 1);
    slewLayout->addWidget(m_rightButton, 1, 2, Qt::AlignCenter);
    slewLayout->addWidget(m_downButton, 2, 1, Qt::AlignCenter);

    motorLayout->addWidget(slewArea, 0, Qt::AlignHCenter | Qt::AlignTop);

    // Speed Slider
    QHBoxLayout *speedLayout = new QHBoxLayout();
    QLabel *speedTextLabel = new QLabel(tr("Speed:"), this);
    speedTextLabel->setObjectName("motorSpeedTextLabel");
    m_speedSlider = new QSlider(Qt::Horizontal, this);
    m_speedSlider->setRange(0, 4); // 5 levels
    m_speedSlider->setValue(1); // Default: 1.0 deg/s
    m_speedLabel = new QLabel("1.0 deg/s", this);
    m_speedLabel->setObjectName("motorSpeedValueLabel");
    m_speedLabel->setMinimumWidth(80);
    m_speedLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    speedLayout->addWidget(speedTextLabel);
    speedLayout->addWidget(m_speedSlider, 1);
    speedLayout->addWidget(m_speedLabel);

    motorLayout->addLayout(speedLayout);

    mainLayout->addWidget(m_motorGroup);

    // Focus controls (docked below Motor Control)
    m_focusGroup = new QGroupBox(tr("Focus"), this);
    m_focusGroup->setObjectName("motorFocusGroup");

    auto *focusGlow = new QGraphicsDropShadowEffect(m_focusGroup);
    focusGlow->setBlurRadius(32);
    focusGlow->setOffset(0, 0);
    focusGlow->setColor(QColor(39, 174, 96, 140));
    m_focusGroup->setGraphicsEffect(focusGlow);

    auto *focusLayout = new QHBoxLayout(m_focusGroup);
    focusLayout->setContentsMargins(10, 8, 10, 10);
    focusLayout->setSpacing(8);

    m_focusFarButton = new QPushButton(tr("Far -"), m_focusGroup);
    m_focusFarButton->setObjectName("motorFocusFarButton");
    m_focusFarButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_focusFarButton->setFixedHeight(32);

    m_autoFocusButton = new QPushButton(tr("AUTO"), m_focusGroup);
    m_autoFocusButton->setObjectName("motorAutoFocusButton");
    m_autoFocusButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_autoFocusButton->setFixedHeight(32);

    m_focusNearButton = new QPushButton(tr("Near +"), m_focusGroup);
    m_focusNearButton->setObjectName("motorFocusNearButton");
    m_focusNearButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_focusNearButton->setFixedHeight(32);

    focusLayout->addWidget(m_focusFarButton);
    focusLayout->addWidget(m_autoFocusButton);
    focusLayout->addWidget(m_focusNearButton);

    mainLayout->addWidget(m_focusGroup);
    mainLayout->addStretch(1);

    m_joystickSendTimer = new QTimer(this);
    m_joystickSendTimer->setInterval(50);
    connect(m_joystickSendTimer, &QTimer::timeout, this, &MotorControlPanel::sendPendingJoystick);

    // Connect signals
    connect(m_speedSlider, &QSlider::valueChanged, this, &MotorControlPanel::onSpeedChanged);
    connect(m_joystick, &VirtualJoystick::joystickMoved, this, &MotorControlPanel::onJoystickMoved);
    connect(m_joystick, &VirtualJoystick::joystickReleased, this, &MotorControlPanel::onJoystickReleased);

    connect(m_upButton, &QPushButton::pressed, this, &MotorControlPanel::onUpPressed);
    connect(m_upButton, &QPushButton::released, this, &MotorControlPanel::onUpReleased);
    connect(m_downButton, &QPushButton::pressed, this, &MotorControlPanel::onDownPressed);
    connect(m_downButton, &QPushButton::released, this, &MotorControlPanel::onDownReleased);
    connect(m_leftButton, &QPushButton::pressed, this, &MotorControlPanel::onLeftPressed);
    connect(m_leftButton, &QPushButton::released, this, &MotorControlPanel::onLeftReleased);
    connect(m_rightButton, &QPushButton::pressed, this, &MotorControlPanel::onRightPressed);
    connect(m_rightButton, &QPushButton::released, this, &MotorControlPanel::onRightReleased);

    connect(m_focusFarButton, &QPushButton::clicked, this, &MotorControlPanel::onFocusFarClicked);
    connect(m_focusNearButton, &QPushButton::clicked, this, &MotorControlPanel::onFocusNearClicked);
    connect(m_autoFocusButton, &QPushButton::clicked, this, &MotorControlPanel::onAutoFocusClicked);

    m_lastJoystickSend.invalidate();
}

void MotorControlPanel::onFocusFarClicked() {
    if (m_focusController)
        m_focusController->manualStepFar();
}

void MotorControlPanel::onFocusNearClicked() {
    if (m_focusController)
        m_focusController->manualStepNear();
}

void MotorControlPanel::onAutoFocusClicked() {
    if (m_focusController)
        m_focusController->autoFocusNormal();
}

void MotorControlPanel::onUpPressed() {
    if (!m_controller) return;
    m_pendingJoystickAngle = 270.0;
    m_pendingJoystickStrength = 1.0;
    m_hasPendingJoystick = true;
    if (m_joystickSendTimer && !m_joystickSendTimer->isActive())
        m_joystickSendTimer->start();
}

void MotorControlPanel::onUpReleased() {
    if (!m_controller) return;
    m_hasPendingJoystick = false;
    if (m_joystickSendTimer)
        m_joystickSendTimer->stop();
    m_controller->stopJoystick();
}

void MotorControlPanel::onDownPressed() {
    if (!m_controller) return;
    m_pendingJoystickAngle = 90.0;
    m_pendingJoystickStrength = 1.0;
    m_hasPendingJoystick = true;
    if (m_joystickSendTimer && !m_joystickSendTimer->isActive())
        m_joystickSendTimer->start();
}

void MotorControlPanel::onDownReleased() {
    if (!m_controller) return;
    m_hasPendingJoystick = false;
    if (m_joystickSendTimer)
        m_joystickSendTimer->stop();
    m_controller->stopJoystick();
}

void MotorControlPanel::onLeftPressed() {
    if (!m_controller) return;
    m_pendingJoystickAngle = 180.0;
    m_pendingJoystickStrength = 1.0;
    m_hasPendingJoystick = true;
    if (m_joystickSendTimer && !m_joystickSendTimer->isActive())
        m_joystickSendTimer->start();
}

void MotorControlPanel::onLeftReleased() {
    if (!m_controller) return;
    m_hasPendingJoystick = false;
    if (m_joystickSendTimer)
        m_joystickSendTimer->stop();
    m_controller->stopJoystick();
}

void MotorControlPanel::onRightPressed() {
    if (!m_controller) return;
    m_pendingJoystickAngle = 0.0;
    m_pendingJoystickStrength = 1.0;
    m_hasPendingJoystick = true;
    if (m_joystickSendTimer && !m_joystickSendTimer->isActive())
        m_joystickSendTimer->start();
}

void MotorControlPanel::onRightReleased() {
    if (!m_controller) return;
    m_hasPendingJoystick = false;
    if (m_joystickSendTimer)
        m_joystickSendTimer->stop();
    m_controller->stopJoystick();
}

void MotorControlPanel::onSpeedChanged(int value) {
    if (value >= 0 && value < 5) {
        double speed = s_speedTable[value];
        m_speedLabel->setText(QString("%1 deg/s").arg(speed));
        emit speedChanged(value);
    }
}

void MotorControlPanel::onJoystickMoved(double angle, double strength) {
    m_pendingJoystickAngle = angle;
    m_pendingJoystickStrength = strength;
    m_hasPendingJoystick = true;

    if (m_joystickSendTimer && !m_joystickSendTimer->isActive()) {
        m_joystickSendTimer->start();
    }
}

void MotorControlPanel::onJoystickReleased() {
    m_hasPendingJoystick = false;
    if (m_joystickSendTimer) {
        m_joystickSendTimer->stop();
    }

    if (m_controller) {
        m_controller->stopJoystick();
    }
}

void MotorControlPanel::sendPendingJoystick() {
    if (!m_controller || !m_hasPendingJoystick || !m_speedSlider)
        return;

    const int idx = m_speedSlider->value();
    const double scale = speedIndexToLengthScale(idx);
    const double length = std::min(1.0, std::max(0.0, m_pendingJoystickStrength * scale));
    const double speedParam = speedIndexToJoystickSpeedParam(idx);
    m_controller->startJoystick(m_pendingJoystickAngle, length, speedParam);
}

void MotorControlPanel::mousePressEvent(QMouseEvent *event) {
    if (m_joystick) {
        const QPoint localToJoystick = m_joystick->mapFrom(this, event->pos());
        if (m_joystick->rect().contains(localToJoystick)) {
            QWidget::mousePressEvent(event);
            return;
        }
    }

    const QPoint localPos = event->pos();
    const QWidget *child = childAt(localPos);
    const QGroupBox *gb = nullptr;
    for (const QWidget *w = child; w; w = w->parentWidget()) {
        gb = qobject_cast<const QGroupBox *>(w);
        if (gb)
            break;
        if (w == this)
            break;
    }

    bool allowDrag = false;
    if (gb) {
        QStyleOptionGroupBox opt;
        opt.initFrom(gb);
        opt.text = gb->title();
        opt.subControls = QStyle::SC_GroupBoxLabel;
        const QRect labelRect =
            style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, gb);
        const QPoint gbPos = gb->mapFrom(this, localPos);
        allowDrag = labelRect.adjusted(-6, -6, 6, 6).contains(gbPos);
    }

    if (allowDrag) {
        m_dragOffset = QCursor::pos() - pos();
    }
    QWidget::mousePressEvent(event);
}

void MotorControlPanel::mouseMoveEvent(QMouseEvent *event) {
    if (m_joystick) {
        const QPoint localToJoystick = m_joystick->mapFrom(this, event->pos());
        if (m_joystick->rect().contains(localToJoystick)) {
            QWidget::mouseMoveEvent(event);
            return;
        }
    }

    if ((event->buttons() & Qt::LeftButton) && !m_dragOffset.isNull()) {
        move(QCursor::pos() - m_dragOffset);
    }
    QWidget::mouseMoveEvent(event);
}

void MotorControlPanel::mouseReleaseEvent(QMouseEvent *event) {
    m_dragOffset = QPoint();
    QWidget::mouseReleaseEvent(event);
}
