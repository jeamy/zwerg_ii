#include "CameraSettingsPanel.h"
#include "../net/DwarfCameraController.h"

#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>
 #include <QPixmap>
 #include <QJsonArray>
 #include <QJsonObject>

// Exposure index values for Tele camera (DWARF II API)
// Format: {api_index, display_name} - API uses indices 0,3,6,9,...,156
const QVector<QPair<int, QString>> CameraSettingsPanel::s_teleExposureValues = {
    {0, "1/10000"}, {3, "1/8000"},  {6, "1/6400"},  {9, "1/5000"},
    {12, "1/4000"}, {15, "1/3200"}, {18, "1/2500"}, {21, "1/2000"},
    {24, "1/1600"}, {27, "1/1250"}, {30, "1/1000"}, {33, "1/800"},
    {36, "1/640"},  {39, "1/500"},  {42, "1/400"},  {45, "1/320"},
    {48, "1/250"},  {51, "1/200"},  {54, "1/160"},  {57, "1/125"},
    {60, "1/100"},  {63, "1/80"},   {66, "1/60"},   {69, "1/50"},
    {72, "1/40"},   {75, "1/30"},   {78, "1/25"},   {81, "1/20"},
    {84, "1/15"},   {87, "1/13"},   {90, "1/10"},   {93, "1/8"},
    {96, "1/6"},    {99, "1/5"},    {102, "1/4"},   {105, "1/3"},
    {108, "0.4s"},  {111, "0.5s"},  {114, "0.6s"},  {117, "0.8s"},
    {120, "1s"},    {123, "1.3s"},  {126, "1.6s"},  {129, "2s"},
    {132, "2.5s"},  {135, "3.2s"},  {138, "4s"},    {141, "5s"},
    {144, "6s"},    {147, "8s"},    {150, "10s"},   {153, "13s"},
    {156, "15s"}};

// Exposure index values for Wide camera (DWARF II API)
// Wide camera has shorter max exposure (1s)
const QVector<QPair<int, QString>> CameraSettingsPanel::s_wideExposureValues = {
    {0, "3/10000"}, {3, "1/2500"}, {6, "1/2000"}, {9, "1/1600"}, {12, "1/1250"},
    {15, "1/1000"}, {18, "1/800"}, {21, "1/640"}, {24, "1/500"}, {27, "1/400"},
    {30, "1/320"},  {33, "1/250"}, {36, "1/160"}, {39, "1/200"}, {42, "1/125"},
    {45, "1/100"},  {48, "1/80"},  {51, "1/60"},  {54, "1/50"},  {57, "1/40"},
    {60, "1/30"},   {63, "1/25"},  {66, "1/20"},  {69, "1/15"},  {72, "1/13"},
    {75, "1/10"},   {78, "1/8"},   {81, "1/6"},   {84, "1/5"},   {87, "1/4"},
    {90, "1/4"},    {93, "0.4s"},  {96, "0.5s"},  {99, "0.6s"},  {102, "0.8s"},
    {105, "1s"}};

// Gain index values for Tele camera (DWARF II API)
// Format: {api_index, gain_value} - Gain 0-240 in steps of 10
const QVector<QPair<int, int>> CameraSettingsPanel::s_teleGainValues = {
    {0, 0},   {3, 10},  {6, 20},  {9, 30},   {12, 40},  {15, 50}, {18, 60},
    {21, 70}, {24, 80}, {27, 90}, {30, 100}, {33, 110}, {36, 120}};

// Gain index values for Wide camera (DWARF II API)
// Wide camera gain starts at 0 and goes to 120
const QVector<QPair<int, int>> CameraSettingsPanel::s_wideGainValues = {
    {0, 0},   {3, 10},  {6, 20},  {9, 30},   {12, 40},  {15, 50}, {18, 60},
    {21, 70}, {24, 80}, {27, 90}, {30, 100}, {33, 110}, {36, 120}};

// White balance color temperatures in Kelvin
const QVector<int> CameraSettingsPanel::s_wbTemperatureValues = {
    2800, 3200, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000};

CameraSettingsPanel::CameraSettingsPanel(QWidget *parent) : QWidget(parent) {
  setupUi();
  updateRangesForMode();
  updateButtonStates();
}

static bool parseIndexNameList(const QJsonValue &v,
                               QVector<QPair<int, QString>> *out) {
  if (!out)
    return false;
  if (!v.isArray())
    return false;
  QVector<QPair<int, QString>> tmp;
  for (const auto &it : v.toArray()) {
    if (!it.isObject())
      continue;
    const QJsonObject o = it.toObject();
    const int idx = o.value(QStringLiteral("index")).toInt(
        o.value(QStringLiteral("exp_index")).toInt(-1));
    QString name = o.value(QStringLiteral("name")).toString(
        o.value(QStringLiteral("exp_name")).toString());
    if (idx < 0)
      continue;
    if (name.isEmpty())
      name = QString::number(idx);
    tmp.append(qMakePair(idx, name));
  }
  if (tmp.isEmpty())
    return false;
  *out = tmp;
  return true;
}

static bool parseIndexValueList(const QJsonValue &v, QVector<QPair<int, int>> *out) {
  if (!out)
    return false;
  if (!v.isArray())
    return false;
  QVector<QPair<int, int>> tmp;
  for (const auto &it : v.toArray()) {
    if (!it.isObject())
      continue;
    const QJsonObject o = it.toObject();
    const int idx = o.value(QStringLiteral("index")).toInt(
        o.value(QStringLiteral("gain_index")).toInt(-1));
    const int val = o.value(QStringLiteral("value")).toInt(
        o.value(QStringLiteral("gain_value")).toInt(-1));
    if (idx < 0)
      continue;
    tmp.append(qMakePair(idx, val));
  }
  if (tmp.isEmpty())
    return false;
  *out = tmp;
  return true;
}

void CameraSettingsPanel::applyDefaultParamsConfig(const QJsonDocument &document) {
  const QJsonObject root = document.isObject() ? document.object() : QJsonObject();
  if (root.isEmpty())
    return;

  // Best-effort parsing: firmware JSON structure can vary.
  // Try common top-level keys first.
  const QJsonValue tele = root.value(QStringLiteral("tele"));
  const QJsonValue wide = root.value(QStringLiteral("wide"));

  QVector<QPair<int, QString>> teleExp;
  QVector<QPair<int, QString>> wideExp;
  QVector<QPair<int, int>> teleGain;
  QVector<QPair<int, int>> wideGain;

  auto parseCameraObj = [&](const QJsonValue &cam, bool isTele) {
    if (!cam.isObject())
      return;
    const QJsonObject o = cam.toObject();
    parseIndexNameList(o.value(QStringLiteral("exposure")), isTele ? &teleExp : &wideExp);
    parseIndexNameList(o.value(QStringLiteral("exp")), isTele ? &teleExp : &wideExp);
    parseIndexNameList(o.value(QStringLiteral("exposure_list")), isTele ? &teleExp : &wideExp);

    parseIndexValueList(o.value(QStringLiteral("gain")), isTele ? &teleGain : &wideGain);
    parseIndexValueList(o.value(QStringLiteral("gain_list")), isTele ? &teleGain : &wideGain);
  };

  parseCameraObj(tele, true);
  parseCameraObj(wide, false);

  // Fallback: look for any array fields named exp_name/exp_index and gain_index/gain_value.
  if (teleExp.isEmpty() || wideExp.isEmpty() || teleGain.isEmpty() || wideGain.isEmpty()) {
    for (auto it = root.begin(); it != root.end(); ++it) {
      const QJsonValue v = it.value();
      if (v.isObject()) {
        const QJsonObject o = v.toObject();
        if (teleExp.isEmpty())
          parseIndexNameList(o.value(QStringLiteral("exp_list")), &teleExp);
        if (teleExp.isEmpty())
          parseIndexNameList(o.value(QStringLiteral("exposure")), &teleExp);
        if (teleGain.isEmpty())
          parseIndexValueList(o.value(QStringLiteral("gain")), &teleGain);
      }
    }
  }

  if (!teleExp.isEmpty())
    m_teleExposureValuesDyn = teleExp;
  if (!wideExp.isEmpty())
    m_wideExposureValuesDyn = wideExp;
  if (!teleGain.isEmpty())
    m_teleGainValuesDyn = teleGain;
  if (!wideGain.isEmpty())
    m_wideGainValuesDyn = wideGain;

  updateRangesForMode();
  updateValueLabels();
}

void CameraSettingsPanel::setDisplayMode(DisplayMode mode) {
  m_displayMode = mode;

  const bool compact = (m_displayMode == DisplayMode::Compact);
  const bool captureOnly = (m_displayMode == DisplayMode::CaptureOnly);

  // Rows
  if (m_sourceRow)
    m_sourceRow->setVisible(!compact);
  if (m_captureRow)
    m_captureRow->setVisible(!compact);

  // Groups
  if (m_exposureGroup)
    m_exposureGroup->setVisible(!captureOnly);
  if (m_imageGroup)
    m_imageGroup->setVisible(!captureOnly);
  if (m_wbGroup)
    m_wbGroup->setVisible(!captureOnly);
}

void CameraSettingsPanel::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(8);

  // === Camera Source Selection ===
  m_sourceRow = new QWidget(this);
  QHBoxLayout *sourceLayout = new QHBoxLayout(m_sourceRow);
  sourceLayout->setContentsMargins(0, 0, 0, 0);
  sourceLayout->setSpacing(8);
  m_teleButton = new QPushButton(tr("TELE"), m_sourceRow);
  m_wideButton = new QPushButton(tr("WIDE"), m_sourceRow);
  m_teleButton->setCheckable(true);
  m_wideButton->setCheckable(true);
  m_teleButton->setChecked(true);
  m_teleButton->setMinimumHeight(36);
  m_wideButton->setMinimumHeight(36);
  sourceLayout->addWidget(m_teleButton);
  sourceLayout->addWidget(m_wideButton);
  mainLayout->addWidget(m_sourceRow);

  connect(m_teleButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onTeleClicked);
  connect(m_wideButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onWideClicked);

  // === Capture Buttons ===
  m_captureRow = new QWidget(this);
  auto *captureV = new QVBoxLayout(m_captureRow);
  captureV->setContentsMargins(0, 0, 0, 0);
  captureV->setSpacing(6);

  auto *captureButtonsRow = new QWidget(m_captureRow);
  auto *captureLayout = new QHBoxLayout(captureButtonsRow);
  captureLayout->setContentsMargins(0, 0, 0, 0);
  captureLayout->setSpacing(8);

  m_photoButton = new QPushButton(tr("📷 PHOTO"), captureButtonsRow);
  m_recButton = new QPushButton(tr("⏺ REC"), captureButtonsRow);
  m_recordTimerLabel = new QLabel(tr("00:00"), captureButtonsRow);
  m_recordTimerLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  m_recordTimerLabel->setMinimumWidth(60);
  m_recordTimerLabel->setVisible(false);

  // Match capture button sizing to TELE/WIDE buttons
  m_photoButton->setMinimumHeight(36);
  m_recButton->setMinimumHeight(36);
  m_photoButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_recButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_captureStatusLabel = new QLabel(QString(), m_captureRow);
  m_captureStatusLabel->setSizePolicy(QSizePolicy::Expanding,
                                      QSizePolicy::Preferred);
  m_captureStatusLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  // Status row under buttons: status text on the left, recording timer on the right
  auto *statusRow = new QWidget(m_captureRow);
  auto *statusLayout = new QHBoxLayout(statusRow);
  statusLayout->setContentsMargins(0, 0, 0, 0);
  statusLayout->setSpacing(8);
  statusLayout->addWidget(m_captureStatusLabel, 0);
  statusLayout->addWidget(m_recordTimerLabel, 0);
  statusLayout->addStretch(1);

  m_capturePreviewLabel = new QLabel(m_captureRow);
  m_capturePreviewLabel->setVisible(false);
  m_capturePreviewLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_capturePreviewLabel->setMinimumHeight(60);

  m_photoButton->setObjectName("photoButton");
  m_recButton->setObjectName("recButton");
  captureLayout->addWidget(m_photoButton, 1);
  captureLayout->addWidget(m_recButton, 1);

  captureV->addWidget(captureButtonsRow);
  captureV->addWidget(statusRow);
  captureV->addWidget(m_capturePreviewLabel);
  mainLayout->addWidget(m_captureRow);

  connect(m_photoButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onPhotoClicked);
  connect(m_recButton, &QPushButton::clicked, this,
          &CameraSettingsPanel::onRecordClicked);

  // === Exposure Group ===
  m_exposureGroup = new QGroupBox(tr("Exposure"), this);
  m_exposureGroup->setObjectName("paramsExposureGroup");
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

  connect(m_exposureModeCombo,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &CameraSettingsPanel::onExposureModeChanged);
  connect(m_exposureSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onExposureSliderChanged);
  connect(m_gainModeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &CameraSettingsPanel::onGainModeChanged);
  connect(m_gainSlider, &QSlider::valueChanged, this,
          &CameraSettingsPanel::onGainSliderChanged);

  // === Image Parameters Group ===
  m_imageGroup = new QGroupBox(tr("Image"), this);
  m_imageGroup->setObjectName("paramsImageGroup");
  QGridLayout *imageLayout = new QGridLayout(m_imageGroup);
  imageLayout->setColumnStretch(1, 1);

  // IR-Cut filter
  m_irCutCheckBox = new QCheckBox(tr("IR-Cut (Day mode)"), m_imageGroup);
  imageLayout->addWidget(m_irCutCheckBox, 0, 0, 1, 3);

  // Brightness
  QLabel *brightnessLabel = new QLabel(tr("Brightness:"), m_imageGroup);
  m_brightnessSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_brightnessSlider->setRange(-100, 100);
  m_brightnessSlider->setValue(0);
  m_brightnessValueLabel = new QLabel("0", m_imageGroup);
  m_brightnessValueLabel->setMinimumWidth(40);
  m_brightnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(brightnessLabel, 1, 0);
  imageLayout->addWidget(m_brightnessSlider, 1, 1);
  imageLayout->addWidget(m_brightnessValueLabel, 1, 2);

  // Contrast
  QLabel *contrastLabel = new QLabel(tr("Contrast:"), m_imageGroup);
  m_contrastSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_contrastSlider->setRange(-100, 100);
  m_contrastSlider->setValue(0);
  m_contrastValueLabel = new QLabel("0", m_imageGroup);
  m_contrastValueLabel->setMinimumWidth(40);
  m_contrastValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(contrastLabel, 2, 0);
  imageLayout->addWidget(m_contrastSlider, 2, 1);
  imageLayout->addWidget(m_contrastValueLabel, 2, 2);

  // Saturation
  QLabel *saturationLabel = new QLabel(tr("Saturation:"), m_imageGroup);
  m_saturationSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_saturationSlider->setRange(-100, 100);
  m_saturationSlider->setValue(0);
  m_saturationValueLabel = new QLabel("0", m_imageGroup);
  m_saturationValueLabel->setMinimumWidth(40);
  m_saturationValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(saturationLabel, 3, 0);
  imageLayout->addWidget(m_saturationSlider, 3, 1);
  imageLayout->addWidget(m_saturationValueLabel, 3, 2);

  // Sharpness
  QLabel *sharpnessLabel = new QLabel(tr("Sharpness:"), m_imageGroup);
  m_sharpnessSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_sharpnessSlider->setRange(0, 100);
  m_sharpnessSlider->setValue(50);
  m_sharpnessValueLabel = new QLabel("50", m_imageGroup);
  m_sharpnessValueLabel->setMinimumWidth(40);
  m_sharpnessValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(sharpnessLabel, 4, 0);
  imageLayout->addWidget(m_sharpnessSlider, 4, 1);
  imageLayout->addWidget(m_sharpnessValueLabel, 4, 2);

  // Hue
  QLabel *hueLabel = new QLabel(tr("Hue:"), m_imageGroup);
  m_hueSlider = new QSlider(Qt::Horizontal, m_imageGroup);
  m_hueSlider->setRange(-180, 180);
  m_hueSlider->setValue(0);
  m_hueValueLabel = new QLabel("0", m_imageGroup);
  m_hueValueLabel->setMinimumWidth(40);
  m_hueValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  imageLayout->addWidget(hueLabel, 5, 0);
  imageLayout->addWidget(m_hueSlider, 5, 1);
  imageLayout->addWidget(m_hueValueLabel, 5, 2);

  mainLayout->addWidget(m_imageGroup);

  connect(m_irCutCheckBox, &QCheckBox::toggled, this,
          &CameraSettingsPanel::onIrCutToggled);
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
  m_wbGroup->setObjectName("paramsWhiteBalanceGroup");
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

  setDisplayMode(m_displayMode);

  // Add stretch at bottom
  mainLayout->addStretch();
}

void CameraSettingsPanel::setCameraController(
    DwarfCameraController *controller) {
  m_controller = controller;

  if (m_controller) {
    connect(m_controller, &DwarfCameraController::photoCaptureFinished, this,
            &CameraSettingsPanel::onPhotoCaptureFinished);
    connect(m_controller, &DwarfCameraController::recordFinished, this,
            &CameraSettingsPanel::onRecordFinished);
  }

  if (!m_recordTimer) {
    m_recordTimer = new QTimer(this);
    m_recordTimer->setInterval(1000);
    connect(m_recordTimer, &QTimer::timeout, this,
            &CameraSettingsPanel::onRecordTimerTick);
  }

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
    const auto &exp = m_teleExposureValuesDyn.isEmpty() ? s_teleExposureValues
                                                        : m_teleExposureValuesDyn;
    const auto &gain = m_teleGainValuesDyn.isEmpty() ? s_teleGainValues
                                                     : m_teleGainValuesDyn;
    m_exposureSlider->setRange(0, exp.size() - 1);
    m_gainSlider->setRange(0, gain.size() - 1);
    // Video recording and IR-Cut only available on Tele
    m_recButton->setEnabled(true);
    m_recButton->setToolTip(QString());
    m_irCutCheckBox->setEnabled(true);
    m_irCutCheckBox->setVisible(true);
  } else {
    const auto &exp = m_wideExposureValuesDyn.isEmpty() ? s_wideExposureValues
                                                        : m_wideExposureValuesDyn;
    const auto &gain = m_wideGainValuesDyn.isEmpty() ? s_wideGainValues
                                                     : m_wideGainValuesDyn;
    m_exposureSlider->setRange(0, exp.size() - 1);
    m_gainSlider->setRange(0, gain.size() - 1);
    // Video recording and IR-Cut not available on Wide
    m_recButton->setEnabled(false);
    m_recButton->setToolTip(
        tr("Video recording only available on TELE camera"));
    m_irCutCheckBox->setEnabled(false);
    m_irCutCheckBox->setVisible(false);
  }

  m_exposureSlider->blockSignals(false);
  m_gainSlider->blockSignals(false);

  updateValueLabels();
}

void CameraSettingsPanel::updateButtonStates() {
  m_teleButton->setChecked(m_cameraMode == CameraMode::Tele);
  m_wideButton->setChecked(m_cameraMode == CameraMode::Wide);

  // Visual feedback for active camera
  QString activeStyle =
      "font-weight: bold; background-color: #3498db; color: white;";
  QString inactiveStyle = "";

  m_teleButton->setStyleSheet(m_cameraMode == CameraMode::Tele ? activeStyle
                                                               : inactiveStyle);
  m_wideButton->setStyleSheet(m_cameraMode == CameraMode::Wide ? activeStyle
                                                               : inactiveStyle);
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

  // Image + IR-cut
  setIrCut(m_controller->irCutEnabled(kind));
  setBrightness(m_controller->brightness(kind));
  setContrast(m_controller->contrast(kind));
  setSaturation(m_controller->saturation(kind));
  setHue(m_controller->hue(kind));
  setSharpness(m_controller->sharpness(kind));
}

void CameraSettingsPanel::setCaptureStatusText(const QString &text) {
  if (!m_captureStatusLabel)
    return;
  m_captureStatusLabel->setText(text);
}

void CameraSettingsPanel::setCapturePreview(const QPixmap &pixmap) {
  if (!m_capturePreviewLabel)
    return;

  if (pixmap.isNull()) {
    clearCapturePreview();
    return;
  }

  const int h = 72;
  m_capturePreviewLabel->setPixmap(
      pixmap.scaledToHeight(h, Qt::SmoothTransformation));
  m_capturePreviewLabel->setVisible(true);
}

void CameraSettingsPanel::clearCapturePreview() {
  if (!m_capturePreviewLabel)
    return;
  m_capturePreviewLabel->clear();
  m_capturePreviewLabel->setVisible(false);
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

QString CameraSettingsPanel::formatExposureValue(int sliderIndex) const {
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleExposureValuesDyn.isEmpty() ? s_teleExposureValues
                                                              : m_teleExposureValuesDyn)
                           : (m_wideExposureValuesDyn.isEmpty() ? s_wideExposureValues
                                                              : m_wideExposureValuesDyn);

  if (sliderIndex < 0 || sliderIndex >= values.size())
    return "---";

  return values[sliderIndex].second;
}

QString CameraSettingsPanel::formatGainValue(int sliderIndex) const {
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleGainValuesDyn.isEmpty() ? s_teleGainValues
                                                          : m_teleGainValuesDyn)
                           : (m_wideGainValuesDyn.isEmpty() ? s_wideGainValues
                                                          : m_wideGainValuesDyn);

  if (sliderIndex < 0 || sliderIndex >= values.size())
    return "---";

  return QString::number(values[sliderIndex].second);
}

// === Slots ===

void CameraSettingsPanel::onTeleClicked() { setCameraMode(CameraMode::Tele); }

void CameraSettingsPanel::onWideClicked() { setCameraMode(CameraMode::Wide); }

void CameraSettingsPanel::onPhotoClicked() {
  if (!m_controller || m_photoPending)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;

  m_photoPending = true;
  m_photoButton->setEnabled(false);
  m_captureStatusLabel->setText(tr("Taking photo..."));
  m_controller->takePhoto(kind);
  emit photoRequested();
}

void CameraSettingsPanel::onRecordClicked() {
  if (!m_controller || m_recordPending)
    return;

  // Only Tele supports recording
  if (m_cameraMode != CameraMode::Tele)
    return;

  m_recordPending = true;
  m_recButton->setEnabled(false);

  if (m_isRecording) {
    m_captureStatusLabel->setText(tr("Stopping video..."));
    m_controller->stopRecord(DwarfCameraController::CameraKind::Tele);
  } else {
    m_captureStatusLabel->setText(tr("Starting video..."));
    m_controller->startRecord(DwarfCameraController::CameraKind::Tele);
  }
}

void CameraSettingsPanel::onPhotoCaptureFinished(
    DwarfCameraController::CameraKind kind, bool success, int code,
    const QString &fileName) {
  Q_UNUSED(fileName);

  const auto expected = (m_cameraMode == CameraMode::Tele)
                            ? DwarfCameraController::CameraKind::Tele
                            : DwarfCameraController::CameraKind::Wide;
  if (kind != expected)
    return;

  m_photoPending = false;
  m_photoButton->setEnabled(true);

  if (success) {
    // cmd 10002 ack is often only "accepted"; actual save can take as long as
    // the exposure time. Final filename/preview is shown after MediaList refresh.
    m_captureStatusLabel->setText(tr("Processing..."));
  } else {
    m_captureStatusLabel->setText(tr("Photo failed (code %1)").arg(code));
    m_photoButton->setStyleSheet(
        "background-color: #e74c3c; color: white; font-weight: bold; border-radius: 4px;");
    QTimer::singleShot(1200, this,
                       [this]() { m_photoButton->setStyleSheet(""); });
  }
}

void CameraSettingsPanel::onRecordFinished(DwarfCameraController::CameraKind kind,
                                          bool recording, bool success,
                                          int code) {
  if (kind != DwarfCameraController::CameraKind::Tele)
    return;

  m_recordPending = false;
  m_recButton->setEnabled(m_cameraMode == CameraMode::Tele);

  if (!success) {
    QString errText = tr("Video failed (code %1)").arg(code);
    if (code == -10515)
      errText = tr("Video start failed (code %1)").arg(code);
    else if (code == -10518)
      errText = tr("Video stop failed: not recording (code %1)").arg(code);
    m_captureStatusLabel->setText(errText);

    // Reset retry flag after any failure.
    m_recordStartRetryPending = false;

    // If START failed, keep UI in non-recording state.
    // If STOP failed, keep UI in recording state (unknown device state).
    if (!recording && code == -10518) {
      // Firmware says "not recording" -> force UI to non-recording state
      m_isRecording = false;
      m_recButton->setText(tr("⏺ REC"));
      m_recButton->setStyleSheet("");
      if (m_recordTimer)
        m_recordTimer->stop();
      if (m_recordTimerLabel)
        m_recordTimerLabel->setVisible(false);
      return;
    }

    if (recording) {
      // START failed -> not recording
      m_isRecording = false;
      m_recButton->setText(tr("⏺ REC"));
      m_recButton->setStyleSheet("");
      if (m_recordTimer)
        m_recordTimer->stop();
      if (m_recordTimerLabel)
        m_recordTimerLabel->setVisible(false);
    } else {
      // STOP failed -> assume still recording
      m_isRecording = true;
      m_recButton->setText(tr("⏹ STOP"));
      m_recButton->setStyleSheet("background-color: #e74c3c; color: white;");
      if (m_recordTimerLabel)
        m_recordTimerLabel->setVisible(true);
      if (m_recordTimer && !m_recordTimer->isActive())
        m_recordTimer->start();
    }
    return;
  }

  // Success clears any pending retry state
  m_recordStartRetryPending = false;

  m_isRecording = recording;
  if (m_isRecording) {
    m_captureStatusLabel->setText(tr("Recording..."));
    m_recButton->setText(tr("⏹ STOP"));
    m_recButton->setStyleSheet("background-color: #e74c3c; color: white;");

    m_recordElapsed.restart();
    m_recordTimerLabel->setText(tr("00:00"));
    m_recordTimerLabel->setVisible(true);
    if (m_recordTimer)
      m_recordTimer->start();
  } else {
    m_captureStatusLabel->setText(tr("Video saved"));
    m_recButton->setText(tr("⏺ REC"));
    m_recButton->setStyleSheet("");

    if (m_recordTimer)
      m_recordTimer->stop();
    m_recordTimerLabel->setVisible(false);
  }

  emit recordRequested(m_isRecording);
}

void CameraSettingsPanel::onRecordTimerTick() {
  if (!m_isRecording || !m_recordTimerLabel)
    return;

  const qint64 secs = m_recordElapsed.isValid() ? (m_recordElapsed.elapsed() / 1000)
                                                : 0;
  const qint64 mm = secs / 60;
  const qint64 ss = secs % 60;
  m_recordTimerLabel->setText(QStringLiteral("%1:%2")
                                  .arg(mm, 2, 10, QLatin1Char('0'))
                                  .arg(ss, 2, 10, QLatin1Char('0')));
}

void CameraSettingsPanel::onExposureModeChanged(int index) {
  if (!m_controller)
    return;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setExposureMode(kind, index);

  // After switching Auto/Manual, fetch current parameters from camera so
  // the GUI reflects whatever the device actually chose/clamped.
  m_controller->fetchAllParams(kind);

  // Enable/disable manual controls
  bool manual = (index == 1);
  m_exposureSlider->setEnabled(manual);
}

void CameraSettingsPanel::onExposureSliderChanged(int sliderIndex) {
  m_exposureValueLabel->setText(formatExposureValue(sliderIndex));

  if (!m_controller) {
    qWarning() << "[CameraSettingsPanel] ERROR: m_controller is null!";
    return;
  }

  // If user adjusts exposure value, ensure we are in manual mode;
  // otherwise firmware may ignore exp_index while exp_mode is Auto.
  if (m_exposureModeCombo && m_exposureModeCombo->currentIndex() != 1) {
    auto kind = (m_cameraMode == CameraMode::Tele)
                    ? DwarfCameraController::CameraKind::Tele
                    : DwarfCameraController::CameraKind::Wide;
    m_controller->setExposureMode(kind, 1);
    m_exposureModeCombo->blockSignals(true);
    m_exposureModeCombo->setCurrentIndex(1);
    m_exposureModeCombo->blockSignals(false);
    m_exposureSlider->setEnabled(true);
  }

  // Convert slider index to API index
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleExposureValuesDyn.isEmpty()
                                  ? s_teleExposureValues
                                  : m_teleExposureValuesDyn)
                           : (m_wideExposureValuesDyn.isEmpty()
                                  ? s_wideExposureValues
                                  : m_wideExposureValuesDyn);
  if (sliderIndex < 0 || sliderIndex >= values.size())
    return;

  int apiIndex = values[sliderIndex].first;
  qWarning() << "[CameraSettingsPanel] onExposureSliderChanged slider="
             << sliderIndex << "apiIndex=" << apiIndex;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setExposureIndex(kind, apiIndex);
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

  // Also refresh from camera after changing gain mode
  m_controller->fetchAllParams(kind);
}

void CameraSettingsPanel::onGainSliderChanged(int sliderIndex) {
  m_gainValueLabel->setText(formatGainValue(sliderIndex));

  if (!m_controller)
    return;

  // Convert slider index to API index
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleGainValuesDyn.isEmpty() ? s_teleGainValues
                                                           : m_teleGainValuesDyn)
                           : (m_wideGainValuesDyn.isEmpty() ? s_wideGainValues
                                                           : m_wideGainValuesDyn);
  if (sliderIndex < 0 || sliderIndex >= values.size())
    return;

  int apiIndex = values[sliderIndex].first;
  qWarning() << "[CameraSettingsPanel] onGainSliderChanged slider="
             << sliderIndex << "apiIndex=" << apiIndex
             << "gainValue=" << values[sliderIndex].second;

  auto kind = (m_cameraMode == CameraMode::Tele)
                  ? DwarfCameraController::CameraKind::Tele
                  : DwarfCameraController::CameraKind::Wide;
  m_controller->setGainIndex(kind, apiIndex);
}

void CameraSettingsPanel::onIrCutToggled(bool checked) {
  if (!m_controller || m_cameraMode != CameraMode::Tele)
    return;

  // IR-Cut only available on Tele camera
  m_controller->setIrCut(DwarfCameraController::CameraKind::Tele,
                         checked ? 1 : 0);
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

void CameraSettingsPanel::setExposureIndex(int apiIndex) {
  // Convert API index to slider index
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleExposureValuesDyn.isEmpty() ? s_teleExposureValues
                                                              : m_teleExposureValuesDyn)
                           : (m_wideExposureValuesDyn.isEmpty() ? s_wideExposureValues
                                                              : m_wideExposureValuesDyn);
  int sliderIndex = 0;
  for (int i = 0; i < values.size(); ++i) {
    if (values[i].first == apiIndex) {
      sliderIndex = i;
      break;
    }
  }
  // Find closest match if exact not found
  for (int i = 0; i < values.size(); ++i) {
    if (values[i].first <= apiIndex) {
      sliderIndex = i;
    }
  }

  m_exposureSlider->blockSignals(true);
  m_exposureSlider->setValue(sliderIndex);
  m_exposureSlider->blockSignals(false);
  m_exposureValueLabel->setText(formatExposureValue(sliderIndex));
}

void CameraSettingsPanel::setGainMode(int mode) {
  m_gainModeCombo->blockSignals(true);
  m_gainModeCombo->setCurrentIndex(mode);
  m_gainModeCombo->blockSignals(false);
  m_gainSlider->setEnabled(mode == 1);
}

void CameraSettingsPanel::setGainIndex(int apiIndex) {
  // Convert API index to slider index
  const auto &values = (m_cameraMode == CameraMode::Tele)
                           ? (m_teleGainValuesDyn.isEmpty() ? s_teleGainValues
                                                           : m_teleGainValuesDyn)
                           : (m_wideGainValuesDyn.isEmpty() ? s_wideGainValues
                                                           : m_wideGainValuesDyn);
  int sliderIndex = 0;
  for (int i = 0; i < values.size(); ++i) {
    if (values[i].first == apiIndex) {
      sliderIndex = i;
      break;
    }
    // Find closest match if exact not found
    if (values[i].first <= apiIndex) {
      sliderIndex = i;
    }
  }

  m_gainSlider->blockSignals(true);
  m_gainSlider->setValue(sliderIndex);
  m_gainSlider->blockSignals(false);
  m_gainValueLabel->setText(formatGainValue(sliderIndex));
}

void CameraSettingsPanel::setIrCut(bool enabled) {
  m_irCutCheckBox->blockSignals(true);
  m_irCutCheckBox->setChecked(enabled);
  m_irCutCheckBox->blockSignals(false);
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

