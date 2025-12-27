#include "ImageSettingsWidget.h"
#include "../net/DwarfCameraController.h"

#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

// ============================================================================
// ImageSettingsWidget
// ============================================================================

ImageSettingsWidget::ImageSettingsWidget(QWidget *parent)
    : QWidget(parent)
    , m_controller(nullptr)
{
    setupUi();
}

void ImageSettingsWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    auto *group = new QGroupBox(tr("Image Settings"), this);
    auto *gridLayout = new QGridLayout(group);
    
    // Brightness
    gridLayout->addWidget(new QLabel(tr("Brightness:"), group), 0, 0);
    m_brightnessSlider = new QSlider(Qt::Horizontal, group);
    m_brightnessSlider->setRange(-100, 100);
    m_brightnessSlider->setValue(0);
    m_brightnessValueLabel = new QLabel("0", group);
    m_brightnessValueLabel->setMinimumWidth(30);
    m_brightnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_brightnessSlider, 0, 1);
    gridLayout->addWidget(m_brightnessValueLabel, 0, 2);
    
    // Contrast
    gridLayout->addWidget(new QLabel(tr("Contrast:"), group), 1, 0);
    m_contrastSlider = new QSlider(Qt::Horizontal, group);
    m_contrastSlider->setRange(-100, 100);
    m_contrastSlider->setValue(0);
    m_contrastValueLabel = new QLabel("0", group);
    m_contrastValueLabel->setMinimumWidth(30);
    m_contrastValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_contrastSlider, 1, 1);
    gridLayout->addWidget(m_contrastValueLabel, 1, 2);
    
    // Saturation
    gridLayout->addWidget(new QLabel(tr("Saturation:"), group), 2, 0);
    m_saturationSlider = new QSlider(Qt::Horizontal, group);
    m_saturationSlider->setRange(-100, 100);
    m_saturationSlider->setValue(0);
    m_saturationValueLabel = new QLabel("0", group);
    m_saturationValueLabel->setMinimumWidth(30);
    m_saturationValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_saturationSlider, 2, 1);
    gridLayout->addWidget(m_saturationValueLabel, 2, 2);
    
    // Sharpness
    gridLayout->addWidget(new QLabel(tr("Sharpness:"), group), 3, 0);
    m_sharpnessSlider = new QSlider(Qt::Horizontal, group);
    m_sharpnessSlider->setRange(0, 100);
    m_sharpnessSlider->setValue(50);
    m_sharpnessValueLabel = new QLabel("50", group);
    m_sharpnessValueLabel->setMinimumWidth(30);
    m_sharpnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_sharpnessSlider, 3, 1);
    gridLayout->addWidget(m_sharpnessValueLabel, 3, 2);
    
    // Hue
    gridLayout->addWidget(new QLabel(tr("Hue:"), group), 4, 0);
    m_hueSlider = new QSlider(Qt::Horizontal, group);
    m_hueSlider->setRange(-180, 180);
    m_hueSlider->setValue(0);
    m_hueValueLabel = new QLabel("0", group);
    m_hueValueLabel->setMinimumWidth(30);
    m_hueValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_hueSlider, 4, 1);
    gridLayout->addWidget(m_hueValueLabel, 4, 2);
    
    layout->addWidget(group);
    
    // Connect signals
    connect(m_brightnessSlider, &QSlider::valueChanged, this, &ImageSettingsWidget::onBrightnessChanged);
    connect(m_contrastSlider, &QSlider::valueChanged, this, &ImageSettingsWidget::onContrastChanged);
    connect(m_saturationSlider, &QSlider::valueChanged, this, &ImageSettingsWidget::onSaturationChanged);
    connect(m_sharpnessSlider, &QSlider::valueChanged, this, &ImageSettingsWidget::onSharpnessChanged);
    connect(m_hueSlider, &QSlider::valueChanged, this, &ImageSettingsWidget::onHueChanged);
}

void ImageSettingsWidget::setCameraController(DwarfCameraController *controller) {
    m_controller = controller;
}

void ImageSettingsWidget::setBrightness(int value) {
    m_brightnessSlider->blockSignals(true);
    m_brightnessSlider->setValue(value);
    m_brightnessSlider->blockSignals(false);
    m_brightnessValueLabel->setText(QString::number(value));
}

void ImageSettingsWidget::setContrast(int value) {
    m_contrastSlider->blockSignals(true);
    m_contrastSlider->setValue(value);
    m_contrastSlider->blockSignals(false);
    m_contrastValueLabel->setText(QString::number(value));
}

void ImageSettingsWidget::setSaturation(int value) {
    m_saturationSlider->blockSignals(true);
    m_saturationSlider->setValue(value);
    m_saturationSlider->blockSignals(false);
    m_saturationValueLabel->setText(QString::number(value));
}

void ImageSettingsWidget::setSharpness(int value) {
    m_sharpnessSlider->blockSignals(true);
    m_sharpnessSlider->setValue(value);
    m_sharpnessSlider->blockSignals(false);
    m_sharpnessValueLabel->setText(QString::number(value));
}

void ImageSettingsWidget::setHue(int value) {
    m_hueSlider->blockSignals(true);
    m_hueSlider->setValue(value);
    m_hueSlider->blockSignals(false);
    m_hueValueLabel->setText(QString::number(value));
}

int ImageSettingsWidget::brightness() const { return m_brightnessSlider->value(); }
int ImageSettingsWidget::contrast() const { return m_contrastSlider->value(); }
int ImageSettingsWidget::saturation() const { return m_saturationSlider->value(); }
int ImageSettingsWidget::sharpness() const { return m_sharpnessSlider->value(); }
int ImageSettingsWidget::hue() const { return m_hueSlider->value(); }

void ImageSettingsWidget::onBrightnessChanged(int value) {
    m_brightnessValueLabel->setText(QString::number(value));
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setBrightness(kind, value);
    }
}

void ImageSettingsWidget::onContrastChanged(int value) {
    m_contrastValueLabel->setText(QString::number(value));
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setContrast(kind, value);
    }
}

void ImageSettingsWidget::onSaturationChanged(int value) {
    m_saturationValueLabel->setText(QString::number(value));
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setSaturation(kind, value);
    }
}

void ImageSettingsWidget::onSharpnessChanged(int value) {
    m_sharpnessValueLabel->setText(QString::number(value));
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setSharpness(kind, value);
    }
}

void ImageSettingsWidget::onHueChanged(int value) {
    m_hueValueLabel->setText(QString::number(value));
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setHue(kind, value);
    }
}

// ============================================================================
// WhiteBalanceWidget
// ============================================================================

const QVector<int> WhiteBalanceWidget::s_wbTemperatureValues = {
    2800, 3000, 3200, 3400, 3600, 3800, 4000, 4200, 4400, 4600,
    4800, 5000, 5200, 5400, 5600, 5800, 6000, 6200, 6400, 6600,
    6800, 7000, 7200, 7400, 7600, 7800, 8000
};

WhiteBalanceWidget::WhiteBalanceWidget(QWidget *parent)
    : QWidget(parent)
    , m_controller(nullptr)
{
    setupUi();
}

void WhiteBalanceWidget::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    
    auto *group = new QGroupBox(tr("White Balance"), this);
    auto *gridLayout = new QGridLayout(group);
    
    // WB Mode
    gridLayout->addWidget(new QLabel(tr("Mode:"), group), 0, 0);
    m_wbModeCombo = new QComboBox(group);
    m_wbModeCombo->addItem(tr("Auto"));
    m_wbModeCombo->addItem(tr("Manual"));
    gridLayout->addWidget(m_wbModeCombo, 0, 1, 1, 2);
    
    // Color Temperature
    gridLayout->addWidget(new QLabel(tr("Temperature:"), group), 1, 0);
    m_wbTemperatureSlider = new QSlider(Qt::Horizontal, group);
    m_wbTemperatureSlider->setRange(0, s_wbTemperatureValues.size() - 1);
    m_wbTemperatureSlider->setValue(13); // ~5600K (daylight)
    m_wbTemperatureSlider->setEnabled(false); // Disabled in Auto mode
    m_wbTemperatureValueLabel = new QLabel("5600K", group);
    m_wbTemperatureValueLabel->setMinimumWidth(50);
    m_wbTemperatureValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    gridLayout->addWidget(m_wbTemperatureSlider, 1, 1);
    gridLayout->addWidget(m_wbTemperatureValueLabel, 1, 2);
    
    layout->addWidget(group);
    
    // Connect signals
    connect(m_wbModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WhiteBalanceWidget::onWbModeChanged);
    connect(m_wbTemperatureSlider, &QSlider::valueChanged,
            this, &WhiteBalanceWidget::onWbTemperatureChanged);
}

void WhiteBalanceWidget::setCameraController(DwarfCameraController *controller) {
    m_controller = controller;
}

void WhiteBalanceWidget::setWhiteBalanceMode(int mode) {
    m_wbModeCombo->blockSignals(true);
    m_wbModeCombo->setCurrentIndex(mode);
    m_wbModeCombo->blockSignals(false);
    m_wbTemperatureSlider->setEnabled(mode == 1); // Manual
}

void WhiteBalanceWidget::setWhiteBalanceTemperature(int index) {
    if (index < 0 || index >= s_wbTemperatureValues.size()) return;
    m_wbTemperatureSlider->blockSignals(true);
    m_wbTemperatureSlider->setValue(index);
    m_wbTemperatureSlider->blockSignals(false);
    updateTemperatureLabel();
}

int WhiteBalanceWidget::whiteBalanceMode() const {
    return m_wbModeCombo->currentIndex();
}

int WhiteBalanceWidget::whiteBalanceTemperatureIndex() const {
    return m_wbTemperatureSlider->value();
}

void WhiteBalanceWidget::updateTemperatureLabel() {
    int index = m_wbTemperatureSlider->value();
    if (index >= 0 && index < s_wbTemperatureValues.size()) {
        m_wbTemperatureValueLabel->setText(QString("%1K").arg(s_wbTemperatureValues[index]));
    }
}

void WhiteBalanceWidget::onWbModeChanged(int index) {
    m_wbTemperatureSlider->setEnabled(index == 1); // Manual mode
    if (m_controller) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setWhiteBalanceMode(kind, index);
    }
}

void WhiteBalanceWidget::onWbTemperatureChanged(int value) {
    updateTemperatureLabel();
    if (m_controller && value >= 0 && value < s_wbTemperatureValues.size()) {
        auto kind = (m_cameraKind == CameraKind::Tele) 
            ? DwarfCameraController::CameraKind::Tele 
            : DwarfCameraController::CameraKind::Wide;
        m_controller->setWhiteBalanceByTemperature(kind, value);
    }
}
