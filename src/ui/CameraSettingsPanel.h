#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QWidget>

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

  explicit CameraSettingsPanel(QWidget *parent = nullptr);

  void setCameraController(DwarfCameraController *controller);
  void setCameraMode(CameraMode mode);
  CameraMode cameraMode() const { return m_cameraMode; }

  // Update UI from external state (e.g., when receiving camera status)
  void setExposureMode(int mode);
  void setExposureIndex(int index);
  void setGainMode(int mode);
  void setGainIndex(int index);
  void setIrCut(bool enabled);
  void setBinning(int index);
  void setBrightness(int value);
  void setContrast(int value);
  void setSaturation(int value);
  void setSharpness(int value);
  void setHue(int value);
  void setWhiteBalanceMode(int mode);
  void setWhiteBalanceTemperature(int index);

  // Update all controls from the attached DwarfCameraController
  void syncFromController();

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
  void onBinningChanged(int index);
  void onBrightnessChanged(int value);
  void onContrastChanged(int value);
  void onSaturationChanged(int value);
  void onSharpnessChanged(int value);
  void onHueChanged(int value);
  void onWbModeChanged(int index);
  void onWbTemperatureChanged(int value);

private:
  void setupUi();
  void updateRangesForMode();
  void updateValueLabels();
  void updateButtonStates();
  QString formatExposureValue(int index) const;
  QString formatGainValue(int index) const;

  DwarfCameraController *m_controller = nullptr;
  CameraMode m_cameraMode = CameraMode::Tele;
  bool m_isRecording = false;

  // Camera source buttons
  QPushButton *m_teleButton;
  QPushButton *m_wideButton;

  // Capture buttons
  QPushButton *m_photoButton;
  QPushButton *m_recButton;

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
  QComboBox *m_binningCombo;
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

  // Exposure time values in microseconds for Tele camera (index -> value)
  static const QVector<int> s_teleExposureValues;
  // Exposure time values for Wide camera
  static const QVector<int> s_wideExposureValues;
  // Gain values for Tele camera
  static const QVector<int> s_teleGainValues;
  // Gain values for Wide camera
  static const QVector<int> s_wideGainValues;
  // White balance color temperature values (Kelvin)
  static const QVector<int> s_wbTemperatureValues;
};
