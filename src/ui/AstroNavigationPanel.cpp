#include "AstroNavigationPanel.h"
#include "StarMapWidget.h"
#include "ImageSettingsWidget.h"
#include "../net/DwarfWebSocketClient.h"
#include "../net/DwarfCameraController.h"
#include "../net/DwarfAstroController.h"
#include "../net/Lx200Server.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QScrollArea>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStyle>
#include <QRegularExpression>

#include <cmath>

AstroNavigationPanel::AstroNavigationPanel(QWidget *parent)
    : QWidget(parent)
    , m_tabWidget(new QTabWidget(this))
    , m_starMapTab(nullptr)
    , m_starMapTabLayout(nullptr)
    , m_starMapContent(nullptr)
    , m_starMap(nullptr)
    , m_wsClient(nullptr)
    , m_cameraController(nullptr)
    , m_astroController(nullptr)
    , m_isStacking(false)
    , m_currentFrame(0)
    , m_totalFrames(0)
    , m_rejectedFrames(0)
    , m_stackingTimer(new QTimer(this))
    , m_lx200Server(new Lx200Server(this))
    , m_catalogLoaded(false)
{
    setupUI();
    connectSignals();
    loadCatalog();
}

AstroNavigationPanel::~AstroNavigationPanel() = default;

void AstroNavigationPanel::setupUI() {
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    
    setupStarMapTab();
    setupStackingTab();
    setupSettingsTab();
    
    mainLayout->addWidget(m_tabWidget);
}

void AstroNavigationPanel::setupStarMapTab() {
    m_starMapTab = new QWidget();
    m_starMapTabLayout = new QVBoxLayout(m_starMapTab);

    m_starMapContent = new QWidget(m_starMapTab);
    auto *contentLayout = new QVBoxLayout(m_starMapContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(8);
    
    // Star map widget
    m_starMap = new StarMapWidget(m_starMapContent);
    m_starMap->setMinimumSize(400, 400);

    auto *topRowLayout = new QHBoxLayout();

    // Search input (integrated)
    auto *searchGroup = new QGroupBox(tr("Object Search"), m_starMapContent);
    auto *searchLayout = new QVBoxLayout(searchGroup);

    m_searchEdit = new QLineEdit(searchGroup);
    m_searchEdit->setPlaceholderText(tr("Search for stars, galaxies, nebulae..."));
    m_searchEdit->setClearButtonEnabled(true);

    m_searchStatusLabel = new QLabel(searchGroup);

    m_searchResults = new QListWidget(searchGroup);
    m_searchResults->setMaximumHeight(200);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchStatusLabel);
    searchLayout->addWidget(m_searchResults);

    // Visible objects tonight (integrated)
    auto *visibleGroup = new QGroupBox(tr("Visible Tonight"), m_starMapContent);
    auto *visibleLayout = new QVBoxLayout(visibleGroup);

    m_visibleObjectsList = new QListWidget(visibleGroup);

    auto *radecRow = new QWidget(visibleGroup);
    auto *radecRowLayout = new QGridLayout(radecRow);
    radecRowLayout->setContentsMargins(0, 0, 0, 0);
    radecRowLayout->setHorizontalSpacing(8);
    radecRowLayout->setVerticalSpacing(6);

    m_raInput = new QLineEdit(radecRow);
    m_raInput->setPlaceholderText(tr("RA (hh:mm:ss or deg)"));
    m_raInput->setClearButtonEnabled(true);

    m_decInput = new QLineEdit(radecRow);
    m_decInput->setPlaceholderText(tr("Dec (dd:mm:ss or deg)"));
    m_decInput->setClearButtonEnabled(true);

    m_radecGotoButton = new QPushButton(tr("GOTO"), radecRow);

    radecRowLayout->addWidget(m_raInput, 0, 0);
    radecRowLayout->addWidget(m_decInput, 0, 1);
    radecRowLayout->addWidget(m_radecGotoButton, 0, 2);
    radecRowLayout->setColumnStretch(0, 1);
    radecRowLayout->setColumnStretch(1, 1);

    auto *refreshButton = new QPushButton(tr("Refresh List"), visibleGroup);
    connect(refreshButton, &QPushButton::clicked, this,
            &AstroNavigationPanel::updateVisibleObjectsList);

    visibleLayout->addWidget(m_visibleObjectsList);
    visibleLayout->addWidget(refreshButton);
    visibleLayout->addWidget(radecRow);

    auto *rightPanel = new QWidget(m_starMapContent);
    rightPanel->setObjectName("astroStarMapSidePanel");
    rightPanel->setFixedWidth(320);
    auto *rightPanelLayout = new QVBoxLayout(rightPanel);
    rightPanelLayout->setContentsMargins(0, 0, 0, 0);
    rightPanelLayout->setSpacing(8);
    rightPanelLayout->addWidget(searchGroup);
    rightPanelLayout->addWidget(visibleGroup, 1);

    topRowLayout->addWidget(m_starMap, 1);
    topRowLayout->addWidget(rightPanel);
    
    // Info panel below map
    auto *infoGroup = new QGroupBox(tr("Selected Object"), m_starMapContent);
    auto *infoLayout = new QGridLayout(infoGroup);
    
    m_selectedObjectLabel = new QLabel(tr("No object selected"), infoGroup);
    m_selectedObjectLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    
    m_objectInfoLabel = new QLabel(infoGroup);
    m_objectInfoLabel->setWordWrap(true);
    
    m_gotoButton = new QPushButton(tr("GOTO"), infoGroup);
    m_gotoButton->setEnabled(false);
    m_gotoButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 10px; }");
    
    m_stopGotoButton = new QPushButton(tr("Stop"), infoGroup);
    m_stopGotoButton->setEnabled(false);
    m_stopGotoButton->setStyleSheet("QPushButton { background-color: #c0392b; color: white; font-weight: bold; padding: 10px; }");
    
    infoLayout->addWidget(m_selectedObjectLabel, 0, 0, 1, 2);
    infoLayout->addWidget(m_objectInfoLabel, 1, 0, 1, 2);

    auto *gotoRow = new QWidget(infoGroup);
    auto *gotoRowLayout = new QHBoxLayout(gotoRow);
    gotoRowLayout->setContentsMargins(0, 0, 0, 0);
    gotoRowLayout->setSpacing(12);
    gotoRowLayout->addStretch(1);
    m_gotoButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    m_stopGotoButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    gotoRowLayout->addWidget(m_gotoButton);
    gotoRowLayout->addWidget(m_stopGotoButton);
    gotoRowLayout->addStretch(1);

    infoLayout->addWidget(gotoRow, 2, 0, 1, 2);
    
    // Calibration controls
    auto *calibrationGroup = new QGroupBox(tr("Calibration"), m_starMapContent);
    auto *calibrationLayout = new QHBoxLayout(calibrationGroup);
    
    m_calibrateButton = new QPushButton(tr("Calibrate"), calibrationGroup);
    m_calibrateButton->setToolTip(tr("Manually start plate solving calibration.\n"
                                      "This is automatically done during GOTO,\n"
                                      "but can be triggered separately if needed."));
    
    m_cancelCalibrationButton = new QPushButton(tr("Stop"), calibrationGroup);
    m_cancelCalibrationButton->setEnabled(false);

    m_calibrationStatusLabel = new QLabel(tr("Not calibrated"), calibrationGroup);
    m_calibrationStatusLabel->setStyleSheet("color: gray;");
    
    calibrationLayout->addWidget(m_calibrateButton);
    calibrationLayout->addWidget(m_cancelCalibrationButton);
    calibrationLayout->addWidget(m_calibrationStatusLabel, 1);
    
    connect(m_calibrateButton, &QPushButton::clicked, this, &AstroNavigationPanel::onCalibrateClicked);
    connect(m_cancelCalibrationButton, &QPushButton::clicked, this, &AstroNavigationPanel::onCancelCalibrationClicked);

    // Place Selected Object + Calibration under Visible Tonight in the right column
    rightPanelLayout->addWidget(infoGroup);
    rightPanelLayout->addWidget(calibrationGroup);

    contentLayout->addLayout(topRowLayout, 1);

    m_starMapTabLayout->addWidget(m_starMapContent, 1);
    
    m_tabWidget->addTab(m_starMapTab, tr("Star Map"));
}

StarMapWidget *AstroNavigationPanel::starMapWidget() const {
    return m_starMap;
}

QWidget *AstroNavigationPanel::starMapTabWidget() const {
    return m_starMapTab;
}

QVBoxLayout *AstroNavigationPanel::starMapTabLayout() const {
    return m_starMapTabLayout;
}

QWidget *AstroNavigationPanel::starMapContentWidget() const {
    return m_starMapContent;
}

QTabWidget *AstroNavigationPanel::tabWidget() const {
    return m_tabWidget;
}

void AstroNavigationPanel::setCurrentTabIndex(int index) {
    if (m_tabWidget)
        m_tabWidget->setCurrentIndex(index);
}

int AstroNavigationPanel::currentTabIndex() const {
    return m_tabWidget ? m_tabWidget->currentIndex() : -1;
}

void AstroNavigationPanel::setupSearchTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    
    // Search input
    auto *searchGroup = new QGroupBox(tr("Object Search"), tab);
    auto *searchLayout = new QVBoxLayout(searchGroup);
    
    m_searchEdit = new QLineEdit(searchGroup);
    m_searchEdit->setPlaceholderText(tr("Search for stars, galaxies, nebulae..."));
    m_searchEdit->setClearButtonEnabled(true);
    
    m_searchStatusLabel = new QLabel(searchGroup);
    
    m_searchResults = new QListWidget(searchGroup);
    m_searchResults->setMaximumHeight(200);
    
    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchStatusLabel);
    searchLayout->addWidget(m_searchResults);
    
    // Visible objects tonight
    auto *visibleGroup = new QGroupBox(tr("Visible Tonight"), tab);
    auto *visibleLayout = new QVBoxLayout(visibleGroup);
    
    m_visibleObjectsList = new QListWidget(visibleGroup);
    
    auto *refreshButton = new QPushButton(tr("Refresh List"), visibleGroup);
    connect(refreshButton, &QPushButton::clicked, this, &AstroNavigationPanel::updateVisibleObjectsList);
    
    visibleLayout->addWidget(m_visibleObjectsList);
    visibleLayout->addWidget(refreshButton);
    
    layout->addWidget(searchGroup);
    layout->addWidget(visibleGroup, 1);
    
    m_tabWidget->addTab(tab, tr("Search"));
}

// Astro exposure values in microseconds (same as Tele camera for astro mode)
static const std::vector<int> s_astroExposureValues = {
    1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000,
    1000000, 2000000, 3000000, 4000000, 5000000, 6000000, 7000000, 8000000,
    9000000, 10000000, 11000000, 12000000, 13000000, 14000000, 15000000
};

// Astro gain values (0-120)
static const std::vector<int> s_astroGainValues = {
    0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120
};

QString AstroNavigationPanel::formatExposureValue(int sliderIndex) const {
    if (sliderIndex < 0 || sliderIndex >= static_cast<int>(s_astroExposureValues.size()))
        return "?";
    
    int us = s_astroExposureValues[sliderIndex];
    if (us < 1000) {
        return QString("%1µs").arg(us);
    } else if (us < 1000000) {
        return QString("%1ms").arg(us / 1000);
    } else {
        double secs = us / 1000000.0;
        return QString("%1s").arg(secs, 0, 'f', 1);
    }
}

QString AstroNavigationPanel::formatGainValue(int sliderIndex) const {
    if (sliderIndex < 0 || sliderIndex >= static_cast<int>(s_astroGainValues.size()))
        return "?";
    return QString::number(s_astroGainValues[sliderIndex]);
}

void AstroNavigationPanel::setupStackingTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    
    // Capture settings
    auto *settingsGroup = new QGroupBox(tr("Capture Settings"), tab);
    auto *settingsLayout = new QGridLayout(settingsGroup);
    
    // Number of frames
    settingsLayout->addWidget(new QLabel(tr("Frames:")), 0, 0);
    m_numFramesSpin = new QSpinBox(settingsGroup);
    m_numFramesSpin->setRange(1, 1000);
    m_numFramesSpin->setValue(100);
    settingsLayout->addWidget(m_numFramesSpin, 0, 1, 1, 2);
    
    // Exposure slider (like camera panel)
    settingsLayout->addWidget(new QLabel(tr("Exposure:")), 1, 0);
    m_astroExposureSlider = new QSlider(Qt::Horizontal, settingsGroup);
    m_astroExposureSlider->setRange(0, s_astroExposureValues.size() - 1);
    m_astroExposureSlider->setValue(14); // Default: 5s
    m_astroExposureValueLabel = new QLabel(formatExposureValue(14), settingsGroup);
    m_astroExposureValueLabel->setMinimumWidth(60);
    m_astroExposureValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    settingsLayout->addWidget(m_astroExposureSlider, 1, 1);
    settingsLayout->addWidget(m_astroExposureValueLabel, 1, 2);
    
    // Gain slider (like camera panel)
    settingsLayout->addWidget(new QLabel(tr("Gain:")), 2, 0);
    m_astroGainSlider = new QSlider(Qt::Horizontal, settingsGroup);
    m_astroGainSlider->setRange(0, s_astroGainValues.size() - 1);
    m_astroGainSlider->setValue(6); // Default: 60
    m_astroGainValueLabel = new QLabel(formatGainValue(6), settingsGroup);
    m_astroGainValueLabel->setMinimumWidth(40);
    m_astroGainValueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    settingsLayout->addWidget(m_astroGainSlider, 2, 1);
    settingsLayout->addWidget(m_astroGainValueLabel, 2, 2);
    
    // Connect sliders to update labels
    connect(m_astroExposureSlider, &QSlider::valueChanged, this, [this](int value) {
        m_astroExposureValueLabel->setText(formatExposureValue(value));
    });
    connect(m_astroGainSlider, &QSlider::valueChanged, this, [this](int value) {
        m_astroGainValueLabel->setText(formatGainValue(value));
    });
    
    // Control buttons
    auto *buttonLayout = new QHBoxLayout();
    
    m_startStackingButton = new QPushButton(tr("Start Stacking"), settingsGroup);
    m_startStackingButton->setStyleSheet("QPushButton { background-color: #27ae60; color: white; font-weight: bold; padding: 12px; }");
    
    m_stopStackingButton = new QPushButton(tr("Stop"), settingsGroup);
    m_stopStackingButton->setStyleSheet("QPushButton { background-color: #c0392b; color: white; font-weight: bold; padding: 12px; }");
    m_stopStackingButton->setEnabled(false);
    
    buttonLayout->addWidget(m_startStackingButton);
    buttonLayout->addWidget(m_stopStackingButton);
    settingsLayout->addLayout(buttonLayout, 3, 0, 1, 3);
    
    // Calibration group
    auto *calibrationGroup = new QGroupBox(tr("Calibration Frames"), tab);
    auto *calibrationLayout = new QGridLayout(calibrationGroup);
    
    // Dark frames
    calibrationLayout->addWidget(new QLabel(tr("Dark Frames:")), 0, 0);
    m_darkFramesSpin = new QSpinBox(calibrationGroup);
    m_darkFramesSpin->setRange(0, 100);
    m_darkFramesSpin->setValue(20);
    calibrationLayout->addWidget(m_darkFramesSpin, 0, 1);
    m_captureDarksButton = new QPushButton(tr("Capture Darks"), calibrationGroup);
    calibrationLayout->addWidget(m_captureDarksButton, 0, 2);
    
    // Flat frames
    calibrationLayout->addWidget(new QLabel(tr("Flat Frames:")), 1, 0);
    m_flatFramesSpin = new QSpinBox(calibrationGroup);
    m_flatFramesSpin->setRange(0, 100);
    m_flatFramesSpin->setValue(20);
    calibrationLayout->addWidget(m_flatFramesSpin, 1, 1);
    m_captureFlatsButton = new QPushButton(tr("Capture Flats"), calibrationGroup);
    calibrationLayout->addWidget(m_captureFlatsButton, 1, 2);
    
    // Bias frames
    calibrationLayout->addWidget(new QLabel(tr("Bias Frames:")), 2, 0);
    m_biasFramesSpin = new QSpinBox(calibrationGroup);
    m_biasFramesSpin->setRange(0, 100);
    m_biasFramesSpin->setValue(20);
    calibrationLayout->addWidget(m_biasFramesSpin, 2, 1);
    m_captureBiasButton = new QPushButton(tr("Capture Bias"), calibrationGroup);
    calibrationLayout->addWidget(m_captureBiasButton, 2, 2);
    
    m_calibrationFramesStatusLabel = new QLabel(tr("No calibration frames captured"), calibrationGroup);
    m_calibrationFramesStatusLabel->setStyleSheet("color: gray; font-style: italic;");
    calibrationLayout->addWidget(m_calibrationFramesStatusLabel, 3, 0, 1, 3);
    
    // Progress group
    auto *progressGroup = new QGroupBox(tr("Progress"), tab);
    auto *progressLayout = new QGridLayout(progressGroup);
    
    m_stackingProgress = new QProgressBar(progressGroup);
    m_stackingProgress->setRange(0, 100);
    m_stackingProgress->setValue(0);
    progressLayout->addWidget(m_stackingProgress, 0, 0, 1, 2);
    
    progressLayout->addWidget(new QLabel(tr("Status:")), 1, 0);
    m_stackingStatusLabel = new QLabel(tr("Idle"), progressGroup);
    progressLayout->addWidget(m_stackingStatusLabel, 1, 1);
    
    progressLayout->addWidget(new QLabel(tr("Frames:")), 2, 0);
    m_frameCountLabel = new QLabel("0 / 0", progressGroup);
    progressLayout->addWidget(m_frameCountLabel, 2, 1);
    
    progressLayout->addWidget(new QLabel(tr("Elapsed:")), 3, 0);
    m_elapsedTimeLabel = new QLabel("00:00:00", progressGroup);
    progressLayout->addWidget(m_elapsedTimeLabel, 3, 1);
    
    progressLayout->addWidget(new QLabel(tr("Rejected:")), 4, 0);
    m_rejectedFramesLabel = new QLabel("0", progressGroup);
    progressLayout->addWidget(m_rejectedFramesLabel, 4, 1);
    
    // Image settings (shared functionality with Camera panel)
    m_imageSettings = new ImageSettingsWidget(tab);
    
    // White balance settings (shared functionality with Camera panel)
    m_whiteBalance = new WhiteBalanceWidget(tab);
    
    layout->addWidget(settingsGroup);
    layout->addWidget(calibrationGroup);
    layout->addWidget(m_imageSettings);
    layout->addWidget(m_whiteBalance);
    layout->addWidget(progressGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, tr("Stacking"));
}

void AstroNavigationPanel::setupSettingsTab() {
    auto *tab = new QWidget();
    auto *layout = new QVBoxLayout(tab);
    
    // Display settings
    auto *displayGroup = new QGroupBox(tr("Display Settings"), tab);
    auto *displayLayout = new QGridLayout(displayGroup);
    
    displayLayout->addWidget(new QLabel(tr("Magnitude Limit:")), 0, 0);
    m_magnitudeLimitSpin = new QDoubleSpinBox(displayGroup);
    m_magnitudeLimitSpin->setRange(1.0, 12.0);
    m_magnitudeLimitSpin->setValue(6.0);
    m_magnitudeLimitSpin->setDecimals(1);
    displayLayout->addWidget(m_magnitudeLimitSpin, 0, 1);
    
    m_showConstellationsCheck = new QCheckBox(tr("Show Constellations"), displayGroup);
    m_showConstellationsCheck->setChecked(true);
    displayLayout->addWidget(m_showConstellationsCheck, 1, 0, 1, 2);
    
    m_showGridCheck = new QCheckBox(tr("Show Coordinate Grid"), displayGroup);
    m_showGridCheck->setChecked(false);
    displayLayout->addWidget(m_showGridCheck, 2, 0, 1, 2);
    
    m_showLabelsCheck = new QCheckBox(tr("Show Labels"), displayGroup);
    m_showLabelsCheck->setChecked(true);
    displayLayout->addWidget(m_showLabelsCheck, 3, 0, 1, 2);
    
    // Location settings
    auto *locationGroup = new QGroupBox(tr("Observer Location"), tab);
    auto *locationLayout = new QGridLayout(locationGroup);
    
    // Auto-detect button
    m_autoLocationButton = new QPushButton(tr("Auto-Detect Location"), locationGroup);
    m_autoLocationButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    locationLayout->addWidget(m_autoLocationButton, 0, 0, 1, 2);
    
    m_locationStatusLabel = new QLabel(tr("Location not set"), locationGroup);
    m_locationStatusLabel->setStyleSheet("color: gray; font-style: italic;");
    locationLayout->addWidget(m_locationStatusLabel, 1, 0, 1, 2);
    
    // Latitude
    locationLayout->addWidget(new QLabel(tr("Latitude:")), 2, 0);
    m_latitudeSpin = new QDoubleSpinBox(locationGroup);
    m_latitudeSpin->setRange(-90.0, 90.0);
    m_latitudeSpin->setValue(52.52);
    m_latitudeSpin->setDecimals(4);
    m_latitudeSpin->setSuffix("°");
    locationLayout->addWidget(m_latitudeSpin, 2, 1);
    
    // Longitude
    locationLayout->addWidget(new QLabel(tr("Longitude:")), 3, 0);
    m_longitudeSpin = new QDoubleSpinBox(locationGroup);
    m_longitudeSpin->setRange(-180.0, 180.0);
    m_longitudeSpin->setValue(13.405);
    m_longitudeSpin->setDecimals(4);
    m_longitudeSpin->setSuffix("°");
    locationLayout->addWidget(m_longitudeSpin, 3, 1);
    
    // Altitude
    locationLayout->addWidget(new QLabel(tr("Altitude:")), 4, 0);
    m_altitudeSpin = new QDoubleSpinBox(locationGroup);
    m_altitudeSpin->setRange(-500.0, 10000.0);
    m_altitudeSpin->setValue(0.0);
    m_altitudeSpin->setDecimals(0);
    m_altitudeSpin->setSuffix(" m");
    locationLayout->addWidget(m_altitudeSpin, 4, 1);
    
    // Apply button
    auto *applyLocationButton = new QPushButton(tr("Apply Location"), locationGroup);
    connect(applyLocationButton, &QPushButton::clicked, this, [this]() {
        m_starMap->setLocation(m_latitudeSpin->value(), m_longitudeSpin->value());
        m_locationStatusLabel->setText(tr("Location applied"));
        m_locationStatusLabel->setStyleSheet("color: green;");
    });
    locationLayout->addWidget(applyLocationButton, 5, 0, 1, 2);
    
    // Connect auto-location button
    connect(m_autoLocationButton, &QPushButton::clicked, this, &AstroNavigationPanel::onAutoLocationClicked);
    
    // LX200 Server settings
    auto *lx200Group = new QGroupBox(tr("LX200 Server"), tab);
    auto *lx200Layout = new QGridLayout(lx200Group);
    
    m_lx200EnableCheck = new QCheckBox(tr("Enable LX200 Server"), lx200Group);
    m_lx200EnableCheck->setToolTip(tr("Start a local TCP server for external apps like SkySafari or Stellarium"));
    lx200Layout->addWidget(m_lx200EnableCheck, 0, 0, 1, 2);
    
    lx200Layout->addWidget(new QLabel(tr("Port:")), 1, 0);
    m_lx200PortSpin = new QSpinBox(lx200Group);
    m_lx200PortSpin->setRange(1024, 65535);
    m_lx200PortSpin->setValue(4030);
    m_lx200PortSpin->setToolTip(tr("TCP port for LX200 connections (default: 4030)"));
    lx200Layout->addWidget(m_lx200PortSpin, 1, 1);
    
    m_lx200StatusLabel = new QLabel(tr("Server stopped"), lx200Group);
    m_lx200StatusLabel->setStyleSheet("color: gray;");
    lx200Layout->addWidget(m_lx200StatusLabel, 2, 0, 1, 2);
    
    connect(m_lx200EnableCheck, &QCheckBox::toggled, this, &AstroNavigationPanel::onLx200EnableToggled);
    connect(m_lx200PortSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &AstroNavigationPanel::onLx200PortChanged);
    
    layout->addWidget(displayGroup);
    layout->addWidget(locationGroup);
    layout->addWidget(lx200Group);
    layout->addStretch();
    
    m_tabWidget->addTab(tab, tr("Settings"));
}

void AstroNavigationPanel::connectSignals() {
    // Star map signals
    connect(m_starMap, &StarMapWidget::objectSelected, this, &AstroNavigationPanel::onObjectSelected);
    connect(m_starMap, &StarMapWidget::objectDoubleClicked, this, &AstroNavigationPanel::onObjectDoubleClicked);
    connect(m_starMap, &StarMapWidget::coordinatesClicked, this, &AstroNavigationPanel::onStarMapCoordinatesClicked);

    connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
        emit starMapOverlayRequested(index == 0);
    });
    connect(m_gotoButton, &QPushButton::clicked, this, &AstroNavigationPanel::onGotoClicked);
    connect(m_stopGotoButton, &QPushButton::clicked, this, &AstroNavigationPanel::onStopGotoClicked);

    if (m_radecGotoButton)
        connect(m_radecGotoButton, &QPushButton::clicked, this, &AstroNavigationPanel::onRaDecGotoClicked);
    
    // Search signals
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AstroNavigationPanel::onSearchTextChanged);
    connect(m_searchResults, &QListWidget::itemClicked, this, &AstroNavigationPanel::onSearchResultClicked);
    connect(m_searchResults, &QListWidget::itemDoubleClicked, this, &AstroNavigationPanel::onSearchResultDoubleClicked);
    connect(m_visibleObjectsList, &QListWidget::itemClicked, this, &AstroNavigationPanel::onSearchResultClicked);
    connect(m_visibleObjectsList, &QListWidget::itemDoubleClicked, this, &AstroNavigationPanel::onSearchResultDoubleClicked);
    
    // Stacking signals
    connect(m_startStackingButton, &QPushButton::clicked, this, &AstroNavigationPanel::onStartStackingClicked);
    connect(m_stopStackingButton, &QPushButton::clicked, this, &AstroNavigationPanel::onStopStackingClicked);
    connect(m_stackingTimer, &QTimer::timeout, this, &AstroNavigationPanel::updateStackingProgress);
    
    // Settings signals
    connect(m_magnitudeLimitSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &AstroNavigationPanel::onMagnitudeLimitChanged);
    connect(m_showConstellationsCheck, &QCheckBox::toggled, this, &AstroNavigationPanel::onShowConstellationsToggled);
    connect(m_showGridCheck, &QCheckBox::toggled, this, &AstroNavigationPanel::onShowGridToggled);
    connect(m_showLabelsCheck, &QCheckBox::toggled, this, &AstroNavigationPanel::onShowLabelsToggled);
    
    // LX200 server signals
    connect(m_lx200Server, &Lx200Server::runningChanged, this, [this](bool running) {
        if (running) {
            m_lx200StatusLabel->setText(tr("Listening on port %1").arg(m_lx200Server->listeningPort()));
            m_lx200StatusLabel->setStyleSheet("color: green;");
        } else {
            m_lx200StatusLabel->setText(tr("Server stopped"));
            m_lx200StatusLabel->setStyleSheet("color: gray;");
        }
        m_lx200PortSpin->setEnabled(!running);
    });
    
    connect(m_lx200Server, &Lx200Server::clientConnected, this, [this](const QHostAddress &, quint16) {
        if (m_lx200Server->isRunning()) {
            int count = m_lx200Server->clientCount();
            m_lx200StatusLabel->setText(tr("Port %1 - %2 client(s)").arg(m_lx200Server->listeningPort()).arg(count));
        }
    });
    
    connect(m_lx200Server, &Lx200Server::clientDisconnected, this, [this](const QHostAddress &, quint16) {
        if (m_lx200Server->isRunning()) {
            int count = m_lx200Server->clientCount();
            if (count > 0) {
                m_lx200StatusLabel->setText(tr("Port %1 - %2 client(s)").arg(m_lx200Server->listeningPort()).arg(count));
            } else {
                m_lx200StatusLabel->setText(tr("Listening on port %1").arg(m_lx200Server->listeningPort()));
            }
        }
    });
    
    connect(m_lx200Server, &Lx200Server::errorOccurred, this, [this](const QString &error) {
        m_lx200StatusLabel->setText(tr("Error: %1").arg(error));
        m_lx200StatusLabel->setStyleSheet("color: red;");
        m_lx200EnableCheck->setChecked(false);
    });
    
    connect(m_lx200Server, &Lx200Server::gotoRequested, this, [this](double raDeg, double decDeg) {
        qDebug() << "[LX200] GOTO requested: RA" << raDeg << "Dec" << decDeg;
        emit gotoRequested(raDeg, decDeg);
        
        if (m_astroController) {
            QString targetName = QString("LX200 Target RA:%1 Dec:%2")
                .arg(raDeg / 15.0, 0, 'f', 2)
                .arg(decDeg, 0, 'f', 2);
            m_astroController->oneClickGotoDSO(raDeg, decDeg, targetName);
        }
    });
    
    connect(m_lx200Server, &Lx200Server::stopRequested, this, [this]() {
        qDebug() << "[LX200] Stop requested";
        if (m_astroController) {
            m_astroController->stopGoto();
        }
    });
}

void AstroNavigationPanel::onStarMapCoordinatesClicked(double ra, double dec) {
    if (!m_raInput || !m_decInput)
        return;

    const QSignalBlocker raBlocker(m_raInput);
    const QSignalBlocker decBlocker(m_decInput);

    m_raInput->setText(QString::number(ra, 'f', 4));
    m_decInput->setText(QString::number(dec, 'f', 4));
}

static bool parseSexagesimalAngle(const QString &text, double *outDegrees, bool hoursToDegrees) {
    if (!outDegrees)
        return false;

    QString s = text.trimmed();
    if (s.isEmpty())
        return false;

    s.replace(',', '.');
    s.replace('h', ':');
    s.replace('m', ':');
    s.replace('s', ' ');
    s.replace(QChar(0x00B0), ':');
    s.replace(QChar(u'′'), QChar(':'));
    s.replace(QChar(u'’'), QChar(':'));
    s.replace(QChar(u'″'), QChar(' '));
    s.replace('"', ' ');
    s = s.simplified();

    const QRegularExpression re(R"(^\s*([+-])?\s*(\d+(?:\.\d+)?)\s*(?:(?:\:|\s)\s*(\d+(?:\.\d+)?))?\s*(?:(?:\:|\s)\s*(\d+(?:\.\d+)?))?\s*$)");
    const QRegularExpressionMatch m = re.match(s);
    if (!m.hasMatch())
        return false;

    const bool negative = (m.captured(1) == "-");
    const double a = m.captured(2).toDouble();
    const double b = m.captured(3).isEmpty() ? 0.0 : m.captured(3).toDouble();
    const double c = m.captured(4).isEmpty() ? 0.0 : m.captured(4).toDouble();
    double value = a + (b / 60.0) + (c / 3600.0);
    if (negative)
        value = -value;

    if (hoursToDegrees)
        value *= 15.0;

    *outDegrees = value;
    return true;
}

static bool parseRaDegrees(const QString &text, double *outRaDeg) {
    if (!outRaDeg)
        return false;

    QString s = text.trimmed();
    if (s.isEmpty())
        return false;

    double deg = 0.0;
    if (s.contains(':') || s.contains('h') || s.contains('m') || s.contains('s')) {
        if (!parseSexagesimalAngle(s, &deg, true))
            return false;
    } else {
        QString n = s;
        n.replace(',', '.');
        bool ok = false;
        const double v = n.toDouble(&ok);
        if (!ok)
            return false;
        deg = (v <= 24.0) ? (v * 15.0) : v;
    }

    deg = std::fmod(deg, 360.0);
    if (deg < 0)
        deg += 360.0;
    *outRaDeg = deg;
    return true;
}

static bool parseDecDegrees(const QString &text, double *outDecDeg) {
    if (!outDecDeg)
        return false;

    QString s = text.trimmed();
    if (s.isEmpty())
        return false;

    double deg = 0.0;
    if (s.contains(':') || s.contains(QChar(0x00B0)) || s.contains(QChar(u'′')) || s.contains('"')) {
        if (!parseSexagesimalAngle(s, &deg, false))
            return false;
    } else {
        QString n = s;
        n.replace(',', '.');
        bool ok = false;
        deg = n.toDouble(&ok);
        if (!ok)
            return false;
    }

    if (deg < -90.0 || deg > 90.0)
        return false;

    *outDecDeg = deg;
    return true;
}

void AstroNavigationPanel::onRaDecGotoClicked() {
    if (!m_raInput || !m_decInput)
        return;

    double raDeg = 0.0;
    double decDeg = 0.0;
    if (!parseRaDegrees(m_raInput->text(), &raDeg) || !parseDecDegrees(m_decInput->text(), &decDeg)) {
        QMessageBox::warning(this, tr("Invalid coordinates"),
                             tr("Please enter valid coordinates.\nRA: hh:mm:ss or degrees\nDec: dd:mm:ss or degrees"));
        return;
    }

    CelestialObject obj;
    obj.ra = raDeg;
    obj.dec = decDeg;
    obj.name = QString("Custom RA:%1 Dec:%2").arg(raDeg, 0, 'f', 4).arg(decDeg, 0, 'f', 4);
    obj.commonName = obj.name;
    obj.isVisible = true;
    gotoObject(obj);
}

void AstroNavigationPanel::loadCatalog() {
    // Try to find the star catalog database
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/../data/stars.db",
        QCoreApplication::applicationDirPath() + "/data/stars.db",
        QDir::currentPath() + "/data/stars.db",
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/stars.db",
        "/usr/share/zwergii/stars.db"
    };
    
    for (const QString &path : searchPaths) {
        qDebug() << "Checking for star catalog at:" << path;
        if (QFile::exists(path)) {
            if (m_starMap->loadCatalog(path)) {
                qDebug() << "Loaded star catalog from:" << path;
                m_catalogLoaded = true;

                if (m_searchEdit)
                    m_searchEdit->setEnabled(true);
                if (m_searchResults)
                    m_searchResults->setEnabled(true);
                if (m_visibleObjectsList)
                    m_visibleObjectsList->setEnabled(true);
                if (m_searchStatusLabel)
                    m_searchStatusLabel->setText("");
                return;
            }
        }
    }
    
    qWarning() << "Star catalog not found. Star map will be empty.";
    qWarning() << "Searched paths:" << searchPaths;

    m_catalogLoaded = false;
    if (m_searchStatusLabel)
        m_searchStatusLabel->setText(tr("Star catalog not found. Search disabled."));
    if (m_searchEdit)
        m_searchEdit->setEnabled(false);
    if (m_searchResults)
        m_searchResults->setEnabled(false);
    if (m_visibleObjectsList)
        m_visibleObjectsList->setEnabled(false);
}

void AstroNavigationPanel::setWebSocketClient(DwarfWebSocketClient *client) {
    m_wsClient = client;
}

void AstroNavigationPanel::setCameraController(DwarfCameraController *controller) {
    m_cameraController = controller;
    
    // Pass controller to shared widgets
    if (m_imageSettings) {
        m_imageSettings->setCameraController(controller);
    }
    if (m_whiteBalance) {
        m_whiteBalance->setCameraController(controller);
    }
}

void AstroNavigationPanel::setAstroController(DwarfAstroController *controller) {
    m_astroController = controller;
    
    if (m_astroController) {
        // Connect GOTO signals
        connect(m_astroController, &DwarfAstroController::gotoStarted, this, [this](const QString &name) {
            m_gotoButton->setEnabled(false);
            m_gotoButton->setText(tr("Going to %1...").arg(name));
            m_stopGotoButton->setEnabled(true);
        });
        connect(m_astroController, &DwarfAstroController::gotoProgress, this, [this](int step) {
            // Steps: 10=calibrating, 20=slewing, 30=centering, 40=tracking
            QString stepName;
            switch (step) {
                case 10: stepName = tr("Calibrating..."); break;
                case 20: stepName = tr("Slewing..."); break;
                case 30: stepName = tr("Centering..."); break;
                case 40: stepName = tr("Tracking..."); break;
                default: stepName = tr("Step %1...").arg(step);
            }
            m_gotoButton->setText(stepName);
        });
        connect(m_astroController, &DwarfAstroController::gotoCompleted, this, [this]() {
            m_gotoButton->setEnabled(true);
            m_gotoButton->setText(tr("GOTO"));
            m_stopGotoButton->setEnabled(false);
        });
        connect(m_astroController, &DwarfAstroController::gotoFailed, this, [this](const QString &error) {
            m_gotoButton->setEnabled(true);
            m_gotoButton->setText(tr("GOTO"));
            m_stopGotoButton->setEnabled(false);
            QMessageBox::warning(this, tr("GOTO Failed"), error);
        });
        
        // Connect calibration signals
        connect(m_astroController, &DwarfAstroController::calibrationStarted, this, [this]() {
            m_calibrateButton->setEnabled(false);
            m_calibrateButton->setText(tr("Calibrating..."));
            if (m_cancelCalibrationButton)
                m_cancelCalibrationButton->setEnabled(true);
            m_calibrationStatusLabel->setText(tr("Plate solving..."));
            m_calibrationStatusLabel->setStyleSheet("color: blue;");
        });
        connect(m_astroController, &DwarfAstroController::calibrationCompleted, this, [this](bool success) {
            m_calibrateButton->setEnabled(true);
            m_calibrateButton->setText(tr("Calibrate"));
            if (m_cancelCalibrationButton)
                m_cancelCalibrationButton->setEnabled(false);
            if (success) {
                m_calibrationStatusLabel->setText(tr("Calibrated successfully"));
                m_calibrationStatusLabel->setStyleSheet("color: green;");
            } else {
                m_calibrationStatusLabel->setText(tr("Calibration failed"));
                m_calibrationStatusLabel->setStyleSheet("color: red;");
            }
        });
        connect(m_astroController, &DwarfAstroController::calibrationFailed, this, [this](const QString &error) {
            m_calibrateButton->setEnabled(true);
            m_calibrateButton->setText(tr("Calibrate"));
            if (m_cancelCalibrationButton)
                m_cancelCalibrationButton->setEnabled(false);
            m_calibrationStatusLabel->setText(tr("Failed: %1").arg(error));
            m_calibrationStatusLabel->setStyleSheet("color: red;");
        });
        
        // Connect stacking progress signals
        connect(m_astroController, &DwarfAstroController::stackingProgress, this, 
                [this](int currentFrame, int totalFrames, int stackedFrames, int rejectedFrames) {
            m_frameCountLabel->setText(QString("%1 / %2").arg(stackedFrames).arg(totalFrames > 0 ? totalFrames : currentFrame));
            m_rejectedFramesLabel->setText(QString::number(rejectedFrames));
            
            if (totalFrames > 0) {
                m_stackingProgress->setRange(0, totalFrames);
                m_stackingProgress->setValue(stackedFrames);
            } else {
                // Unlimited mode - just show current count
                m_stackingProgress->setRange(0, 0);  // Indeterminate
            }
            
            m_stackingStatusLabel->setText(tr("Capturing frame %1...").arg(currentFrame));
        });
        
        connect(m_astroController, &DwarfAstroController::stackingStateChanged, this, [this](int state) {
            switch (state) {
                case 0:
                    m_stackingStatusLabel->setText(tr("Idle"));
                    m_isStacking = false;
                    m_startStackingButton->setEnabled(true);
                    m_stopStackingButton->setEnabled(false);
                    break;
                case 1:
                    m_stackingStatusLabel->setText(tr("Capturing..."));
                    break;
                case 2:
                    m_stackingStatusLabel->setText(tr("Stacking..."));
                    break;
            }
        });
        
        connect(m_astroController, &DwarfAstroController::stackingFailed, this, [this](const QString &error) {
            m_stackingStatusLabel->setText(tr("Failed: %1").arg(error));
            m_isStacking = false;
            m_stackingTimer->stop();
            m_startStackingButton->setEnabled(true);
            m_stopStackingButton->setEnabled(false);
            m_numFramesSpin->setEnabled(true);
            m_astroExposureSlider->setEnabled(true);
            m_astroGainSlider->setEnabled(true);
            
            // Show error in message box
            QMessageBox::warning(this, tr("Stacking Failed"), error);
        });
    }
}

double AstroNavigationPanel::latitude() const {
    return m_latitudeSpin ? m_latitudeSpin->value() : 0.0;
}

double AstroNavigationPanel::longitude() const {
    return m_longitudeSpin ? m_longitudeSpin->value() : 0.0;
}

double AstroNavigationPanel::altitude() const {
    return m_altitudeSpin ? m_altitudeSpin->value() : 0.0;
}

void AstroNavigationPanel::setLocation(double latitude, double longitude) {
    m_latitudeSpin->setValue(latitude);
    m_longitudeSpin->setValue(longitude);
    m_starMap->setLocation(latitude, longitude);
}

void AstroNavigationPanel::setTelescopePointing(double ra, double dec) {
    m_starMap->setTelescopePointing(ra, dec);
    m_lx200Server->setCurrentPosition(ra, dec);
}

void AstroNavigationPanel::onObjectSelected(const CelestialObject &obj) {
    m_selectedObject = obj;
    
    QString name = obj.commonName.isEmpty() ? obj.name : obj.commonName;
    m_selectedObjectLabel->setText(name);
    
    QString info;
    info += QString("Type: %1\n").arg(obj.type);
    info += QString("RA: %1°  Dec: %2°\n").arg(obj.ra, 0, 'f', 2).arg(obj.dec, 0, 'f', 2);
    info += QString("Magnitude: %1\n").arg(obj.magnitude, 0, 'f', 1);
    if (obj.isVisible) {
        info += QString("Altitude: %1°  Azimuth: %2°\n").arg(obj.altitude, 0, 'f', 1).arg(obj.azimuth, 0, 'f', 1);
    } else {
        info += "Currently below horizon";
    }
    if (!obj.constellation.isEmpty()) {
        info += QString("Constellation: %1").arg(obj.constellation);
    }
    
    m_objectInfoLabel->setText(info);
    m_gotoButton->setEnabled(obj.isVisible);
}

void AstroNavigationPanel::onObjectDoubleClicked(const CelestialObject &obj) {
    if (obj.isVisible) {
        gotoObject(obj);
    }
}

void AstroNavigationPanel::onGotoClicked() {
    if (m_selectedObject.isVisible) {
        gotoObject(m_selectedObject);
    }
}

void AstroNavigationPanel::onStopGotoClicked() {
    if (m_astroController) {
        qDebug() << "Stopping GOTO...";
        m_astroController->stopGoto();
        m_gotoButton->setEnabled(true);
        m_gotoButton->setText(tr("GOTO"));
        m_stopGotoButton->setEnabled(false);
    }
}

void AstroNavigationPanel::onCalibrateClicked() {
    if (!m_astroController) {
        qWarning() << "No astro controller set, cannot calibrate";
        return;
    }
    
    m_calibrateButton->setEnabled(false);
    m_calibrateButton->setText(tr("Calibrating..."));
    if (m_cancelCalibrationButton)
        m_cancelCalibrationButton->setEnabled(true);
    m_calibrationStatusLabel->setText(tr("Plate solving in progress..."));
    m_calibrationStatusLabel->setStyleSheet("color: blue;");
    
    m_astroController->startCalibration();
}

void AstroNavigationPanel::onCancelCalibrationClicked() {
    if (!m_astroController)
        return;

    qDebug() << "Stopping calibration...";
    m_astroController->stopCalibration();
    if (m_cancelCalibrationButton)
        m_cancelCalibrationButton->setEnabled(false);
    // Status will be updated by calibrationCompleted/calibrationFailed
}

void AstroNavigationPanel::gotoObject(const CelestialObject &obj) {
    qDebug() << "GOTO:" << obj.name << "RA:" << obj.ra << "Dec:" << obj.dec;
    emit gotoRequested(obj.ra, obj.dec);
    
    if (!m_astroController) {
        qWarning() << "No astro controller set, cannot perform GOTO";
        return;
    }
    
    // Determine object type and send appropriate command
    QString displayName = obj.commonName.isEmpty() ? obj.name : obj.commonName;
    
    // Check if it's a solar system object (planets, sun, moon)
    // Solar system indices: 1=Mercury, 2=Venus, 3=Mars, 4=Jupiter, 5=Saturn, 6=Uranus, 7=Neptune, 8=Moon, 9=Sun
    static const QMap<QString, int> solarSystemObjects = {
        {"Sun", 9}, {"Moon", 8}, {"Mercury", 1}, {"Venus", 2}, {"Mars", 3},
        {"Jupiter", 4}, {"Saturn", 5}, {"Uranus", 6}, {"Neptune", 7}
    };
    
    if (solarSystemObjects.contains(obj.name)) {
        // Solar system object - needs GPS coordinates
        int index = solarSystemObjects[obj.name];
        m_astroController->oneClickGotoSolarSystem(index, longitude(), latitude(), displayName);
    } else {
        // Deep sky object - only needs RA/Dec
        m_astroController->oneClickGotoDSO(obj.ra, obj.dec, displayName);
    }
}

void AstroNavigationPanel::onSearchTextChanged(const QString &text) {
    m_searchResults->clear();

    if (!m_catalogLoaded) {
        m_searchStatusLabel->setText(tr("Star catalog not found. Search disabled."));
        return;
    }
    
    if (text.length() < 2) {
        m_searchStatusLabel->setText("");
        return;
    }
    
    auto results = m_starMap->searchObjects(text, 20);
    m_searchStatusLabel->setText(tr("Found %1 objects").arg(results.size()));
    
    for (const auto &obj : results) {
        QString displayName = obj.commonName.isEmpty() ? obj.name : obj.commonName;
        QString itemText = QString("%1 (mag %2)").arg(displayName).arg(obj.magnitude, 0, 'f', 1);
        
        auto *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, obj.ra);
        item->setData(Qt::UserRole + 1, obj.dec);
        item->setData(Qt::UserRole + 2, obj.name);
        m_searchResults->addItem(item);
    }
}

void AstroNavigationPanel::onSearchResultClicked(QListWidgetItem *item) {
    QString name = item->data(Qt::UserRole + 2).toString();
    m_starMap->selectObjectByName(name);
}

void AstroNavigationPanel::onSearchResultDoubleClicked(QListWidgetItem *item) {
    QString name = item->data(Qt::UserRole + 2).toString();
    
    // First select the object on the map
    m_starMap->selectObjectByName(name);
    
    // Then GOTO if the object is visible
    if (m_selectedObject.isVisible) {
        gotoObject(m_selectedObject);
    } else {
        // Object not in our catalog with full info, create a minimal one
        CelestialObject obj;
        obj.ra = item->data(Qt::UserRole).toDouble();
        obj.dec = item->data(Qt::UserRole + 1).toDouble();
        obj.name = name;
        obj.isVisible = true;
        gotoObject(obj);
    }
}

void AstroNavigationPanel::updateVisibleObjectsList() {
    m_visibleObjectsList->clear();
    
    auto visible = m_starMap->getVisibleObjects(15.0); // Min 15° altitude
    
    // Sort by magnitude (brightest first)
    std::sort(visible.begin(), visible.end(), [](const CelestialObject &a, const CelestialObject &b) {
        return a.magnitude < b.magnitude;
    });
    
    // Limit to top 50
    int count = qMin(visible.size(), 50);
    for (int i = 0; i < count; ++i) {
        const auto &obj = visible[i];
        QString displayName = obj.commonName.isEmpty() ? obj.name : obj.commonName;
        QString itemText = QString("%1 - Alt: %2° (mag %3)")
            .arg(displayName)
            .arg(obj.altitude, 0, 'f', 0)
            .arg(obj.magnitude, 0, 'f', 1);
        
        auto *item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, obj.ra);
        item->setData(Qt::UserRole + 1, obj.dec);
        item->setData(Qt::UserRole + 2, obj.name);
        m_visibleObjectsList->addItem(item);
    }
}

void AstroNavigationPanel::onStartStackingClicked() {
    if (!m_astroController) {
        qWarning() << "No AstroController set!";
        return;
    }
    if (!m_cameraController) {
        qWarning() << "No CameraController set!";
        return;
    }
    
    m_isStacking = true;
    m_currentFrame = 0;
    m_totalFrames = m_numFramesSpin->value();
    m_rejectedFrames = 0;
    
    m_startStackingButton->setEnabled(false);
    m_stopStackingButton->setEnabled(true);
    m_numFramesSpin->setEnabled(false);
    m_astroExposureSlider->setEnabled(false);
    m_astroGainSlider->setEnabled(false);
    
    m_stackingProgress->setRange(0, m_totalFrames);
    m_stackingProgress->setValue(0);
    m_stackingStatusLabel->setText(tr("Starting..."));
    
    m_stackingElapsed.start();
    m_stackingTimer->start(100); // Update UI every 100ms
    
    int exposureIndex = m_astroExposureSlider->value();
    int gainIndex = m_astroGainSlider->value();
    
    // STEP 1: Set camera parameters BEFORE starting stacking
    // The DWARF II requires exposure/gain to be set via camera API first
    qDebug() << "Setting camera params for stacking: exposure index=" << exposureIndex 
             << "gain index=" << gainIndex;
    m_stackingStatusLabel->setText(tr("Preparing camera..."));
    m_cameraController->setExposureMode(DwarfCameraController::CameraKind::Tele, 1);  // Manual mode
    m_cameraController->setExposureIndex(DwarfCameraController::CameraKind::Tele, exposureIndex);
    m_cameraController->setGainMode(DwarfCameraController::CameraKind::Tele, 1);  // Manual mode
    m_cameraController->setGainIndex(DwarfCameraController::CameraKind::Tele, gainIndex);
    
    emit stackingStarted(m_totalFrames, exposureIndex);
    
    // STEP 2: Wait 300ms for camera params to apply, then activate Astro mode
    QTimer::singleShot(300, this, [this]() {
        if (!m_isStacking) return; // User cancelled
        
        m_stackingStatusLabel->setText(tr("Activating Astro mode..."));
        qDebug() << "=== STEP 2: Activating Astro mode (Go Live)...";
        m_astroController->goLive();
        
        // STEP 3: Wait 1500ms for Go Live to activate, then start stacking
        QTimer::singleShot(1500, this, [this]() {
            if (!m_isStacking) return; // User cancelled
            
            m_stackingStatusLabel->setText(tr("Starting stacking..."));
            qDebug() << "=== STEP 3: Starting live stacking:" << m_totalFrames << "frames";
            m_astroController->startLiveStacking();
        });
    });
}

void AstroNavigationPanel::onStopStackingClicked() {
    m_isStacking = false;
    m_stackingTimer->stop();
    
    m_startStackingButton->setEnabled(true);
    m_stopStackingButton->setEnabled(false);
    m_numFramesSpin->setEnabled(true);
    m_astroExposureSlider->setEnabled(true);
    m_astroGainSlider->setEnabled(true);
    
    m_stackingStatusLabel->setText(tr("Stopped"));
    
    // Stop stacking via AstroController
    if (m_astroController) {
        m_astroController->stopLiveStacking();
    }
    
    emit stackingStopped();
}

void AstroNavigationPanel::updateStackingProgress() {
    if (!m_isStacking) return;
    
    // Update elapsed time
    qint64 elapsed = m_stackingElapsed.elapsed();
    int hours = elapsed / 3600000;
    int mins = (elapsed % 3600000) / 60000;
    int secs = (elapsed % 60000) / 1000;
    m_elapsedTimeLabel->setText(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(mins, 2, 10, QChar('0'))
        .arg(secs, 2, 10, QChar('0')));
    
    // Simulate progress for now (TODO: get actual progress from camera)
    // In real implementation, this would be updated by camera callbacks
    m_frameCountLabel->setText(QString("%1 / %2").arg(m_currentFrame).arg(m_totalFrames));
    m_rejectedFramesLabel->setText(QString::number(m_rejectedFrames));
    m_stackingProgress->setValue(m_currentFrame);
    
    if (m_currentFrame >= m_totalFrames) {
        m_stackingStatusLabel->setText(tr("Complete!"));
        onStopStackingClicked();
    } else {
        m_stackingStatusLabel->setText(tr("Capturing frame %1...").arg(m_currentFrame + 1));
    }
}

void AstroNavigationPanel::onMagnitudeLimitChanged(double value) {
    m_starMap->setMagnitudeLimit(value);
}

void AstroNavigationPanel::onShowConstellationsToggled(bool checked) {
    m_starMap->setShowConstellations(checked);
}

void AstroNavigationPanel::onShowGridToggled(bool checked) {
    m_starMap->setShowGrid(checked);
}

void AstroNavigationPanel::onShowLabelsToggled(bool checked) {
    m_starMap->setShowLabels(checked);
}

void AstroNavigationPanel::onAutoLocationClicked() {
    m_autoLocationButton->setEnabled(false);
    m_locationStatusLabel->setText(tr("Detecting location..."));
    m_locationStatusLabel->setStyleSheet("color: blue;");
    
    // Use ip-api.com for GeoIP location (free, no API key required)
    auto *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, [this, manager](QNetworkReply *reply) {
        manager->deleteLater();
        m_autoLocationButton->setEnabled(true);
        
        if (reply->error() != QNetworkReply::NoError) {
            m_locationStatusLabel->setText(tr("Failed: %1").arg(reply->errorString()));
            m_locationStatusLabel->setStyleSheet("color: red;");
            reply->deleteLater();
            return;
        }
        
        QByteArray data = reply->readAll();
        reply->deleteLater();
        
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            m_locationStatusLabel->setText(tr("Invalid response"));
            m_locationStatusLabel->setStyleSheet("color: red;");
            return;
        }
        
        onLocationReceived(doc.object());
    });
    
    // ip-api.com returns: lat, lon, city, country, etc.
    QNetworkRequest request(QUrl("http://ip-api.com/json/?fields=status,message,lat,lon,city,country"));
    request.setHeader(QNetworkRequest::UserAgentHeader, "zwergii/1.0");
    manager->get(request);
}

void AstroNavigationPanel::onLocationReceived(const QJsonObject &location) {
    QString status = location["status"].toString();
    
    if (status != "success") {
        QString message = location["message"].toString();
        m_locationStatusLabel->setText(tr("Failed: %1").arg(message));
        m_locationStatusLabel->setStyleSheet("color: red;");
        return;
    }
    
    double lat = location["lat"].toDouble();
    double lon = location["lon"].toDouble();
    QString city = location["city"].toString();
    QString country = location["country"].toString();
    
    // Update spin boxes
    m_latitudeSpin->setValue(lat);
    m_longitudeSpin->setValue(lon);
    
    // Apply to star map
    m_starMap->setLocation(lat, lon);
    
    // Update status
    QString locationStr = city.isEmpty() ? country : QString("%1, %2").arg(city, country);
    m_locationStatusLabel->setText(tr("Detected: %1").arg(locationStr));
    m_locationStatusLabel->setStyleSheet("color: green;");
    
    qDebug() << "Auto-detected location:" << lat << lon << "-" << locationStr;
}

void AstroNavigationPanel::onLx200EnableToggled(bool enabled) {
    if (enabled) {
        quint16 port = static_cast<quint16>(m_lx200PortSpin->value());
        m_lx200Server->setLocation(m_latitudeSpin->value(), m_longitudeSpin->value());
        if (!m_lx200Server->start(port)) {
            m_lx200EnableCheck->setChecked(false);
        }
    } else {
        m_lx200Server->stop();
    }
}

void AstroNavigationPanel::onLx200PortChanged(int port) {
    Q_UNUSED(port);
    // Port change only takes effect on next server start
    // If server is running, user needs to restart it
    if (m_lx200Server->isRunning()) {
        m_lx200StatusLabel->setText(tr("Restart server to apply new port"));
        m_lx200StatusLabel->setStyleSheet("color: orange;");
    }
}
