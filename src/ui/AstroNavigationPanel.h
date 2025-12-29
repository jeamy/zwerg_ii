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

class QVBoxLayout;
class QTabWidget;

#include "StarMapWidget.h"  // For CelestialObject

class DwarfWebSocketClient;
class DwarfCameraController;
class DwarfAstroController;
class Lx200Server;
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
    void setClientMode(bool enabled);
    
    void loadSettings();
    void saveSettings();
    
    // Location settings
    void setLocation(double latitude, double longitude);
    double latitude() const;
    double longitude() const;
    double altitude() const;
    
    // Update telescope pointing from external source
    void setTelescopePointing(double ra, double dec);

    StarMapWidget *starMapWidget() const;
    QWidget *starMapTabWidget() const;
    QVBoxLayout *starMapTabLayout() const;
    QWidget *starMapContentWidget() const;

    QTabWidget *tabWidget() const;
    void setCurrentTabIndex(int index);
    int currentTabIndex() const;

signals:
    void gotoRequested(double ra, double dec);
    void stackingStarted(int numFrames, double exposureSeconds);
    void stackingStopped();
    void starMapOverlayRequested(bool enabled);

private slots:
    // Star map
    void onObjectSelected(const CelestialObject &obj);
    void onObjectDoubleClicked(const CelestialObject &obj);
    void onGotoClicked();
    void onStopGotoClicked();
    void onCalibrateClicked();
    void onCancelCalibrationClicked();
    
    // Special Tracking
    void onTrackSunClicked();
    void onTrackMoonClicked();
    void onStopSpecialTrackingClicked();
    
    // Search
    void onSearchTextChanged(const QString &text);
    void onSearchResultClicked(QListWidgetItem *item);
    void onSearchResultDoubleClicked(QListWidgetItem *item);

    void onStarMapCoordinatesClicked(double ra, double dec);
    
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

    void onLx200EnableToggled(bool enabled);
    void onLx200PortChanged(int port);

    void onRaDecGotoClicked();

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

    QWidget *m_starMapTab;
    QVBoxLayout *m_starMapTabLayout;
    QWidget *m_starMapContent;
    
    // Star Map Tab
    StarMapWidget *m_starMap;
    QLabel *m_selectedObjectLabel;
    QLabel *m_objectInfoLabel;
    QPushButton *m_gotoButton;
    QPushButton *m_stopGotoButton;
    QPushButton *m_calibrateButton = nullptr;
    QPushButton *m_cancelCalibrationButton = nullptr;
    QLabel *m_calibrationStatusLabel = nullptr;
    
    // Search Tab
    QLineEdit *m_searchEdit;
    QListWidget *m_searchResults;
    QListWidget *m_visibleObjectsList;
    QLabel *m_searchStatusLabel;

    QLineEdit *m_raInput = nullptr;
    QLineEdit *m_decInput = nullptr;
    QPushButton *m_radecGotoButton = nullptr;

    // Special Tracking
    QPushButton *m_trackSunButton = nullptr;
    QPushButton *m_trackMoonButton = nullptr;
    QPushButton *m_stopSpecialTrackingButton = nullptr;
    
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
    QCheckBox *m_useDarkFramesCheck;
    QSpinBox *m_darkFramesSpin;
    QSpinBox *m_flatFramesSpin;
    QSpinBox *m_biasFramesSpin;
    QPushButton *m_captureDarksButton;
    QPushButton *m_captureFlatsButton;
    QPushButton *m_captureBiasButton;
    QLabel *m_calibrationFramesStatusLabel;
    
    // Image & White Balance are controlled via the global Parameters overlay.
    
    // Stacking state
    bool m_isStacking;
    int m_currentFrame;
    int m_totalFrames;
    int m_rejectedFrames;
    QTimer *m_stackingTimer;
    QElapsedTimer m_stackingElapsed;
    
    // Calibration safety
    QTimer *m_calibrationTimeoutTimer = nullptr;
    
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

    QCheckBox *m_lx200EnableCheck;
    QSpinBox *m_lx200PortSpin;
    QLabel *m_lx200StatusLabel;
    
    // Controllers
    DwarfWebSocketClient *m_wsClient;
    DwarfCameraController *m_cameraController;
    DwarfAstroController *m_astroController;

    Lx200Server *m_lx200Server;

    bool m_catalogLoaded;
    
    // Current selection
    CelestialObject m_selectedObject;
};
