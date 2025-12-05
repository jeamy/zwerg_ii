#include "CameraSettingsPanel.h"
#include "../net/DwarfCameraController.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTimer>

// Exposure times in microseconds for Tele camera
// Based on DWARF II protocol: indices 0-27
const QVector<int> CameraSettingsPanel::s_teleExposureValues = {
    100,     200,     500,     1000,    2000,    5000,    10000,
    20000,   50000,   100000,  200000,  500000,  1000000, 2000000,
    3000000, 4000000, 5000000, 6000000, 7000000, 8000000, 9000000,
    10000000, 12000000, 15000000, 20000000, 25000000, 30000000, 60000000};

// Exposure times for Wide camera (different range)
// Based on DWARF II protocol: indices 0-15
const QVector<int> CameraSettingsPanel::s_wideExposureValues = {
    100,   200,   500,   1000,  2000,   5000,   10000,  20000,
    33333, 50000, 66666, 100000, 125000, 166666, 200000, 250000};

// Gain values for Tele camera (0-300 in steps)
const QVector<int> CameraSettingsPanel::s_teleGainValues = {
    0,  10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150,
    160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 260, 270, 280, 290, 300};

// Gain values for Wide camera (0-100)
const QVector<int> CameraSettingsPanel::s_wideGainValues = {
    0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

// White balance color temperatures in Kelvin
const QVector<int> CameraSettingsPanel::s_wbTemperatureValues = {
    2800, 3200, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000};

CameraSettingsPanel::CameraSettingsPanel(QWidget *parent) : QWidget(parent) {
  setupUi();
  updateRangesForMode();
  updateButtonStates();
}

void CameraSettingsPanel::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(8);

  // === Camera Source Selection ===
  QHBoxLayout *sourceLayout = new QHBoxLayout();
  m_teleButton = new QPushButton(tr("TELE"), this);
  m_wideButton = new QPushButton(tr("WIDE"), this);
  m_teleButton->setCheckable(true);
  m_wideButton->setCheckable(true);
  m_teleButton->setChecked(true);
  m_teleButton->setMinimumHeight(36);
  m_wideButton->setMinimumHeight(36);
  sourceLayout->addWidget(m_teleButton);
  sourceLayout->addWidget(m_wideButton);
  mainLayout->addLayout(sourceLayout);

  connect(m_teleButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onTeleClicked);
  connect(m_wideButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onWideClicked);

  // === Capture Buttons ===
  QHBoxLayout *captureLayout = new QHBoxLayout();
  m_photoButton = new QPushButton(tr("📷 PHOTO"), this);
  m_recButton = new QPushButton(tr("⏺ REC"), this);
  m_photoButton->setMinimumHeight(44);
  m_recButton->setMinimumHeight(44);
  m_photoButton->setObjectName("photoButton");
  m_recButton->setObjectName("recButton");
  captureLayout->addWidget(m_photoButton);
  captureLayout->addWidget(m_recButton);
  mainLayout->addLayout(captureLayout);

  connect(m_photoButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onPhotoClicked);
  connect(m_recButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onRecordClicked);

  // === Exposure Group ===
  m_exposureGroup = new QGroupBox(tr("Exposure"), this);
  QGridLayout *exposureLayout = new QGridLayout(m_exposureGroup);
  exposureLayout->setColumnStretch(1, 1);

  // Exposure mode
  QLabel *expModeLabel = new QLabel(tr("Mode:"), m_exposureGroup);
  m_exposureModeCombo = new QComboBox(m_exposureGroup);
  m_exposureModeCombo->addItem(tr("Auto"));
  m_exposureModeCombo->addItem(tr("Manual"));
  exposureLayout->addWidget(expModeLabel, 0, 0);
  exposureLayout->addWidget(m_exposureModeCombo, 0, 1, 1, 2);

  // Shutter/Exposure time
  QLabel *shutterLabel = new QLabel(tr("Shutter:"), m_exposureGroup);
  m_exposureSlider = new QSlider(Qt::Horizontal, m_exposureGroup);
  m_exposureValueLabel = new QLabel("1/1000s", m_exposureGroup);
  m_exposureValueLabel->setMinimumWidth(70);
  m_exposureValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  exposureLayout->addWidget(shutterLabel, 1, 0);
  exposureLayout->addWidget(m_exposureSlider, 1, 1);
  exposureLayout->addWidget(m_exposureValueLabel, 1, 2);

  // Gain mode
  QLabel *gainModeLabel = new QLabel(tr("Gain Mode:"), m_exposureGroup);
  m_gainModeCombo = new QComboBox(m_exposureGroup);
  m_gainModeCombo->addItem(tr("Auto"));
  m_gainModeCombo->addItem(tr("Manual"));
  exposureLayout->addWidget(gainModeLabel, 2, 0);
  exposureLayout->addWidget(m_gainModeCombo, 2, 1, 1, 2);

  // Gain value
  QLabel *gainLabel = new QLabel(tr("Gain:"), m_exposureGroup);
  m_gainSlider = new QSlider(Qt::Horizontal, m_exposureGroup);
  m_gainValueLabel = new QLabel("0", m_exposureGroup);
  m_gainValueLabel->setMinimumWidth(70);
  m_gainValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  exposureLayout->addWidget(gainLabel, 3, 0);
  exposureLayout->addWidget(m_gainSlider, 3, 1);
  exposureLayout->addWidget(m_gainValueLabel, 3, 2);

  mainLayout->addWidget(m_exposureGroup);

  connect(m_exposureModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CameraSettingsPanel::onExposureModeChanged);
  connect(m_exposureSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onExposureSliderChanged);
  connect(m_gainModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CameraSettingsPanel::onGainModeChanged);
  connect(m_gainSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onGainSliderChanged);

  // === Image Parameters Group ===
  m_imageGroup = new QGroupBox(tr("Image"), this);
  QGridLayout *imageLayout = new QGridLayout(m_imageGroup);
  imageLayout->setColumnStretch(1, 1);

  // IR-Cut filter
  m_irCutCheckBox = new QCheckBox(tr("IR-Cut (Day mode)"), m_imageGroup);
  imageLayout->addWidget(m_irCutCheckBox, 0, 0, 1, 3);

  // Binning
  QLabel *binningLabel = new QLabel(tr("Resolution:"), m_imageGroup);
  m_binningCombo = new QComboBox(m_imageGroup);
  m_binningCombo->addItem(tr("4K (No Binning)"));
  m_binningCombo->addItem(tr("2K (2x2 Binning)"));
  imageLayout->addWidget(binningLabel, 1, 0);
  imageLayout->addWidget(m_binningCombo, 1, 1, 1, 2);

  // Brightness
  QLabel *brightnessLabel = new QLabel(tr("Brightness:"), m_imageGroup);
  m_brightnessSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_brightnessSlider->setRange(0, 100);
  m_brightnessSlider->setValue(50);
  m_brightnessValueLabel = new QLabel("50", m_imageGroup);
  m_brightnessValueLabel->setMinimumWidth(40);
  m_brightnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(brightnessLabel, 2, 0);
  imageLayout->addWidget(m_brightnessSlider, 2, 1);
  imageLayout->addWidget(m_brightnessValueLabel, 2, 2);

  // Contrast
  QLabel *contrastLabel = new QLabel(tr("Contrast:"), m_imageGroup);
  m_contrastSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_contrastSlider->setRange(0, 100);
  m_contrastSlider->setValue(50);
  m_contrastValueLabel = new QLabel("50", m_imageGroup);
  m_contrastValueLabel->setMinimumWidth(40);
  m_contrastValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(contrastLabel, 3, 0);
  imageLayout->addWidget(m_contrastSlider, 3, 1);
  imageLayout->addWidget(m_contrastValueLabel, 3, 2);

  // Saturation
  QLabel *saturationLabel = new QLabel(tr("Saturation:"), m_imageGroup);
  m_saturationSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_saturationSlider->setRange(0, 100);
  m_saturationSlider->setValue(50);
  m_saturationValueLabel = new QLabel("50", m_imageGroup);
  m_saturationValueLabel->setMinimumWidth(40);
  m_saturationValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(saturationLabel, 4, 0);
  imageLayout->addWidget(m_saturationSlider, 4, 1);
  imageLayout->addWidget(m_saturationValueLabel, 4, 2);

  // Sharpness
  QLabel *sharpnessLabel = new QLabel(tr("Sharpness:"), m_imageGroup);
  m_sharpnessSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_sharpnessSlider->setRange(0, 100);
  m_sharpnessSlider->setValue(50);
  m_sharpnessValueLabel = new QLabel("50", m_imageGroup);
  m_sharpnessValueLabel->setMinimumWidth(40);
  m_sharpnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(sharpnessLabel, 5, 0);
  imageLayout->addWidget(m_sharpnessSlider, 5, 1);
  imageLayout->addWidget(m_sharpnessValueLabel, 5, 2);

  // Hue
  QLabel *hueLabel = new QLabel(tr("Hue:"), m_imageGroup);
  m_hueSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_hueSlider->setRange(0, 100);
  m_hueSlider->setValue(50);
  m_hueValueLabel = new QLabel("50", m_imageGroup);
  m_hueValueLabel->setMinimumWidth(40);
  m_hueValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(hueLabel, 6, 0);
  imageLayout->addWidget(m_hueSlider, 6, 1);
  imageLayout->addWidget(m_hueValueLabel, 6, 2);

  mainLayout->addWidget(m_imageGroup);

  connect(m_irCutCheckBox, &QCheckBox::toggled, this,
          &CameraSettingsPanel::onIrCutToggled);
  connect(m_binningCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CameraSettingsPanel::onBinningChanged);
  connect(m_brightnessSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onBrightnessChanged);
  connect(m_contrastSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onContrastChanged);
  connect(m_saturationSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onSaturationChanged);
  connect(m_sharpnessSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onSharpnessChanged);
  connect(m_hueSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onHueChanged);

  // === White Balance Group ===
  m_wbGroup = new QGroupBox(tr("White Balance"), this);
  QGridLayout *wbLayout = new QGridLayout(m_wbGroup);
  wbLayout->setColumnStretch(1, 1);

  QLabel *wbModeLabel = new QLabel(tr("Mode:"), m_wbGroup);
  m_wbModeCombo = new QComboBox(m_wbGroup);
  m_wbModeCombo->addItem(tr("Auto"));
  m_wbModeCombo->addItem(tr("Manual"));
  wbLayout->addWidget(wbModeLabel, 0, 0);
  wbLayout->addWidget(m_wbModeCombo, 0, 1, 1, 2);

  QLabel *wbTempLabel = new QLabel(tr("Color Temp:"), m_wbGroup);
  m_wbTemperatureSlider = new QSlider(Qt::Horizontal, m_wbGroup);
  m_wbTemperatureSlider->setRange(0, s_wbTemperatureValues.size() - 1);
  m_wbTemperatureValueLabel = new QLabel("5500K", m_wbGroup);
  m_wbTemperatureValueLabel->setMinimumWidth(50);
  m_wbTemperatureValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  wbLayout->addWidget(wbTempLabel, 1, 0);
  wbLayout->addWidget(m_wbTemperatureSlider, 1, 1);
  wbLayout->addWidget(m_wbTemperatureValueLabel, 1, 2);

  mainLayout->addWidget(m_wbGroup);

  connect(m_wbModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CameraSettingsPanel::onWbModeChanged);
  connect(m_wbTemperatureSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onWbTemperatureChanged);

  // Add stretch at bottom
  mainLayout->addStretch();
}

void CameraSettingsPanel::setCameraController(DwarfCameraController *controller) {
  m_controller = controller;
  // Initialize UI from controller state so Auto modes still show a value
  syncFromController();
}

void CameraSettingsPanel::setCameraMode(CameraMode mode) {
  if (m_cameraMode == mode)
    return;

  m_cameraMode = mode;
  updateRangesForMode();
  updateButtonStates();
  // When switching Tele/Wide, refresh values from controller
  syncFromController();
  emit cameraModeChanged(mode);
}

void CameraSettingsPanel::updateRangesForMode() {
  // Block signals during range update to avoid sending commands
  m_exposureSlider->blockSignals(true);
  m_gainSlider->blockSignals(true);

  if (m_cameraMode == CameraMode::Tele) {
    m_exposureSlider->setRange(0, s_teleExposureValues.size() - 1);
    m_gainSlider->setRange(0, s_teleGainValues.size() - 1);
    // Video recording only available on Tele
    m_recButton->setEnabled(true);
    m_recButton->setToolTip(QString());
  } else {
    m_exposureSlider->setRange(0, s_wideExposureValues.size() - 1);
    m_gainSlider->setRange(0, s_wideGainValues.size() - 1);
    // Video recording not available on Wide
    m_recButton->setEnabled(false);
    m_recButton->setToolTip(tr("Video recording only available on TELE camera"));
  }

  m_exposureSlider->blockSignals(false);
  m_gainSlider->blockSignals(false);

  updateValueLabels();
}

void CameraSettingsPanel::updateButtonStates() {
  m_teleButton->setChecked(m_cameraMode == CameraMode::Tele);
  m_wideButton->setChecked(m_cameraMode == CameraMode::Wide);

  // Visual feedback for active camera
  QString activeStyle = "font-weight: bold; background-color: #3498db; color: white;";
  QString inactiveStyle = "";

  m_teleButton->setStyleSheet(m_cameraMode == CameraMode::Tele ? activeStyle : inactiveStyle);
  m_wideButton->setStyleSheet(m_cameraMode == CameraMode::Wide ? activeStyle : inactiveStyle);
}

void CameraSettingsPanel::syncFromController() {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;

  // Exposure
  int expMode = m_controller->exposureMode(kind);
  setExposureMode(expMode);
  int expIndex = m_controller->exposureIndex(kind);
  setExposureIndex(expIndex);

  // Gain
  int gainMode = m_controller->gainMode(kind);
  setGainMode(gainMode);
  int gainIndex = m_controller->gainIndex(kind);
  setGainIndex(gainIndex);

  // White balance (temperature mode only)
  int wbMode = m_controller->whiteBalanceMode(kind);
  setWhiteBalanceMode(wbMode);
  int wbIndex = m_controller->whiteBalanceTemperatureIndex(kind);
  setWhiteBalanceTemperature(wbIndex);

  // Other sliders (brightness/contrast/etc.) bleiben wie initial gesetzt,
  // da es aktuell keine Getter im Controller gibt.
}

void CameraSettingsPanel::updateValueLabels() {
  m_exposureValueLabel->setText(formatExposureValue(m_exposureSlider->value()));
  m_gainValueLabel->setText(formatGainValue(m_gainSlider->value()));
  m_brightnessValueLabel->setText(QString::number(m_brightnessSlider->value()));
  m_contrastValueLabel->setText(QString::number(m_contrastSlider->value()));
  m_saturationValueLabel->setText(QString::number(m_saturationSlider->value()));
  m_sharpnessValueLabel->setText(QString::number(m_sharpnessSlider->value()));
  m_hueValueLabel->setText(QString::number(m_hueSlider->value()));

  int wbIndex = m_wbTemperatureSlider->value();
  if (wbIndex >= 0 && wbIndex < s_wbTemperatureValues.size()) {
    m_wbTemperatureValueLabel->setText(
        QString("%1K").arg(s_wbTemperatureValues[wbIndex]));
  }
}

QString CameraSettingsPanel::formatExposureValue(int index) const {
  const QVector<int> &values = (m_cameraMode == CameraMode::Tele)
                                   ? s_teleExposureValues
                                   : s_wideExposureValues;

  if (index < 0 || index >= values.size())
    return "---";

  int us = values[index];

  // Format as fraction of a second or seconds
  if (us < 1000) {
    return QString("1/%1s").arg(1000000 / us);
  } else if (us < 1000000) {
    int denom = 1000000 / us;
    if (denom > 1)
      return QString("1/%1s").arg(denom);
    else
      return QString("%1ms").arg(us / 1000);
  } else {
    double sec = us / 1000000.0;
    if (sec == (int)sec)
      return QString("%1s").arg((int)sec);
    else
      return QString("%1s").arg(sec, 0, 'f', 1);
  }
}

QString CameraSettingsPanel::formatGainValue(int index) const {
  const QVector<int> &values = (m_cameraMode == CameraMode::Tele)
                                   ? s_teleGainValues
                                   : s_wideGainValues;

  if (index < 0 || index >= values.size())
    return "---";

  return QString::number(values[index]);
}

// === Slots ===

void CameraSettingsPanel::onTeleClicked() {
  setCameraMode(CameraMode::Tele);
}

void CameraSettingsPanel::onWideClicked() {
  setCameraMode(CameraMode::Wide);
}

void CameraSettingsPanel::onPhotoClicked() {
  if (m_controller) {
    auto kind = (m_cameraMode == CameraMode::Tele)
                    ? DwarfCameraController::CameraKind::Tele
                    : DwarfCameraController::CameraKind::Wide;
    m_controller->takePhoto(kind);
  }
  m_photoButton->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; border-radius: 4px;");
  QTimer::singleShot(500, this,
                     [this]() { m_photoButton->setStyleSheet(""); });
  emit photoRequested();
}

void CameraSettingsPanel::onRecordClicked() {
  if (!m_controller)
    return;

  // Only Tele supports recording
  if (m_cameraMode != CameraMode::Tele)
    return;

  if (m_isRecording) {
    m_controller->stopRecord(DwarfCameraController::CameraKind::Tele);
    m_isRecording = false;
    m_recButton->setText(tr("⏺ REC"));
    m_recButton->setStyleSheet("");
  } else {
    m_controller->startRecord(DwarfCameraController::CameraKind::Tele);
    m_isRecording = true;
    m_recButton->setText(tr("⏹ STOP"));
    m_recButton->setStyleSheet("background-color: #e74c3c; color: white;");
  }
  emit recordRequested(m_isRecording);
}

void CameraSettingsPanel::onExposureModeChanged(int index) {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setExposureMode(kind, index);

  // Enable/disable manual controls
  bool manual = (index == 1);
  m_exposureSlider->setEnabled(manual);
}

void CameraSettingsPanel::onExposureSliderChanged(int value) {
  m_exposureValueLabel->setText(formatExposureValue(value));

  qWarning() << "[CameraSettingsPanel] onExposureSliderChanged" << value
             << "m_controller=" << m_controller;

  if (!m_controller) {
    qWarning() << "[CameraSettingsPanel] ERROR: m_controller is null!";
    return;
  }

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setExposureIndex(kind, value);
}

void CameraSettingsPanel::onGainModeChanged(int index) {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setGainMode(kind, index);

  // Enable/disable manual controls
  bool manual = (index == 1);
  m_gainSlider->setEnabled(manual);
}

void CameraSettingsPanel::onGainSliderChanged(int value) {
  m_gainValueLabel->setText(formatGainValue(value));

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setGainIndex(kind, value);
}

void CameraSettingsPanel::onIrCutToggled(bool checked) {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setIrCut(kind, checked ? 1 : 0);
}

void CameraSettingsPanel::onBinningChanged(int index) {
  // Binning requires camera restart - handled by MainWindow
  Q_UNUSED(index);
}

void CameraSettingsPanel::onBrightnessChanged(int value) {
  m_brightnessValueLabel->setText(QString::number(value));

  qWarning() << "[CameraSettingsPanel] onBrightnessChanged" << value
             << "m_controller=" << m_controller;

  if (!m_controller) {
    qWarning() << "[CameraSettingsPanel] ERROR: m_controller is null!";
    return;
  }

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setBrightness(kind, value);
}

void CameraSettingsPanel::onContrastChanged(int value) {
  m_contrastValueLabel->setText(QString::number(value));

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setContrast(kind, value);
}

void CameraSettingsPanel::onSaturationChanged(int value) {
  m_saturationValueLabel->setText(QString::number(value));

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setSaturation(kind, value);
}

void CameraSettingsPanel::onSharpnessChanged(int value) {
  m_sharpnessValueLabel->setText(QString::number(value));

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setSharpness(kind, value);
}

void CameraSettingsPanel::onHueChanged(int value) {
  m_hueValueLabel->setText(QString::number(value));

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setHue(kind, value);
}

void CameraSettingsPanel::onWbModeChanged(int index) {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setWhiteBalanceMode(kind, index);

  // Enable/disable manual controls
  bool manual = (index == 1);
  m_wbTemperatureSlider->setEnabled(manual);
}

void CameraSettingsPanel::onWbTemperatureChanged(int value) {
  if (value >= 0 && value < s_wbTemperatureValues.size()) {
    m_wbTemperatureValueLabel->setText(
        QString("%1K").arg(s_wbTemperatureValues[value]));
  }

  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setWhiteBalanceByTemperature(kind, value);
}

// === Setters for external state updates ===

void CameraSettingsPanel::setExposureMode(int mode) {
  m_exposureModeCombo->blockSignals(true);
  m_exposureModeCombo->setCurrentIndex(mode);
  m_exposureModeCombo->blockSignals(false);
  m_exposureSlider->setEnabled(mode == 1);
}

void CameraSettingsPanel::setExposureIndex(int index) {
  m_exposureSlider->blockSignals(true);
  m_exposureSlider->setValue(index);
  m_exposureSlider->blockSignals(false);
  m_exposureValueLabel->setText(formatExposureValue(index));
}

void CameraSettingsPanel::setGainMode(int mode) {
  m_gainModeCombo->blockSignals(true);
  m_gainModeCombo->setCurrentIndex(mode);
  m_gainModeCombo->blockSignals(false);
  m_gainSlider->setEnabled(mode == 1);
}

void CameraSettingsPanel::setGainIndex(int index) {
  m_gainSlider->blockSignals(true);
  m_gainSlider->setValue(index);
  m_gainSlider->blockSignals(false);
  m_gainValueLabel->setText(formatGainValue(index));
}

void CameraSettingsPanel::setIrCut(bool enabled) {
  m_irCutCheckBox->blockSignals(true);
  m_irCutCheckBox->setChecked(enabled);
  m_irCutCheckBox->blockSignals(false);
}

void CameraSettingsPanel::setBinning(int index) {
  m_binningCombo->blockSignals(true);
  m_binningCombo->setCurrentIndex(index);
  m_binningCombo->blockSignals(false);
}

void CameraSettingsPanel::setBrightness(int value) {
  m_brightnessSlider->blockSignals(true);
  m_brightnessSlider->setValue(value);
  m_brightnessSlider->blockSignals(false);
  m_brightnessValueLabel->setText(QString::number(value));
}

void CameraSettingsPanel::setContrast(int value) {
  m_contrastSlider->blockSignals(true);
  m_contrastSlider->setValue(value);
  m_contrastSlider->blockSignals(false);
  m_contrastValueLabel->setText(QString::number(value));
}

void CameraSettingsPanel::setSaturation(int value) {
  m_saturationSlider->blockSignals(true);
  m_saturationSlider->setValue(value);
  m_saturationSlider->blockSignals(false);
  m_saturationValueLabel->setText(QString::number(value));
}

void CameraSettingsPanel::setSharpness(int value) {
  m_sharpnessSlider->blockSignals(true);
  m_sharpnessSlider->setValue(value);
  m_sharpnessSlider->blockSignals(false);
  m_sharpnessValueLabel->setText(QString::number(value));
}

void CameraSettingsPanel::setHue(int value) {
  m_hueSlider->blockSignals(true);
  m_hueSlider->setValue(value);
  m_hueSlider->blockSignals(false);
  m_hueValueLabel->setText(QString::number(value));
}

void CameraSettingsPanel::setWhiteBalanceMode(int mode) {
  m_wbModeCombo->blockSignals(true);
  m_wbModeCombo->setCurrentIndex(mode);
  m_wbModeCombo->blockSignals(false);
  m_wbTemperatureSlider->setEnabled(mode == 1);
}

void CameraSettingsPanel::setWhiteBalanceTemperature(int index) {
  m_wbTemperatureSlider->blockSignals(true);
  m_wbTemperatureSlider->setValue(index);
  m_wbTemperatureSlider->blockSignals(false);
  if (index >= 0 && index < s_wbTemperatureValues.size()) {
    m_wbTemperatureValueLabel->setText(
        QString("%1K").arg(s_wbTemperatureValues[index]));
  }
}
