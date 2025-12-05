#pragma once

#include <QWidget>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

class DwarfCameraController;

/**
 * @brief Reusable widget for image parameters (brightness, contrast, etc.)
 * 
 * Can be embedded in both CameraSettingsPanel and AstroNavigationPanel.
 * For Astro mode, always uses Tele camera.
 */
class ImageSettingsWidget : public QWidget {
    Q_OBJECT

public:
    enum class CameraKind { Tele, Wide };
    
    explicit ImageSettingsWidget(QWidget *parent = nullptr);
    
    void setCameraController(DwarfCameraController *controller);
    void setCameraKind(CameraKind kind) { m_cameraKind = kind; }
    
    // Setters for external state updates
    void setBrightness(int value);
    void setContrast(int value);
    void setSaturation(int value);
    void setSharpness(int value);
    void setHue(int value);
    
    // Getters
    int brightness() const;
    int contrast() const;
    int saturation() const;
    int sharpness() const;
    int hue() const;

private slots:
    void onBrightnessChanged(int value);
    void onContrastChanged(int value);
    void onSaturationChanged(int value);
    void onSharpnessChanged(int value);
    void onHueChanged(int value);

private:
    void setupUi();
    
    DwarfCameraController *m_controller = nullptr;
    CameraKind m_cameraKind = CameraKind::Tele;
    
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
};

/**
 * @brief Reusable widget for white balance settings
 */
class WhiteBalanceWidget : public QWidget {
    Q_OBJECT

public:
    enum class CameraKind { Tele, Wide };
    
    explicit WhiteBalanceWidget(QWidget *parent = nullptr);
    
    void setCameraController(DwarfCameraController *controller);
    void setCameraKind(CameraKind kind) { m_cameraKind = kind; }
    
    void setWhiteBalanceMode(int mode);
    void setWhiteBalanceTemperature(int index);
    
    int whiteBalanceMode() const;
    int whiteBalanceTemperatureIndex() const;

private slots:
    void onWbModeChanged(int index);
    void onWbTemperatureChanged(int value);

private:
    void setupUi();
    void updateTemperatureLabel();
    
    DwarfCameraController *m_controller = nullptr;
    CameraKind m_cameraKind = CameraKind::Tele;
    
    QComboBox *m_wbModeCombo;
    QSlider *m_wbTemperatureSlider;
    QLabel *m_wbTemperatureValueLabel;
    
    static const QVector<int> s_wbTemperatureValues;
};
