#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QElapsedTimer>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>

#include "StarMapWidget.h"  // For CelestialObject

class DwarfWebSocketClient;
class DwarfCameraController;
class DwarfAstroController;
class ImageSettingsWidget;
class WhiteBalanceWidget;

/**
 * @brief Astro & Navigation panel with star map, object search, and stacking control
 */
class AstroNavigationPanel : public QWidget {
    Q_OBJECT

public:
    explicit AstroNavigationPanel(QWidget *parent = nullptr);
    ~AstroNavigationPanel();

    void setWebSocketClient(DwarfWebSocketClient *client);
    void setCameraController(DwarfCameraController *controller);
    void setAstroController(DwarfAstroController *controller);
    
    // Location settings
    void setLocation(double latitude, double longitude);
    double latitude() const;
    double longitude() const;
    double altitude() const;
    
    // Update telescope pointing from external source
    void setTelescopePointing(double ra, double dec);

signals:
    void gotoRequested(double ra, double dec);
    void stackingStarted(int numFrames, double exposureSeconds);
    void stackingStopped();

private slots:
    // Star map
    void onObjectSelected(const CelestialObject &obj);
    void onObjectDoubleClicked(const CelestialObject &obj);
    void onGotoClicked();
    void onCalibrateClicked();
    
    // Search
    void onSearchTextChanged(const QString &text);
    void onSearchResultClicked(QListWidgetItem *item);
    void onSearchResultDoubleClicked(QListWidgetItem *item);
    
    // Stacking
    void onStartStackingClicked();
    void onStopStackingClicked();
    void updateStackingProgress();
    
    // Settings
    void onMagnitudeLimitChanged(double value);
    void onShowConstellationsToggled(bool checked);
    void onShowGridToggled(bool checked);
    void onShowLabelsToggled(bool checked);
    void onAutoLocationClicked();
    void onLocationReceived(const QJsonObject &location);

private:
    void setupUI();
    void setupStarMapTab();
    void setupSearchTab();
    void setupStackingTab();
    void setupSettingsTab();
    
    void connectSignals();
    void loadCatalog();
    void updateVisibleObjectsList();
    void gotoObject(const CelestialObject &obj);
    
    // Formatting helpers
    QString formatExposureValue(int sliderIndex) const;
    QString formatGainValue(int sliderIndex) const;

    // Main tabs
    QTabWidget *m_tabWidget;
    
    // Star Map Tab
    StarMapWidget *m_starMap;
    QLabel *m_selectedObjectLabel;
    QLabel *m_objectInfoLabel;
    QPushButton *m_gotoButton;
    QPushButton *m_calibrateButton;
    QLabel *m_calibrationStatusLabel;
    
    // Search Tab
    QLineEdit *m_searchEdit;
    QListWidget *m_searchResults;
    QListWidget *m_visibleObjectsList;
    QLabel *m_searchStatusLabel;
    
    // Stacking Tab - Capture Settings
    QSpinBox *m_numFramesSpin;
    QSlider *m_astroExposureSlider;
    QLabel *m_astroExposureValueLabel;
    QSlider *m_astroGainSlider;
    QLabel *m_astroGainValueLabel;
    QPushButton *m_startStackingButton;
    QPushButton *m_stopStackingButton;
    QProgressBar *m_stackingProgress;
    QLabel *m_stackingStatusLabel;
    QLabel *m_frameCountLabel;
    QLabel *m_elapsedTimeLabel;
    QLabel *m_rejectedFramesLabel;
    
    // Calibration Frames (Dark, Flat, Bias)
    QSpinBox *m_darkFramesSpin;
    QSpinBox *m_flatFramesSpin;
    QSpinBox *m_biasFramesSpin;
    QPushButton *m_captureDarksButton;
    QPushButton *m_captureFlatsButton;
    QPushButton *m_captureBiasButton;
    QLabel *m_calibrationFramesStatusLabel;
    
    // Image & White Balance (shared with Camera panel)
    ImageSettingsWidget *m_imageSettings;
    WhiteBalanceWidget *m_whiteBalance;
    
    // Stacking state
    bool m_isStacking;
    int m_currentFrame;
    int m_totalFrames;
    int m_rejectedFrames;
    QTimer *m_stackingTimer;
    QElapsedTimer m_stackingElapsed;
    
    // Settings Tab
    QDoubleSpinBox *m_magnitudeLimitSpin;
    QCheckBox *m_showConstellationsCheck;
    QCheckBox *m_showGridCheck;
    QCheckBox *m_showLabelsCheck;
    QDoubleSpinBox *m_latitudeSpin;
    QDoubleSpinBox *m_longitudeSpin;
    QDoubleSpinBox *m_altitudeSpin;
    QPushButton *m_autoLocationButton;
    QLabel *m_locationStatusLabel;
    
    // Controllers
    DwarfWebSocketClient *m_wsClient;
    DwarfCameraController *m_cameraController;
    DwarfAstroController *m_astroController;
    
    // Current selection
    CelestialObject m_selectedObject;
};
