#pragma once

#include "net/DwarfCameraController.h"
#include <QWidget>
#include <QElapsedTimer>
 #include <QJsonDocument>

class QGroupBox;
class QComboBox;
class QSlider;
class QLabel;
class QPushButton;
class QCheckBox;
class QTimer;
class QPixmap;

class DwarfCameraController;

/**
 * @brief Panel for camera settings with separate ranges for Tele/Wide cameras.
 *
 * Shows current values next to each slider and adapts ranges when switching
 * between Tele and Wide camera modes.
 */
class CameraSettingsPanel : public QWidget {
  Q_OBJECT

public:
  enum class CameraMode { Tele, Wide };
  enum class DisplayMode { Full, Compact, CaptureOnly };

  explicit CameraSettingsPanel(QWidget *parent = nullptr);

  void setCameraController(DwarfCameraController *controller);
  void setDisplayMode(DisplayMode mode);
  void setCameraMode(CameraMode mode);
  CameraMode cameraMode() const { return m_cameraMode; }

  // Update UI from external state (e.g., when receiving camera status)
  void setExposureMode(int mode);
  void setExposureIndex(int index);
  void setGainMode(int mode);
  void setGainIndex(int index);
  void setIrCut(bool enabled);
  void setBrightness(int value);
  void setContrast(int value);
  void setSaturation(int value);
  void setSharpness(int value);
  void setHue(int value);
  void setWhiteBalanceMode(int mode);
  void setWhiteBalanceTemperature(int index);

  // Update all controls from the attached DwarfCameraController
  void syncFromController();

  void setCaptureStatusText(const QString &text);
  void setCapturePreview(const QPixmap &pixmap);
  void clearCapturePreview();

  void applyDefaultParamsConfig(const QJsonDocument &document);

  // Exposure values: {api_index, display_name} for Tele camera
  static const QVector<QPair<int, QString>> s_teleExposureValues;
  // Exposure values: {api_index, display_name} for Wide camera
  static const QVector<QPair<int, QString>> s_wideExposureValues;
  // Gain values: {api_index, gain_value} for Tele camera
  static const QVector<QPair<int, int>> s_teleGainValues;
  // Gain values: {api_index, gain_value} for Wide camera
  static const QVector<QPair<int, int>> s_wideGainValues;
  // White balance color temperature values (Kelvin)
  static const QVector<int> s_wbTemperatureValues;

signals:
  void photoRequested();
  void recordRequested(bool recording);
  void cameraModeChanged(CameraMode mode);

private slots:
  void onTeleClicked();
  void onWideClicked();
  void onPhotoClicked();
  void onRecordClicked();
  void onExposureModeChanged(int index);
  void onExposureSliderChanged(int value);
  void onGainModeChanged(int index);
  void onGainSliderChanged(int value);
  void onIrCutToggled(bool checked);
  void onBrightnessChanged(int value);
  void onContrastChanged(int value);
  void onSaturationChanged(int value);
  void onSharpnessChanged(int value);
  void onHueChanged(int value);
  void onWbModeChanged(int index);
  void onWbTemperatureChanged(int value);

  void onPhotoCaptureFinished(DwarfCameraController::CameraKind kind,
                              bool success, int code,
                              const QString &fileName);
  void onRecordFinished(DwarfCameraController::CameraKind kind, bool recording,
                        bool success, int code);
  void onRecordTimerTick();

private:
  void setupUi();
  void updateRangesForMode();
  void updateValueLabels();
  void updateButtonStates();
  QString formatExposureValue(int index) const;
  QString formatGainValue(int index) const;

  DwarfCameraController *m_controller = nullptr;
  CameraMode m_cameraMode = CameraMode::Tele;
  DisplayMode m_displayMode = DisplayMode::Full;
  bool m_isRecording = false;

  QWidget *m_sourceRow = nullptr;
  QWidget *m_captureRow = nullptr;

  // Camera source buttons
  QPushButton *m_teleButton;
  QPushButton *m_wideButton;

  // Capture buttons
  QPushButton *m_photoButton;
  QPushButton *m_recButton;
  QLabel *m_captureStatusLabel = nullptr;
  QLabel *m_capturePreviewLabel = nullptr;
  QLabel *m_recordTimerLabel = nullptr;
  QTimer *m_recordTimer = nullptr;
  QElapsedTimer m_recordElapsed;
  bool m_photoPending = false;
  bool m_recordPending = false;
  bool m_recordStartRetryPending = false;

  // Exposure group
  QGroupBox *m_exposureGroup;
  QComboBox *m_exposureModeCombo;
  QSlider *m_exposureSlider;
  QLabel *m_exposureValueLabel;
  QComboBox *m_gainModeCombo;
  QSlider *m_gainSlider;
  QLabel *m_gainValueLabel;

  // Image parameters group
  QGroupBox *m_imageGroup;
  QCheckBox *m_irCutCheckBox;
  QSlider *m_brightnessSlider;
  QLabel *m_brightnessValueLabel;
  QSlider *m_contrastSlider;
  QLabel *m_contrastValueLabel;
  QSlider *m_saturationSlider;
  QLabel *m_saturationValueLabel;
  QSlider *m_sharpnessSlider;
  QLabel *m_sharpnessValueLabel;
  QSlider *m_hueSlider;
  QLabel *m_hueValueLabel;

  // White balance group
  QGroupBox *m_wbGroup;
  QComboBox *m_wbModeCombo;
  QSlider *m_wbTemperatureSlider;
  QLabel *m_wbTemperatureValueLabel;

  QVector<QPair<int, QString>> m_teleExposureValuesDyn;
  QVector<QPair<int, QString>> m_wideExposureValuesDyn;
  QVector<QPair<int, int>> m_teleGainValuesDyn;
  QVector<QPair<int, int>> m_wideGainValuesDyn;
};
