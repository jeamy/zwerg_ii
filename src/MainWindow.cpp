#include "MainWindow.h"
#include "net/DwarfAstroController.h"
#include "net/DwarfCameraController.h"
#include "net/DwarfFocusController.h"
#include "net/DwarfFinder.h"
#include "net/DwarfMessageDispatcher.h"
#include "net/DwarfWebSocketClient.h"
#include "net/DwarfPanoramaController.h"
#include "net/DwarfHttpClient.h"
#include "net/DwarfFtpDownloader.h"
#include "net/DwarfMtpClient.h"
#include "net/DwarfMjpegStream.h"
#include "net/DwarfMjpegView.h"
#include "net/DwarfMotorController.h"
#include "ui/AstroNavigationPanel.h"
#include "ui/CameraSettingsPanel.h"
#include "ui/MotorControlPanel.h"
#include "ui/ParametersOverlayPanel.h"
#include "ui/MediaLightbox.h"
#include "ui/StarMapWidget.h"
#include <QButtonGroup>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QListView>
#include <QMessageBox>
#include <QPixmap>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QVector>
#include <cmath>

#include "system.pb.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_wsClient(nullptr), m_dispatcher(nullptr),
      m_scanCancelled(false), m_cameraController(nullptr),
      m_motorController(nullptr), m_focusController(nullptr),
      m_mainVideoWidget(nullptr), m_pipVideoWidget(nullptr),
      m_cameraSettingsPanel(nullptr), m_teleStream(nullptr),
      m_wideStream(nullptr), m_recordTimer(nullptr), m_httpClient(nullptr),
      m_openGalleryButton(nullptr), m_mediaTabs(nullptr),
      m_mediaPhotoList(nullptr), m_mediaVideoList(nullptr),
      m_mediaBurstList(nullptr), m_mediaAstroList(nullptr),
      m_mediaPanoList(nullptr), m_downloadDirEdit(nullptr),
      m_changeDownloadDirButton(nullptr), m_ftpDownloader(nullptr),
      m_mtpClient(nullptr),
      m_thumbnailsLoading(0) {
  m_mainStreamView = nullptr;
  m_pipContainer = nullptr;

  m_recordTimer = new QTimer(this);
  m_recordTimer->setInterval(500);
  m_mainStream = CameraStream::Tele;
  m_pipStream = CameraStream::Wide;
  m_cameraController = new DwarfCameraController(this);
  m_motorController = new DwarfMotorController(this);
  m_focusController = new DwarfFocusController(this);
  m_teleStream = new DwarfMjpegStream(this);
  m_wideStream = new DwarfMjpegStream(this);
  m_astroController = new DwarfAstroController(this);
  m_panoramaController = new DwarfPanoramaController(this);
  m_finder = new DwarfFinder(this);
  connect(m_finder, &DwarfFinder::deviceFound, this,
          &MainWindow::onDeviceFound);
  connect(m_finder, &DwarfFinder::scanFinished, this,
          &MainWindow::onScanFinished);
  connect(m_finder, &DwarfFinder::scanProgress, this,
          &MainWindow::onScanProgress);

  m_dispatcher = new DwarfMessageDispatcher(this);
  connect(m_dispatcher, &DwarfMessageDispatcher::cameraTeleMessage, this,
          &MainWindow::onCameraTeleMessage);
  connect(m_dispatcher, &DwarfMessageDispatcher::cameraWideMessage, this,
          &MainWindow::onCameraWideMessage);

  if (m_astroController) {
    connect(m_dispatcher, &DwarfMessageDispatcher::astroMessage, m_astroController,
            &DwarfAstroController::handleAstroMessage);
    connect(m_dispatcher, &DwarfMessageDispatcher::notifyMessage, m_astroController,
            &DwarfAstroController::handleNotification);
  }

  if (m_cameraController) {
    connect(m_cameraController, &DwarfCameraController::photoCaptureFinished,
            this, &MainWindow::onPhotoCaptureFinished);
    connect(m_cameraController, &DwarfCameraController::recordFinished, this,
            &MainWindow::onRecordFinished);
  }

  if (m_panoramaController) {
    connect(m_dispatcher, &DwarfMessageDispatcher::panoramaMessage, m_panoramaController,
            &DwarfPanoramaController::handlePanoramaMessage);
  }

  setupUi();
}

void MainWindow::ensureHttpClientForCurrentIp() {
  QString ip = m_ipInput ? m_ipInput->text().trimmed() : QString();
  if (ip.isEmpty())
    return;

  if (m_httpClient) {
    return;
  }

  m_httpClient = new DwarfHttpClient(ip, this);
  connect(m_httpClient, &DwarfHttpClient::mediaListReceived, this,
          &MainWindow::onMediaListReceived);
  connect(m_httpClient, &DwarfHttpClient::errorOccurred, this,
          &MainWindow::onMediaListError);
}

void MainWindow::setCaptureStatusTextAllPanels(const QString &text) {
  if (m_cameraSettingsPanel)
    m_cameraSettingsPanel->setCaptureStatusText(text);
  if (m_paramsOverlay)
    m_paramsOverlay->setCaptureStatusText(text);
}

static void setCapturePreviewAllPanels(CameraSettingsPanel *mainPanel,
                                       ParametersOverlayPanel *overlay,
                                       const QPixmap &pixmap) {
  if (mainPanel)
    mainPanel->setCapturePreview(pixmap);
  if (overlay)
    overlay->setCapturePreview(pixmap);
}

static void clearCapturePreviewAllPanels(CameraSettingsPanel *mainPanel,
                                         ParametersOverlayPanel *overlay) {
  if (mainPanel)
    mainPanel->clearCapturePreview();
  if (overlay)
    overlay->clearCapturePreview();
}

void MainWindow::onPhotoCaptureFinished(DwarfCameraController::CameraKind kind,
                                        bool success, int code,
                                        const QString &fileName) {
  Q_UNUSED(code);
  Q_UNUSED(fileName);

  if (!success)
    return;

  clearCapturePreviewAllPanels(m_cameraSettingsPanel, m_paramsOverlay);

  // Trigger a media refresh to obtain the latest filename.
  // mediaType: 1=photo
  m_pendingCaptureLookup.active = true;
  m_pendingCaptureLookup.expectedMediaType = 1;
  m_pendingCaptureLookup.expectedKind = kind;
  m_pendingCaptureLookup.prefix = tr("Photo saved:");
  m_pendingCaptureLookup.thumbnailPath.clear();
  m_pendingCaptureLookup.attempts = 0;

  ensureHttpClientForCurrentIp();
  if (m_httpClient)
    m_httpClient->fetchMediaList();
}

void MainWindow::onRecordFinished(DwarfCameraController::CameraKind kind,
                                  bool recording, bool success, int code) {
  Q_UNUSED(code);
  if (!success)
    return;

  // Only after STOP was confirmed (recording=false) we can resolve the filename.
  if (kind != DwarfCameraController::CameraKind::Tele)
    return;
  if (recording)
    return;

  m_pendingCaptureLookup.active = true;
  m_pendingCaptureLookup.expectedMediaType = 2; // video
  m_pendingCaptureLookup.expectedKind = DwarfCameraController::CameraKind::Tele;
  m_pendingCaptureLookup.prefix = tr("Video saved:");

  ensureHttpClientForCurrentIp();
  if (m_httpClient)
    m_httpClient->fetchMediaList();
}

MainWindow::~MainWindow() {
  if (m_wsClient) {
    m_wsClient->disconnect();
    delete m_wsClient;
  }

  stopStreaming();
}

void MainWindow::updateStatusStyle(const char *statusKey) {
  if (!m_statusLabel)
    return;
  m_statusLabel->setProperty("status", statusKey);
  m_statusLabel->style()->unpolish(m_statusLabel);
  m_statusLabel->style()->polish(m_statusLabel);
  m_statusLabel->update();
}

void MainWindow::updateCameraStreamViews() {
  if (!m_mainStreamView || !m_pipContainer)
    return;

  const bool mainIsTele = (m_mainStream == CameraStream::Tele);

  if (mainIsTele) {
    m_streamNameOverlay->setText(tr("Live Stream (TELE)"));
    // m_pipStreamView->setText(tr("WIDE")); // No text on PiP, video covers it
  } else {
    m_streamNameOverlay->setText(tr("Live Stream (WIDE)"));
    // m_pipStreamView->setText(tr("TELE"));
  }

  // Sync camera settings panel with stream view
  if (m_cameraSettingsPanel) {
    auto mode = mainIsTele ? CameraSettingsPanel::CameraMode::Tele
                           : CameraSettingsPanel::CameraMode::Wide;
    if (m_cameraSettingsPanel->cameraMode() != mode) {
      m_cameraSettingsPanel->setCameraMode(mode);
    }

    if (m_paramsOverlay)
      m_paramsOverlay->setCameraMode(mode);
  }

  // Ensure overlays stay on top
  m_streamNameOverlay->raise();
  if (m_pipContainer)
    m_pipContainer->raise();

  updateStreamRouting();
}

void MainWindow::updateStreamRouting() {
  if (!m_teleStream || !m_wideStream || !m_mainVideoWidget || !m_pipVideoWidget)
    return;

  const bool mainIsTele = (m_mainStream == CameraStream::Tele);
  qWarning() << "[MainWindow] updateStreamRouting: mainIsTele=" << mainIsTele;
  if (mainIsTele) {
    m_mainVideoWidget->setSourceImage(&m_teleStream->currentFrame());
    m_pipVideoWidget->setSourceImage(&m_wideStream->currentFrame());
  } else {
    m_mainVideoWidget->setSourceImage(&m_wideStream->currentFrame());
    m_pipVideoWidget->setSourceImage(&m_teleStream->currentFrame());
  }
}

void MainWindow::onGalleryOverlayRequested(bool enabled) {
  m_galleryOverlayEnabled = enabled;

  updateOverlayVisibility();

  if (!m_galleryOverlayContainer || !m_galleryTab)
    return;

  auto *overlayLayout = qobject_cast<QVBoxLayout *>(m_galleryOverlayContainer->layout());
  if (!overlayLayout) {
    overlayLayout = new QVBoxLayout(m_galleryOverlayContainer);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->setSpacing(0);
  }

  if (enabled) {
    if (m_contentStack)
      m_contentStack->setVisible(false);

    if (m_contentStack) {
      m_contentStack->removeWidget(m_galleryTab);
    }

    m_galleryTab->setParent(m_galleryOverlayContainer);
    overlayLayout->addWidget(m_galleryTab);
    m_galleryTab->show();

    m_galleryOverlayContainer->setVisible(true);
    updateOverlayPositions();
    m_galleryOverlayContainer->raise();
    return;
  }

  // Restore
  overlayLayout->removeWidget(m_galleryTab);
  if (m_contentStack)
    m_galleryTab->setParent(m_contentStack);
  if (m_contentStack && m_contentStack->indexOf(m_galleryTab) < 0) {
    // Gallery is at index 4
    m_contentStack->insertWidget(4, m_galleryTab);
  }

  // Let QStackedWidget control visibility of non-current pages.
  m_galleryTab->setVisible(false);

  m_galleryOverlayContainer->setVisible(false);
  if (m_contentStack && !m_starMapOverlayEnabled)
    m_contentStack->setVisible(true);

  updateOverlayVisibility();
}

void MainWindow::onMotorSpeedSliderChanged(int value) {
  int idx = value;
  if (idx < 0)
    idx = 0;
  else if (idx > 4)
    idx = 4;
  static const double speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
  double speed = speedTable[idx];
  if (m_motorSpeedValueLabel)
    m_motorSpeedValueLabel->setText(tr("%1 deg/s").arg(speed));
}

void MainWindow::startStreaming(const QString &ip) {
  if (!m_teleStream || !m_wideStream)
    return;

  qWarning() << "[MainWindow] startStreaming called for IP" << ip;

  qDebug() << "Sending OpenCamera commands...";
  // Ensure camera is opened on both Tele and Wide before requesting RTSP
  if (m_cameraController) {
    m_cameraController->openCamera(DwarfCameraController::CameraKind::Tele,
                                   true, 0);
    m_cameraController->openCamera(DwarfCameraController::CameraKind::Wide,
                                   false, 0);
  } else {
    qWarning()
        << "[MainWindow] m_cameraController is null, cannot open cameras";
  }
  // RTSP players will be started in onCameraTeleMessage / onCameraWideMessage
}

void MainWindow::stopStreaming() {
  if (m_teleStream)
    m_teleStream->stop();
  if (m_wideStream)
    m_wideStream->stop();
}


void MainWindow::setupUi() {
  setWindowTitle(tr("DWARF II Controller"));
  resize(1280, 800);

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);
  QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // SIDEBAR
  m_sidebar = new QWidget(centralWidget);
  m_sidebar->setObjectName("sidebar");
  m_sidebar->setFixedWidth(70);
  QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
  sidebarLayout->setContentsMargins(5, 10, 5, 10);
  sidebarLayout->setSpacing(15);

  m_sidebarGroup = new QButtonGroup(this);
  auto addSidebarBtn = [&](const QString &iconPath, const QString &label, int index) {
    QToolButton *btn = new QToolButton(m_sidebar);
    btn->setCheckable(true);
    btn->setText(label);
    btn->setIcon(QIcon(iconPath));
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setFixedSize(65, 75);
    btn->setIconSize(QSize(32, 32));
    m_sidebarGroup->addButton(btn, index);
    sidebarLayout->addWidget(btn);
    connect(btn, &QToolButton::clicked, this, [this, index]() {
      const bool gallerySelected = (index == 4);
      const bool astroSelected = (index == 2);

      if (gallerySelected) {
        // Ensure other overlays are off before enabling Gallery overlay.
        onStarMapOverlayRequested(false);
        if (m_astroTabsOverlayContainer)
          m_astroTabsOverlayContainer->setVisible(false);
        onGalleryOverlayRequested(true);
        onOpenGalleryClicked();
        return;
      }

      onGalleryOverlayRequested(false);

      if (m_contentStack)
        m_contentStack->setCurrentIndex(index);

      updateOverlayVisibility();

      if (m_astroTabsOverlayContainer)
        m_astroTabsOverlayContainer->setVisible(astroSelected);

      if (astroSelected) {
        if (m_astroPanel) {
          m_astroPanel->setCurrentTabIndex(0);
        }
        onStarMapOverlayRequested(true);
      } else {
        onStarMapOverlayRequested(false);
      }
    });
  };

  addSidebarBtn(":/icons/icons/scan.svg", tr("SCAN"), 0);
  addSidebarBtn(":/icons/icons/camera.svg", tr("CAM"), 1);
  addSidebarBtn(":/icons/icons/astro.svg", tr("ASTRO"), 2);
  addSidebarBtn(":/icons/icons/panorama.svg", tr("PANO"), 3);
  addSidebarBtn(":/icons/icons/gallery.svg", tr("GAL"), 4);
  addSidebarBtn(":/icons/icons/settings.svg", tr("SET"), 5);

  auto addOverlayToggleBtn = [&](const QString &iconPath, const QString &toolTip) {
    QToolButton *btn = new QToolButton(m_sidebar);
    btn->setCheckable(true);
    btn->setChecked(true);
    btn->setIcon(QIcon(iconPath));
    btn->setToolTip(toolTip);
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setFixedSize(65, 75);
    btn->setIconSize(QSize(32, 32));
    sidebarLayout->addWidget(btn);
    return btn;
  };

  m_motorOverlayToggleButton = addOverlayToggleBtn(
      ":/icons/icons/overlay_motor.svg", tr("Toggle Motor Control"));
  m_paramsOverlayToggleButton = addOverlayToggleBtn(
      ":/icons/icons/overlay_params.svg", tr("Toggle Parameters"));

  connect(m_motorOverlayToggleButton, &QToolButton::toggled, this,
          [this](bool checked) {
            m_motorOverlayUserVisible = checked;
            updateOverlayVisibility();
          });
  connect(m_paramsOverlayToggleButton, &QToolButton::toggled, this,
          [this](bool checked) {
            m_paramsOverlayUserVisible = checked;
            updateOverlayVisibility();
          });

  sidebarLayout->addStretch();
  mainLayout->addWidget(m_sidebar);

  // CENTRAL VIEWPORT (Camera Stream)
  m_mainStreamView = new QWidget(centralWidget);
  m_mainVideoWidget = new DwarfMjpegView(m_mainStreamView);
  m_mainVideoWidget->installEventFilter(this);
  QVBoxLayout *viewportLayout = new QVBoxLayout(m_mainStreamView);
  viewportLayout->setContentsMargins(0, 0, 0, 0);
  viewportLayout->addWidget(m_mainVideoWidget);

  // StarMap overlay (over stream)
  m_starMapOverlayContainer = new QWidget(centralWidget);
  m_starMapOverlayContainer->setObjectName("starMapOverlayContainer");
  m_starMapOverlayContainer->setVisible(false);
  QVBoxLayout *starMapOverlayLayout = new QVBoxLayout(m_starMapOverlayContainer);
  starMapOverlayLayout->setContentsMargins(0, 0, 0, 0);
  starMapOverlayLayout->setSpacing(0);

  // Gallery overlay (over stream)
  m_galleryOverlayContainer = new QWidget(centralWidget);
  m_galleryOverlayContainer->setObjectName("galleryOverlayContainer");
  m_galleryOverlayContainer->setVisible(false);
  auto *galleryOverlayLayout = new QVBoxLayout(m_galleryOverlayContainer);
  galleryOverlayLayout->setContentsMargins(0, 0, 0, 0);
  galleryOverlayLayout->setSpacing(0);

  // Astro tabs overlay (over stream)
  m_astroTabsOverlayContainer = new QWidget(centralWidget);
  m_astroTabsOverlayContainer->setVisible(false);
  auto *astroTabsOverlayLayout = new QVBoxLayout(m_astroTabsOverlayContainer);
  astroTabsOverlayLayout->setContentsMargins(0, 0, 0, 0);
  astroTabsOverlayLayout->setSpacing(0);

  // OVERLAYS ON MAIN VIDEO
  m_streamNameOverlay = new QLabel(m_mainVideoWidget);
  m_streamNameOverlay->setObjectName("streamNameOverlay");
  m_streamNameOverlay->move(10, 10);

  m_pipContainer = new DraggablePiP(m_mainVideoWidget);
  m_pipContainer->setFixedSize(240, 135);
  m_pipContainer->move(10, 50);
  m_pipContainer->setStyleSheet(
      "DraggablePiP { border: 2px solid #27ae60; background: black; }");
  m_pipVideoWidget = new DwarfMjpegView(m_pipContainer);
  QVBoxLayout *pipLayout = new QVBoxLayout(m_pipContainer);
  pipLayout->setContentsMargins(0, 0, 0, 0);
  pipLayout->addWidget(m_pipVideoWidget);
  connect(m_pipContainer, &DraggablePiP::doubleClicked, this,
          &MainWindow::onPipStreamClicked);

  // Motor Control Overlay (draggable, top layer)
  // Parent is centralWidget so it can be dragged over other panels
  m_motorOverlay = new MotorControlPanel(centralWidget);
  m_motorOverlay->setMotorController(m_motorController);
  m_motorOverlay->setFocusController(m_focusController);
  m_motorOverlay->setFixedSize(290, 380);
  m_motorOverlay->setCursor(Qt::OpenHandCursor);  // Show it's draggable
  m_motorOverlay->setMouseTracking(true);
  // Initial position will be set by updateOverlayPositions
  m_motorOverlay->raise();
  m_motorOverlay->show();

  // Parameters Overlay (draggable, top layer)
  m_paramsOverlay = new ParametersOverlayPanel(centralWidget);
  m_paramsOverlay->setCameraController(m_cameraController);
  m_paramsOverlay->setFixedSize(340, 520);
  m_paramsOverlay->setCursor(Qt::OpenHandCursor);
  m_paramsOverlay->setMouseTracking(true);
  m_paramsOverlay->raise();
  m_paramsOverlay->show();

  mainLayout->addWidget(m_mainStreamView, 1);

  // CONTENT PANELS
  m_contentStack = new QStackedWidget(this);
  m_contentStack->setFixedWidth(350);

  // 0: Connect/Scan Panel
  QWidget *connectTab = new QWidget();
  QVBoxLayout *cl = new QVBoxLayout(connectTab);
  m_statusLabel = new QLabel("Disconnected");
  m_statusLabel->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #e74c3c; padding: 10px;");
  cl->addWidget(m_statusLabel);

  m_ipInput = new QLineEdit();
  m_ipInput->setPlaceholderText("DWARF II IP");
  {
    QSettings settings("DwarfLab", "DwarfController");
    const QString ip = settings.value("ip", QStringLiteral("192.168.8.223")).toString();
    m_ipInput->setText(ip);
  }

  m_subnetInput = new QLineEdit();
  m_subnetInput->setPlaceholderText("Subnet (e.g. 192.168.1)");
  {
    QSettings settings("DwarfLab", "DwarfController");
    const QString subnet = settings.value("subnet", QStringLiteral("192.168.8")).toString();
    m_subnetInput->setText(subnet);
  }
  connect(m_subnetInput, &QLineEdit::textChanged, this,
          &MainWindow::onSubnetTextChanged);

  connect(m_ipInput, &QLineEdit::editingFinished, this, [this]() {
    if (!m_ipInput)
      return;
    QSettings settings("DwarfLab", "DwarfController");
    settings.setValue("ip", m_ipInput->text().trimmed());
  });

  connect(m_subnetInput, &QLineEdit::editingFinished, this, [this]() {
    if (!m_subnetInput)
      return;
    QSettings settings("DwarfLab", "DwarfController");
    settings.setValue("subnet", m_subnetInput->text().trimmed());
  });

  QHBoxLayout *scanBtnLayout = new QHBoxLayout();
  m_scanButton = new QPushButton(tr("Scan Network"));
  m_cancelScanButton = new QPushButton(tr("Cancel Scan"));
  m_cancelScanButton->setEnabled(false);
  scanBtnLayout->addWidget(m_scanButton);
  scanBtnLayout->addWidget(m_cancelScanButton);

  connect(m_scanButton, &QPushButton::clicked, this, &MainWindow::onScanClicked);
  connect(m_cancelScanButton, &QPushButton::clicked, this,
          &MainWindow::onCancelScanClicked);

  cl->addLayout(scanBtnLayout);

  QHBoxLayout *connectBtnLayout = new QHBoxLayout();
  m_connectButton = new QPushButton(tr("Connect"));
  m_cancelConnectButton = new QPushButton(tr("Cancel"));
  m_cancelConnectButton->setEnabled(false);
  connectBtnLayout->addWidget(m_connectButton);
  connectBtnLayout->addWidget(m_cancelConnectButton);

  connect(m_connectButton, &QPushButton::clicked, this,
          &MainWindow::onConnectClicked);
  connect(m_cancelConnectButton, &QPushButton::clicked, this,
          &MainWindow::onCancelConnectClicked);

  auto *ipRow = new QWidget(connectTab);
  auto *ipLayout = new QHBoxLayout(ipRow);
  ipLayout->setContentsMargins(0, 0, 0, 0);
  auto *ipLabel = new QLabel(tr("IP"), ipRow);
  ipLayout->addWidget(ipLabel);
  ipLayout->addWidget(m_ipInput, 1);
  cl->addWidget(ipRow);

  auto *netRow = new QWidget(connectTab);
  auto *netLayout = new QHBoxLayout(netRow);
  netLayout->setContentsMargins(0, 0, 0, 0);
  auto *netLabel = new QLabel(tr("Network"), netRow);
  netLayout->addWidget(netLabel);
  netLayout->addWidget(m_subnetInput, 1);
  cl->addWidget(netRow);

  const int labelW = qMax(ipLabel->sizeHint().width(), netLabel->sizeHint().width());
  ipLabel->setFixedWidth(labelW);
  netLabel->setFixedWidth(labelW);

  cl->addLayout(connectBtnLayout);

  m_deviceList = new QListWidget();

  cl->addWidget(new QLabel(tr("Devices Found:")));
  cl->addWidget(m_deviceList);
  cl->addStretch();
  m_contentStack->addWidget(connectTab);



  // 1: Camera Panel
  m_cameraSettingsPanel = new CameraSettingsPanel(this);
  m_cameraSettingsPanel->setCameraController(m_cameraController);
  m_cameraSettingsPanel->setDisplayMode(CameraSettingsPanel::DisplayMode::CaptureOnly);
  m_contentStack->addWidget(m_cameraSettingsPanel);

  // 2: Astro Panel
  m_astroPanel = new AstroNavigationPanel(this);
  m_astroPanel->setWebSocketClient(m_wsClient);
  m_astroPanel->setCameraController(m_cameraController);
  m_astroPanel->setAstroController(m_astroController);
  m_contentStack->addWidget(m_astroPanel);
  connect(m_astroPanel, &AstroNavigationPanel::starMapOverlayRequested, this,
          &MainWindow::onStarMapOverlayRequested);

  // Build overlay tab bar that controls Astro tabs while StarMap is full overlay
  if (m_astroPanel->tabWidget()) {
    m_astroTabsOverlayBar = new QTabBar(m_astroTabsOverlayContainer);
    m_astroTabsOverlayBar->setExpanding(false);
    m_astroTabsOverlayBar->setDrawBase(false);

    for (int i = 0; i < m_astroPanel->tabWidget()->count(); ++i) {
      m_astroTabsOverlayBar->addTab(m_astroPanel->tabWidget()->tabText(i));
    }

    astroTabsOverlayLayout->addWidget(m_astroTabsOverlayBar);

    connect(m_astroTabsOverlayBar, &QTabBar::currentChanged, this,
            [this](int index) {
              if (!m_astroPanel)
                return;
              m_astroPanel->setCurrentTabIndex(index);
            });

    connect(m_astroPanel->tabWidget(), &QTabWidget::currentChanged, this,
            [this](int index) {
              if (!m_astroTabsOverlayBar)
                return;
              const QSignalBlocker blocker(m_astroTabsOverlayBar);
              m_astroTabsOverlayBar->setCurrentIndex(index);
            });
  }

  // 3: Pano Panel
  QWidget *panoTab = new QWidget();
  auto *pl = new QVBoxLayout(panoTab);
  QLabel *panoHeading = new QLabel(tr("Panorama Mode"));
  panoHeading->setProperty("heading", true);
  pl->addWidget(panoHeading);

  auto *gridRow = new QWidget(panoTab);
  auto *gridLayout = new QHBoxLayout(gridRow);
  gridLayout->setContentsMargins(0, 0, 0, 0);
  gridLayout->setSpacing(10);

  auto *rowsLabel = new QLabel(tr("Rows:"), gridRow);
  auto *rowsSpin = new QSpinBox(gridRow);
  rowsSpin->setRange(1, 10);
  rowsSpin->setValue(3);

  auto *colsLabel = new QLabel(tr("Cols:"), gridRow);
  auto *colsSpin = new QSpinBox(gridRow);
  colsSpin->setRange(1, 10);
  colsSpin->setValue(3);

  gridLayout->addWidget(rowsLabel);
  gridLayout->addWidget(rowsSpin);
  gridLayout->addSpacing(12);
  gridLayout->addWidget(colsLabel);
  gridLayout->addWidget(colsSpin);
  gridLayout->addStretch(1);
  pl->addWidget(gridRow);

  auto *panoStatus = new QLabel(tr("Idle"), panoTab);
  panoStatus->setStyleSheet("color: gray;");
  pl->addWidget(panoStatus);

  auto *btnRow = new QWidget(panoTab);
  auto *btnLayout = new QHBoxLayout(btnRow);
  btnLayout->setContentsMargins(0, 0, 0, 0);
  btnLayout->setSpacing(10);

  auto *panoStart = new QPushButton(tr("Start Panorama"), btnRow);
  panoStart->setStyleSheet(
      "background: #27ae60; color: white; padding: 10px; border-radius: 5px;");
  auto *panoStop = new QPushButton(tr("Stop"), btnRow);
  panoStop->setStyleSheet(
      "background: #c0392b; color: white; padding: 10px; border-radius: 5px;");
  panoStop->setEnabled(false);

  btnLayout->addWidget(panoStart);
  btnLayout->addWidget(panoStop);
  btnLayout->addStretch(1);
  pl->addWidget(btnRow);

  connect(panoStart, &QPushButton::clicked, this, [this, rowsSpin, colsSpin, panoStatus, panoStart, panoStop]() {
    if (!m_panoramaController) {
      panoStatus->setText(tr("Panorama controller not available"));
      panoStatus->setStyleSheet("color: red;");
      return;
    }
    panoStatus->setText(tr("Starting..."));
    panoStatus->setStyleSheet("color: blue;");
    panoStart->setEnabled(false);
    panoStop->setEnabled(true);
    m_panoramaController->startPanoramaGrid(rowsSpin->value(), colsSpin->value());
  });

  connect(panoStop, &QPushButton::clicked, this, [this, panoStatus, panoStart, panoStop]() {
    if (!m_panoramaController)
      return;
    panoStatus->setText(tr("Stopping..."));
    panoStatus->setStyleSheet("color: blue;");
    m_panoramaController->stopPanorama();
    panoStop->setEnabled(false);
    panoStart->setEnabled(true);
  });

  if (m_panoramaController) {
    connect(m_panoramaController, &DwarfPanoramaController::panoramaStarted, this,
            [panoStatus, panoStop, panoStart](int rows, int cols) {
              panoStatus->setText(QObject::tr("Running (%1 x %2)...").arg(rows).arg(cols));
              panoStatus->setStyleSheet("color: green;");
              panoStart->setEnabled(false);
              panoStop->setEnabled(true);
            });
    connect(m_panoramaController, &DwarfPanoramaController::panoramaStopped, this,
            [panoStatus, panoStop, panoStart]() {
              panoStatus->setText(QObject::tr("Stopped"));
              panoStatus->setStyleSheet("color: gray;");
              panoStart->setEnabled(true);
              panoStop->setEnabled(false);
            });
    connect(m_panoramaController, &DwarfPanoramaController::panoramaFailed, this,
            [panoStatus, panoStop, panoStart](const QString &error) {
              panoStatus->setText(QObject::tr("Failed: %1").arg(error));
              panoStatus->setStyleSheet("color: red;");
              panoStart->setEnabled(true);
              panoStop->setEnabled(false);
            });
  }

  pl->addStretch();
  m_contentStack->addWidget(panoTab);

  // 4: Gallery Panel
  m_galleryTab = new QWidget();
  m_galleryTab->setObjectName("galleryTab");
  auto *gl = new QVBoxLayout(m_galleryTab);
  m_mediaTabs = new QTabWidget();
  m_mediaTabs->setObjectName("galleryMediaTabs");
  m_mediaPhotoList = new QListWidget();
  m_mediaPhotoList->setObjectName("galleryMediaList");
  m_mediaVideoList = new QListWidget();
  m_mediaVideoList->setObjectName("galleryMediaList");
  m_mediaBurstList = new QListWidget();
  m_mediaBurstList->setObjectName("galleryMediaList");
  m_mediaAstroList = new QListWidget();
  m_mediaAstroList->setObjectName("galleryMediaList");
  m_mediaPanoList = new QListWidget();
  m_mediaPanoList->setObjectName("galleryMediaList");
  m_mediaTabs->addTab(m_mediaPhotoList, "Photos");
  m_mediaTabs->addTab(m_mediaVideoList, "Videos");
  m_mediaTabs->addTab(m_mediaBurstList, "Burst");
  m_mediaTabs->addTab(m_mediaAstroList, "Astro");
  m_mediaTabs->addTab(m_mediaPanoList, "Panorama");
  gl->addWidget(m_mediaTabs);
  m_contentStack->addWidget(m_galleryTab);

  // 5: Settings Panel
  QWidget *settingsTab = new QWidget();
  auto *sl = new QVBoxLayout(settingsTab);
  sl->addWidget(new QLabel("Application Settings"));

  QHBoxLayout *downloadLayout = new QHBoxLayout();
  downloadLayout->addWidget(new QLabel("Download folder:"));
  m_downloadDirEdit = new QLineEdit();
  m_downloadDirEdit->setReadOnly(true);
  m_changeDownloadDirButton = new QPushButton("Change...");
  downloadLayout->addWidget(m_downloadDirEdit);
  downloadLayout->addWidget(m_changeDownloadDirButton);
  sl->addLayout(downloadLayout);

  sl->addStretch();
  m_contentStack->addWidget(settingsTab);

  mainLayout->addWidget(m_contentStack, 1);

  m_sidebarGroup->button(0)->setChecked(true);
  m_contentStack->setCurrentIndex(0);

  updateOverlayVisibility();

  auto applyGlow = [](QWidget *w) {
    if (!w)
      return;
    if (w->graphicsEffect())
      return;
    auto *glow = new QGraphicsDropShadowEffect(w);
    glow->setBlurRadius(28);
    glow->setOffset(0, 0);
    glow->setColor(QColor(39, 174, 96, 120));
    w->setGraphicsEffect(glow);
  };

  for (QWidget *w : {static_cast<QWidget *>(m_sidebar),
                    static_cast<QWidget *>(m_contentStack),
                    static_cast<QWidget *>(connectTab),
                    static_cast<QWidget *>(m_cameraSettingsPanel),
                    static_cast<QWidget *>(m_astroPanel),
                    static_cast<QWidget *>(panoTab),
                    static_cast<QWidget *>(m_galleryTab),
                    static_cast<QWidget *>(settingsTab)}) {
    applyGlow(w);
  }

  for (QGroupBox *gb : m_contentStack->findChildren<QGroupBox *>()) {
    applyGlow(gb);
  }

  // Re-connect signals
  connect(m_mainVideoWidget, &DwarfMjpegView::pointClicked, this,
          &MainWindow::onMainViewPointClicked);
  connect(m_cameraSettingsPanel, &CameraSettingsPanel::cameraModeChanged, this,
          [this](CameraSettingsPanel::CameraMode mode) {
            if (mode == CameraSettingsPanel::CameraMode::Tele)
              onCameraSourceTele();
            else
              onCameraSourceWide();
          });

  if (m_changeDownloadDirButton)
    connect(m_changeDownloadDirButton, &QPushButton::clicked, this,
            &MainWindow::onChangeDownloadDirClicked);

  connect(m_deviceList, &QListWidget::itemDoubleClicked, this,
          &MainWindow::onDeviceSelected);

  for (QListWidget *list :
       {m_mediaPhotoList, m_mediaVideoList, m_mediaBurstList, m_mediaAstroList,
        m_mediaPanoList}) {
    if (list) {
      connect(list, &QListWidget::itemClicked, this,
              &MainWindow::onMediaItemClicked);
      connect(list, &QListWidget::itemDoubleClicked, this,
              &MainWindow::onMediaItemActivated);
    }
  }

  // MJPEG stream connections
  if (m_teleStream) {
    connect(m_teleStream, &DwarfMjpegStream::frameUpdated, this, [this]() {
      if (!m_mainVideoWidget || !m_pipVideoWidget)
        return;
      if (m_mainStream == CameraStream::Tele)
        m_mainVideoWidget->update();
      if (m_pipStream == CameraStream::Tele)
        m_pipVideoWidget->update();
    });
  }
  if (m_wideStream) {
    connect(m_wideStream, &DwarfMjpegStream::frameUpdated, this, [this]() {
      if (!m_mainVideoWidget || !m_pipVideoWidget)
        return;
      if (m_mainStream == CameraStream::Wide)
        m_mainVideoWidget->update();
      if (m_pipStream == CameraStream::Wide)
        m_pipVideoWidget->update();
    });
  }

  statusBar()->showMessage(tr("Ready"));

  // Settings
  QSettings settings("DwarfLab", "DwarfController");
  QString dir = settings.value("downloadDir").toString();
  if (dir.isEmpty()) {
    QString base =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (base.isEmpty())
      base = QDir::homePath();
    QDir d(base);
    if (!d.exists("DWARF_II"))
      d.mkpath("DWARF_II");
    dir = d.filePath("DWARF_II");
  }
  m_downloadDir = dir;
  if (m_downloadDirEdit)
    m_downloadDirEdit->setText(dir);
    
  // Ensure overlays are positioned correctly after layout is done
  QTimer::singleShot(0, this, &MainWindow::updateOverlayPositions);
}

void MainWindow::updateOverlayVisibility() {
  const QWidget *current = m_contentStack ? m_contentStack->currentWidget() : nullptr;
  const bool camSelected = (current == m_cameraSettingsPanel);
  const bool astroSelected = (current == m_astroPanel);
  const bool panoSelected = (m_contentStack && m_contentStack->currentIndex() == 3);

  const bool allowOverlays = (camSelected || astroSelected || panoSelected);
  const bool blockedByFullscreenOverlay = m_galleryOverlayEnabled;

  const bool motorVisible = allowOverlays && !blockedByFullscreenOverlay && m_motorOverlayUserVisible;
  const bool paramsVisible = allowOverlays && !blockedByFullscreenOverlay && m_paramsOverlayUserVisible;

  if (m_motorOverlay)
    m_motorOverlay->setVisible(motorVisible);
  if (m_paramsOverlay)
    m_paramsOverlay->setVisible(paramsVisible);

  // Ensure overlays stay on top of fullscreen overlays (e.g. StarMap)
  if (m_motorOverlay && m_motorOverlay->isVisible())
    m_motorOverlay->raise();
  if (m_paramsOverlay && m_paramsOverlay->isVisible())
    m_paramsOverlay->raise();
}

void MainWindow::onScanClicked() {
  QString subnet = m_subnetInput->text().trimmed();
  qDebug() << "Scan button clicked, subnet:" << subnet;
  m_scanCancelled = false;
  m_deviceList->clear();
  m_scanButton->setEnabled(false);
  m_cancelScanButton->setEnabled(true);
  m_statusLabel->setText(tr("Scanning %1.0/24...").arg(subnet));
  updateStatusStyle("scanning");
  m_finder->startScan(subnet);
}

void MainWindow::onCancelScanClicked() {
  m_finder->stopScan();
  m_scanCancelled = true;
  m_statusLabel->setText(tr("Scan Cancelled"));
  updateStatusStyle("cancelled");
}

void MainWindow::onScanProgress(int percent) {
  QString subnet = m_subnetInput->text().trimmed();
  m_statusLabel->setText(
      tr("Scanning %1.0/24... %2%").arg(subnet).arg(percent));
}

void MainWindow::onDeviceFound(const DwarfDeviceInfo &info) {
  QString label = QString("%1 - %2").arg(info.ip).arg(info.name);
  QListWidgetItem *item = new QListWidgetItem(label);
  item->setData(Qt::UserRole, info.ip);
  m_deviceList->addItem(item);
}

void MainWindow::onScanFinished() {
  qDebug() << "Scan finished, re-enabling scan button";
  m_scanButton->setEnabled(true);
  m_cancelScanButton->setEnabled(false);

  if (m_scanCancelled) {
    // Keep cancelled status
    qDebug() << "Scan was cancelled";
  } else if (m_deviceList->count() == 0) {
    m_statusLabel->setText(tr("No devices found"));
    updateStatusStyle("noDevices");
    qDebug() << "No devices found";
  } else {
    m_statusLabel->setText(tr("Found %1 devices").arg(m_deviceList->count()));
    updateStatusStyle("ok");
    qDebug() << "Found" << m_deviceList->count() << "devices";
  }
}

void MainWindow::onDeviceSelected(QListWidgetItem *item) {
  QString ip = item->data(Qt::UserRole).toString();
  m_ipInput->setText(ip);
  if (m_wsClient) {
    onDisconnectClicked();
  }
  onConnectClicked();
}

void MainWindow::onConnectClicked() {
  QString ip = m_ipInput->text().trimmed();

  if (ip.isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("Please enter an IP address"));
    return;
  }

  // If we are currently connected or connecting, then this click means
  // DISCONNECT
  if (m_wsClient) {
    qDebug() << "Disconnecting/Aborting...";
    onDisconnectClicked(); // Internal helper for cleanup
    return;
  }

  // Create new client
  m_wsClient = new DwarfWebSocketClient(ip, this);

  connect(m_wsClient, &DwarfWebSocketClient::connected, this,
          &MainWindow::onWebSocketConnected);
  connect(m_wsClient, &DwarfWebSocketClient::disconnected, this,
          &MainWindow::onWebSocketDisconnected);
  connect(m_wsClient, &DwarfWebSocketClient::errorOccurred, this,
          &MainWindow::onWebSocketError);

  if (!m_dispatcher) {
    m_dispatcher = new DwarfMessageDispatcher(this);
  }

  connect(m_wsClient, &DwarfWebSocketClient::messageReceived, m_dispatcher,
          &DwarfMessageDispatcher::dispatch);

  if (m_cameraController)
    m_cameraController->setClient(m_wsClient);
  if (m_motorController)
    m_motorController->setClient(m_wsClient);
  if (m_focusController)
    m_focusController->setClient(m_wsClient);
  if (m_astroController)
    m_astroController->setClient(m_wsClient);
  if (m_panoramaController)
    m_panoramaController->setClient(m_wsClient);
  if (m_astroPanel)
    m_astroPanel->setWebSocketClient(m_wsClient);

  m_wsClient->connectToDevice();
  m_connectButton->setText(tr("Abort"));
  m_cancelConnectButton->setEnabled(true);
  m_statusLabel->setText(tr("Connecting..."));
  updateStatusStyle("connecting");
  statusBar()->showMessage(tr("Connecting to %1").arg(ip));
}

void MainWindow::onDisconnectClicked() {
  if (m_wsClient) {
    m_wsClient->disconnect();
    m_wsClient->deleteLater();
    m_wsClient = nullptr;
  }

  m_connectButton->setText(tr("Connect"));
  m_cancelConnectButton->setEnabled(false);
  m_statusLabel->setText(tr("Disconnected"));
  updateStatusStyle("disconnected");
  statusBar()->showMessage(tr("Disconnected"));

  if (m_cameraController)
    m_cameraController->setClient(nullptr);
  if (m_motorController)
    m_motorController->setClient(nullptr);
  if (m_focusController)
    m_focusController->setClient(nullptr);
  if (m_astroController)
    m_astroController->setClient(nullptr);
  if (m_panoramaController)
    m_panoramaController->setClient(nullptr);
  if (m_astroPanel)
    m_astroPanel->setWebSocketClient(nullptr);

  stopStreaming();
}

void MainWindow::onCancelConnectClicked() {
  if (m_wsClient) {
    m_wsClient->disconnect();
    m_wsClient->deleteLater();
    m_wsClient = nullptr;
  }

  m_connectButton->setEnabled(true);
  m_connectButton->setText(tr("Connect"));
  m_cancelConnectButton->setEnabled(false);
  m_statusLabel->setText(tr("Cancelled"));
  updateStatusStyle("cancelled");
  statusBar()->showMessage(tr("Connection cancelled"));
}

void MainWindow::onSubnetTextChanged(const QString &text) {
  // Only update IP if it's set to a default or empty
  QString currentIp = m_ipInput->text().trimmed();
  if (currentIp.isEmpty() || currentIp.endsWith(".1") ||
      currentIp == "192.168.8.223") {
    QStringList parts = text.split('.');
    if (parts.size() >= 3) {
      QString ip =
          QString("%1.%2.%3.223").arg(parts[0]).arg(parts[1]).arg(parts[2]);
      m_ipInput->setText(ip);
    }
  }
}

void MainWindow::onWebSocketConnected() {
  qWarning() << "[MainWindow] WebSocket connected, starting streaming";
  m_connectButton->setEnabled(true);
  m_connectButton->setText(tr("Disconnect"));
  m_cancelConnectButton->setEnabled(
      false); // Can't cancel if already connected, use Disconnect
  m_statusLabel->setText(tr("Connected"));
  updateStatusStyle("ok");
  statusBar()->showMessage(tr("Connected to DWARF II"));

  // Sync time with DWARF device
  syncTimeWithDevice();

  // Update panels with the new client
  if (m_cameraSettingsPanel) {
    // CameraSettingsPanel might need the client too, but it uses
    // m_cameraController usually
  }
  if (m_astroPanel) {
    m_astroPanel->setWebSocketClient(m_wsClient);
  }

  // Start streaming now that we are connected
  QString ip = m_ipInput->text().trimmed();
  startStreaming(ip);
}

void MainWindow::syncTimeWithDevice() {
  if (!m_wsClient || !m_wsClient->isConnected()) {
    return;
  }

  // CMD_SYSTEM_SET_TIME = 13000, MODULE_SYSTEM = 4
  // Send current Unix timestamp
  qint64 timestamp = QDateTime::currentSecsSinceEpoch();
  qWarning() << "[MainWindow] Syncing time with DWARF, timestamp:" << timestamp;

  // Create ReqSetTime protobuf message
  dwarf::ReqSetTime req;
  req.set_timestamp(static_cast<uint64_t>(timestamp));
  QByteArray data(req.ByteSizeLong(), '\0');
  req.SerializeToArray(data.data(), data.size());

  m_wsClient->sendCommand(4, 13000,
                          data); // MODULE_SYSTEM=4, CMD_SYSTEM_SET_TIME=13000
}

void MainWindow::onWebSocketDisconnected() {
  m_connectButton->setEnabled(true);
  m_connectButton->setText(tr("Connect"));
  m_cancelConnectButton->setEnabled(false);
  m_statusLabel->setText(tr("Disconnected"));
  updateStatusStyle("disconnected");
  statusBar()->showMessage(tr("Disconnected from DWARF II"));
}

void MainWindow::onWebSocketError(const QString &error) {
  QMessageBox::critical(this, tr("Connection Error"), error);
  onDisconnectClicked(); // Ensure full cleanup on error
  m_statusLabel->setText(tr("Error"));
  updateStatusStyle("error");
  statusBar()->showMessage(tr("Error: %1").arg(error));
}

void MainWindow::onCameraSourceTele() {
  m_mainStream = CameraStream::Tele;
  m_pipStream = CameraStream::Wide;
  updateCameraStreamViews();
}

void MainWindow::onCameraSourceWide() {
  m_mainStream = CameraStream::Wide;
  m_pipStream = CameraStream::Tele;
  updateCameraStreamViews();
}

void MainWindow::onMotorLeftPressed() {
  if (!m_motorController)
    return;
  int idx = 2;
  if (m_motorSpeedSlider)
    idx = m_motorSpeedSlider->value();
  static const double speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
  if (idx < 0)
    idx = 0;
  else if (idx > 4)
    idx = 4;
  double speed = speedTable[idx];
  m_motorController->runMotor(DwarfMotorController::Axis::Azimuth, false,
                              speed);
}

void MainWindow::onMotorLeftReleased() {
  if (!m_motorController)
    return;
  // Stop horizontal axis (Altitude)
  m_motorController->stopMotor(DwarfMotorController::Axis::Azimuth);
}

void MainWindow::onMotorRightPressed() {
  if (!m_motorController)
    return;
  int idx = 2;
  if (m_motorSpeedSlider)
    idx = m_motorSpeedSlider->value();
  static const double speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
  if (idx < 0)
    idx = 0;
  else if (idx > 4)
    idx = 4;
  double speed = speedTable[idx];
  m_motorController->runMotor(DwarfMotorController::Axis::Azimuth, true, speed);
}

void MainWindow::onMotorRightReleased() {
  if (!m_motorController)
    return;
  // Stop horizontal axis (Altitude)
  m_motorController->stopMotor(DwarfMotorController::Axis::Azimuth);
}

void MainWindow::onMotorUpPressed() {
  if (!m_motorController)
    return;
  int idx = 2;
  if (m_motorSpeedSlider)
    idx = m_motorSpeedSlider->value();
  static const double speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
  if (idx < 0)
    idx = 0;
  else if (idx > 4)
    idx = 4;
  double speed = speedTable[idx];
  m_motorController->runMotor(DwarfMotorController::Axis::Altitude, true,
                              speed);
}

void MainWindow::onMotorUpReleased() {
  if (!m_motorController)
    return;
  // Stop Azimuth axis
  m_motorController->stopMotor(DwarfMotorController::Axis::Altitude);
}

void MainWindow::onMotorDownPressed() {
  if (!m_motorController)
    return;
  int idx = 2;
  if (m_motorSpeedSlider)
    idx = m_motorSpeedSlider->value();
  static const double speedTable[5] = {0.1, 1.0, 5.0, 10.0, 30.0};
  if (idx < 0)
    idx = 0;
  else if (idx > 4)
    idx = 4;
  double speed = speedTable[idx];
  m_motorController->runMotor(DwarfMotorController::Axis::Altitude, false,
                              speed);
}

void MainWindow::onFocusMinusClicked() {
  if (!m_focusController)
    return;
  m_focusController->manualStepFar();
}

void MainWindow::onFocusPlusClicked() {
  if (!m_focusController)
    return;
  m_focusController->manualStepNear();
}

void MainWindow::onFocusAutoClicked() {
  if (!m_focusController)
    return;
  m_focusController->autoFocusNormal();
}

void MainWindow::onMotorDownReleased() {
  if (!m_motorController)
    return;
  // Stop Azimuth axis
  m_motorController->stopMotor(DwarfMotorController::Axis::Altitude);
}

void MainWindow::onMainViewPointClicked(const QPointF &normalizedPos) {
  if (!m_motorController)
    return;

  if (m_mainStream != CameraStream::Wide)
    return;

  double nx = normalizedPos.x();
  double ny = normalizedPos.y();

  const double deadZone = 0.1;

  double speed = 5.0;

  double absNx = std::fabs(nx);
  double absNy = std::fabs(ny);

  int baseMs = 800;

  if (absNx > deadZone) {
    bool dirRight = (nx > 0.0);
    int durationMs = static_cast<int>(baseMs * absNx);
    if (durationMs < 150)
      durationMs = 150;
    m_motorController->runMotor(DwarfMotorController::Axis::Azimuth, dirRight,
                                speed);
    QTimer::singleShot(durationMs, this, [this]() {
      if (m_motorController)
        m_motorController->stopMotor(DwarfMotorController::Axis::Azimuth);
    });
  }

  if (absNy > deadZone) {
    bool dirUp = (ny < 0.0);
    int durationMs = static_cast<int>(baseMs * absNy);
    if (durationMs < 150)
      durationMs = 150;
    m_motorController->runMotor(DwarfMotorController::Axis::Altitude, dirUp,
                                speed);
    QTimer::singleShot(durationMs, this, [this]() {
      if (m_motorController)
        m_motorController->stopMotor(DwarfMotorController::Axis::Altitude);
    });
  }
}

void MainWindow::onOpenGalleryClicked() {
  QString ip = m_ipInput ? m_ipInput->text().trimmed() : QString();
  if (ip.isEmpty())
    return;

  // Delete any open lightbox BEFORE clearing lists - must delete immediately
  if (m_currentLightbox) {
    delete m_currentLightbox;
    m_currentLightbox = nullptr;
  }

  if (m_httpClient) {
    m_httpClient->deleteLater();
    m_httpClient = nullptr;
  }

  m_httpClient = new DwarfHttpClient(ip, this);
  connect(m_httpClient, &DwarfHttpClient::mediaListReceived, this,
          &MainWindow::onMediaListReceived);
  connect(m_httpClient, &DwarfHttpClient::errorOccurred, this,
          &MainWindow::onMediaListError);

  if (m_openGalleryButton)
    m_openGalleryButton->setEnabled(false);
  if (m_mediaPhotoList)
    m_mediaPhotoList->clear();
  if (m_mediaVideoList)
    m_mediaVideoList->clear();
  if (m_mediaBurstList)
    m_mediaBurstList->clear();
  if (m_mediaAstroList)
    m_mediaAstroList->clear();
  if (m_mediaPanoList)
    m_mediaPanoList->clear();

  m_httpClient->fetchMediaList();
}

void MainWindow::onMediaListReceived(const QJsonDocument &document) {
  if (m_openGalleryButton)
    m_openGalleryButton->setEnabled(true);

  QJsonArray files;
  if (document.isArray())
    files = document.array();
  else
    files = document.object().value(QStringLiteral("data")).toArray();

  // If we are waiting for a capture filename, try to resolve it from this list.
  if (m_pendingCaptureLookup.active && !files.isEmpty()) {
    QString bestName;
    qint64 bestTs = -1;
    QString bestThumb;

    const bool wantTele = (m_pendingCaptureLookup.expectedKind ==
                           DwarfCameraController::CameraKind::Tele);

    for (const QJsonValue &v : files) {
      QJsonObject obj = v.toObject();
      const int mediaType = obj.value(QStringLiteral("mediaType")).toInt(-1);
      if (mediaType != m_pendingCaptureLookup.expectedMediaType)
        continue;

      QString filePath = obj.value(QStringLiteral("filePath")).toString();
      QString fileNameField = obj.value(QStringLiteral("fileName")).toString();

      QString baseName;
      if (!filePath.isEmpty()) {
        QFileInfo fi(filePath);
        baseName = fi.fileName();
      }
      if (baseName.isEmpty())
        baseName = fileNameField;

      // Try to infer Tele/Wide
      const int camId = obj.value(QStringLiteral("camId")).toInt(-1);
      bool matchesKind = false;
      if (camId != -1) {
        // Firmware varies; don't hard-fail on camId, but use as hint.
        if (wantTele)
          matchesKind = (camId == 1);
        else
          matchesKind = (camId == 2);
      }
      if (!matchesKind && !baseName.isEmpty()) {
        if (wantTele)
          matchesKind = baseName.contains("TELE", Qt::CaseInsensitive);
        else
          matchesKind = baseName.contains("WIDE", Qt::CaseInsensitive);
      }

      // Timestamp heuristics (field names vary by firmware)
      qint64 ts = -1;
      if (obj.contains(QStringLiteral("createTime")))
        ts = static_cast<qint64>(obj.value(QStringLiteral("createTime")).toDouble(-1));
      else if (obj.contains(QStringLiteral("create_time")))
        ts = static_cast<qint64>(obj.value(QStringLiteral("create_time")).toDouble(-1));
      else if (obj.contains(QStringLiteral("timestamp")))
        ts = static_cast<qint64>(obj.value(QStringLiteral("timestamp")).toDouble(-1));

      if (baseName.isEmpty())
        continue;

      // Prefer entries matching kind; otherwise still allow newest as fallback.
      const int score = matchesKind ? 1 : 0;
      const int bestScore = (!bestName.isEmpty() &&
                             ((wantTele && bestName.contains("TELE", Qt::CaseInsensitive)) ||
                              (!wantTele && bestName.contains("WIDE", Qt::CaseInsensitive))))
                                ? 1
                                : 0;

      const bool choose = bestName.isEmpty() ||
                          (score > bestScore) ||
                          (score == bestScore && ts != -1 && bestTs != -1 && ts > bestTs);

      if (choose) {
        bestName = baseName;
        bestTs = ts;
        // Preview source: thumbnailPath preferred, fallback to filePath
        bestThumb = obj.value(QStringLiteral("thumbnailPath")).toString();
        if (bestThumb.isEmpty())
          bestThumb = filePath;
      }
    }

    if (!bestName.isEmpty()) {
      setCaptureStatusTextAllPanels(
          QStringLiteral("%1 %2").arg(m_pendingCaptureLookup.prefix, bestName));

      // Try to show a preview thumbnail under the filename (photo only).
      if (m_pendingCaptureLookup.expectedMediaType == 1 && !bestThumb.isEmpty()) {
        QString ip = m_ipInput ? m_ipInput->text().trimmed() : QString();
        if (!ip.isEmpty()) {
          if (!m_ftpDownloader) {
            m_ftpDownloader = new DwarfFtpDownloader(this);
            connect(m_ftpDownloader, &DwarfFtpDownloader::downloadStarted, this,
                    &MainWindow::onDownloadStarted);
            connect(m_ftpDownloader, &DwarfFtpDownloader::downloadFinished, this,
                    &MainWindow::onDownloadFinished);
            connect(m_ftpDownloader, &DwarfFtpDownloader::downloadError, this,
                    &MainWindow::onDownloadError);
          }

          m_ftpDownloader->downloadThumbnail(
              ip, bestThumb,
              [this](const QByteArray &data) {
                if (data.isEmpty())
                  return;
                QImage img;
                if (!img.loadFromData(data))
                  return;
                setCapturePreviewAllPanels(m_cameraSettingsPanel, m_paramsOverlay,
                                           QPixmap::fromImage(img));
              });
        }
      }

      m_pendingCaptureLookup.active = false;
      m_pendingCaptureLookup.attempts = 0;
    } else {
      // Long exposures can take time before the file appears in the album.
      // Poll a few times before giving up.
      if (m_pendingCaptureLookup.attempts < 20) {
        m_pendingCaptureLookup.attempts++;
        if (m_httpClient) {
          QTimer::singleShot(1000, this, [this]() {
            if (m_httpClient && m_pendingCaptureLookup.active)
              m_httpClient->fetchMediaList();
          });
        }
      }
    }
  }

  if (!m_mediaTabs)
    return;

  // Delete any open lightbox before clearing lists - must delete immediately
  if (m_currentLightbox) {
    delete m_currentLightbox;
    m_currentLightbox = nullptr;
  }

  // Clear pending thumbnail downloads to avoid stale references
  m_pendingThumbnails.clear();
  m_thumbnailsLoading = 0;

  if (m_mediaPhotoList)
    m_mediaPhotoList->clear();
  if (m_mediaVideoList)
    m_mediaVideoList->clear();
  if (m_mediaBurstList)
    m_mediaBurstList->clear();
  if (m_mediaAstroList)
    m_mediaAstroList->clear();
  if (m_mediaPanoList)
    m_mediaPanoList->clear();

  if (files.isEmpty()) {
    if (m_mediaPhotoList)
      m_mediaPhotoList->addItem(tr("No media found"));
    return;
  }

  for (const QJsonValue &v : files) {
    QJsonObject obj = v.toObject();
    QString filePath = obj.value(QStringLiteral("filePath")).toString();
    QString fileNameField = obj.value(QStringLiteral("fileName")).toString();

    QString baseName;
    if (!filePath.isEmpty()) {
      QFileInfo fi(filePath);
      baseName = fi.fileName();
    }
    if (baseName.isEmpty())
      baseName = fileNameField;

    if (baseName.isEmpty()) {
      baseName = obj.value(QStringLiteral("name")).toString();
      if (baseName.isEmpty())
        baseName = obj.value(QStringLiteral("filename")).toString();
      if (baseName.isEmpty())
        baseName = obj.value(QStringLiteral("file")).toString();
      if (baseName.isEmpty())
        baseName = obj.value(QStringLiteral("path")).toString();
    }

    int mediaType = obj.value(QStringLiteral("mediaType")).toInt(-1);
    int camId = obj.value(QStringLiteral("camId")).toInt(-1);

    // Display name: just the filename (Tele/Wide is already in the name)
    QString display = baseName;
    if (display.isEmpty())
      display =
          QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));

    QListWidget *target = nullptr;
    switch (mediaType) {
    case 1:
      target = m_mediaPhotoList;
      break;
    case 2:
      target = m_mediaVideoList;
      break;
    case 3:
      target = m_mediaBurstList;
      break;
    case 4:
      target = m_mediaAstroList;
      break;
    case 5:
      target = m_mediaPanoList;
      break;
    default:
      target = nullptr;
      break;
    }

    if (target) {
      QListWidgetItem *item = new QListWidgetItem(display, target);
      item->setData(Qt::UserRole, obj);
      item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);

      // Queue thumbnail for loading - store safe reference, not raw pointer
      QString thumbnailPath =
          obj.value(QStringLiteral("thumbnailPath")).toString();
      if (!thumbnailPath.isEmpty()) {
        PendingThumbnail pending;
        pending.list = target;
        pending.row = target->count() - 1; // Just added item
        pending.path = thumbnailPath;
        m_pendingThumbnails.append(pending);
      } else {
        qDebug() << "[MainWindow] No thumbnail for:" << baseName
                 << "mediaType:" << mediaType;
      }
    }
  }

  // Configure list widgets for icon mode with grid layout
  // Thumbnails are landscape (16:9), so height is ~67% of width
  int thumbHeight = THUMBNAIL_SIZE * 2 / 3; // ~80px for 120px width
  for (QListWidget *list :
       {m_mediaPhotoList, m_mediaVideoList, m_mediaBurstList, m_mediaAstroList,
        m_mediaPanoList}) {
    if (list) {
      list->setViewMode(QListView::IconMode);
      list->setIconSize(QSize(THUMBNAIL_SIZE, thumbHeight));
      list->setGridSize(QSize(THUMBNAIL_SIZE + 20, thumbHeight + 45));
      list->setSpacing(6);
      list->setResizeMode(QListView::Adjust);
      list->setWordWrap(true);
      list->setUniformItemSizes(false);
      list->setMovement(QListView::Static);
    }
  }

  // Start loading thumbnails
  loadThumbnails();
}

void MainWindow::loadThumbnails() {
  if (m_pendingThumbnails.isEmpty())
    return;

  QString ip = m_ipInput ? m_ipInput->text().trimmed() : QString();
  if (ip.isEmpty())
    return;

  if (!m_ftpDownloader) {
    m_ftpDownloader = new DwarfFtpDownloader(this);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadStarted, this,
            &MainWindow::onDownloadStarted);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadFinished, this,
            &MainWindow::onDownloadFinished);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadError, this,
            &MainWindow::onDownloadError);
  }

  // Load thumbnails in batches (max 5 concurrent)
  while (!m_pendingThumbnails.isEmpty() && m_thumbnailsLoading < 5) {
    PendingThumbnail pending = m_pendingThumbnails.takeFirst();

    // Skip if list was deleted or row is invalid
    if (!pending.list || pending.row < 0 ||
        pending.row >= pending.list->count())
      continue;

    if (pending.path.isEmpty())
      continue;

    m_thumbnailsLoading++;

    // Capture safe references for callback
    QPointer<QListWidget> capturedList = pending.list;
    int capturedRow = pending.row;
    QString capturedPath = pending.path;

    m_ftpDownloader->downloadThumbnail(
        ip, pending.path,
        [this, capturedList, capturedRow,
         capturedPath](const QByteArray &data) {
          if (m_thumbnailsLoading > 0)
            m_thumbnailsLoading--;

          if (!data.isEmpty() && capturedList && capturedRow >= 0 &&
              capturedRow < capturedList->count()) {
            QListWidgetItem *currentItem = capturedList->item(capturedRow);
            if (currentItem)
              setItemThumbnail(currentItem, data);
          }

          // Continue loading more thumbnails
          QTimer::singleShot(50, this, &MainWindow::loadThumbnails);
        });
  }
}

void MainWindow::setItemThumbnail(QListWidgetItem *item,
                                  const QByteArray &data) {
  if (!item || data.isEmpty())
    return;

  QImage image;
  if (image.loadFromData(data)) {
    // Scale to fit width, keep aspect ratio (landscape thumbnails)
    int thumbHeight = THUMBNAIL_SIZE * 2 / 3;
    QPixmap pixmap = QPixmap::fromImage(
        image.scaled(THUMBNAIL_SIZE, thumbHeight, Qt::KeepAspectRatio,
                     Qt::SmoothTransformation));
    item->setIcon(QIcon(pixmap));
  } else {
    qWarning() << "[MainWindow] Failed to decode thumbnail image, size:"
               << data.size();
  }
}

void MainWindow::onChangeDownloadDirClicked() {
  QString startDir = m_downloadDir;
  if (startDir.isEmpty()) {
    startDir =
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (startDir.isEmpty())
      startDir = QDir::homePath();
  }

  QString dir = QFileDialog::getExistingDirectory(
      this, tr("Select download folder"), startDir,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (dir.isEmpty())
    return;

  m_downloadDir = dir;
  if (m_downloadDirEdit)
    m_downloadDirEdit->setText(dir);

  QSettings settings("DwarfLab", "DwarfController");
  settings.setValue("downloadDir", dir);
}

void MainWindow::onMediaItemClicked(QListWidgetItem *item) {
  if (!item)
    return;

  // Find which list widget this item belongs to
  QListWidget *listWidget = item->listWidget();
  if (!listWidget)
    return;

  int currentIndex = listWidget->row(item);
  if (currentIndex < 0)
    return;

  const int count = listWidget->count();
  if (count <= 0)
    return;

  // Build media list and thumbnails from the whole QListWidget
  QVector<QJsonObject> mediaList(count);
  QVector<QPixmap> thumbnails(count);
  for (int i = 0; i < count; ++i) {
    QListWidgetItem *it = listWidget->item(i);
    if (!it)
      continue;

    QVariant d = it->data(Qt::UserRole);
    if (d.isValid())
      mediaList[i] = d.toJsonObject();

    QIcon ic = it->icon();
    if (!ic.isNull())
      thumbnails[i] = ic.pixmap(500, 350);
  }

  if (mediaList.isEmpty())
    return;

  // Delete any existing lightbox first
  if (m_currentLightbox) {
    delete m_currentLightbox;
    m_currentLightbox = nullptr;
  }

  MediaLightbox *lightbox = new MediaLightbox(this);
  lightbox->setMediaList(mediaList, currentIndex, thumbnails);
  m_currentLightbox = lightbox;

  connect(lightbox, &MediaLightbox::downloadRequested, this,
          [this](const QJsonObject &mediaInfo) {
            // Create a temporary item to trigger download
            QListWidgetItem tempItem;
            tempItem.setData(Qt::UserRole, mediaInfo);
            onMediaItemActivated(&tempItem);
          });

  connect(
      lightbox, &MediaLightbox::deleteRequested, this,
      [this, lightbox](const QJsonObject &mediaInfo) {
        QString filePath =
            mediaInfo.value(QStringLiteral("filePath")).toString();
        QString fileName =
            mediaInfo.value(QStringLiteral("fileName")).toString();
        if (fileName.isEmpty()) {
          QFileInfo fi(filePath);
          fileName = fi.fileName();
        }

        // Confirm deletion
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("Delete Media"),
            tr("Are you sure you want to delete '%1' from the DWARF II?")
                .arg(fileName),
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
          statusBar()->showMessage(tr("Deleting %1...").arg(fileName), 0);

          // Try HTTP delete first (network)
          if (m_httpClient) {
            auto deleteContext = new QObject(this);
            
            // HTTP Success handler
            connect(m_httpClient, &DwarfHttpClient::mediaDeleted, deleteContext,
                    [this, fileName, lightbox, deleteContext](const QString &path) {
                      Q_UNUSED(path);
                      statusBar()->showMessage(tr("Deleted %1 via HTTP").arg(fileName), 3000);
                      if (lightbox) {
                        lightbox->close();
                      }
                      if (m_httpClient) {
                        m_httpClient->fetchMediaList();
                      }
                      deleteContext->deleteLater();
                    });

            // HTTP Error handler - fallback to MTP
            connect(m_httpClient, &DwarfHttpClient::deleteError, deleteContext,
                    [this, fileName, filePath, lightbox, deleteContext](const QString &path, const QString &error) {
                      Q_UNUSED(path);
                      
                      // Check if error is "not implemented"
                      if (error.contains("not implemented", Qt::CaseInsensitive)) {
                        qDebug() << "[Delete] HTTP not supported, trying MTP...";
                        statusBar()->showMessage(tr("HTTP delete not supported, trying USB/MTP..."), 0);
                        
                        // Initialize MTP client if needed
                        if (!m_mtpClient) {
                          m_mtpClient = new DwarfMtpClient(this);
                        }
                        
                        if (!m_mtpClient->isSupported()) {
                          statusBar()->showMessage(tr("MTP not supported on this platform"), 5000);
                          QMessageBox::warning(this, tr("Delete Failed"),
                                             tr("Neither HTTP nor MTP delete is supported.\n\n"
                                                "Please delete files using the official DWARF app."));
                          deleteContext->deleteLater();
                          return;
                        }
                        
                        // Check if MTP tools are available
                        if (!m_mtpClient->checkTools()) {
                          statusBar()->showMessage(tr("MTP tools not installed"), 5000);
                          
                          QString installMsg;
#ifdef Q_OS_LINUX
                          installMsg = tr("Install MTP tools with:\nsudo apt-get install mtp-tools");
#elif defined(Q_OS_MACOS)
                          installMsg = tr("Install MTP tools with:\nbrew install libmtp");
#else
                          installMsg = tr("Connect DWARF II via USB and try again.");
#endif
                          
                          QMessageBox::information(this, tr("MTP Tools Required"),
                                                 tr("MTP delete requires USB connection and tools.\n\n%1")
                                                   .arg(installMsg));
                          deleteContext->deleteLater();
                          return;
                        }
                        
                        // Try MTP delete
                        m_mtpClient->deleteFile(filePath, 
                          [this, fileName, lightbox, deleteContext](bool success, const QString &mtpError) {
                            if (success) {
                              statusBar()->showMessage(tr("Deleted %1 via MTP").arg(fileName), 3000);
                              if (lightbox) {
                                lightbox->close();
                              }
                              if (m_httpClient) {
                                m_httpClient->fetchMediaList();
                              }
                            } else {
                              statusBar()->showMessage(tr("MTP delete failed: %1").arg(mtpError), 5000);
                              QMessageBox::warning(this, tr("Delete Failed"),
                                                 tr("Could not delete '%1' via MTP:\n%2\n\n"
                                                    "Make sure DWARF II is connected via USB.")
                                                   .arg(fileName, mtpError));
                            }
                            deleteContext->deleteLater();
                          });
                        
                      } else {
                        // Other HTTP error
                        statusBar()->showMessage(tr("HTTP delete failed: %1").arg(error), 5000);
                        QMessageBox::warning(this, tr("Delete Failed"),
                                           tr("Could not delete '%1' via HTTP:\n%2")
                                             .arg(fileName, error));
                        deleteContext->deleteLater();
                      }
                    });

            m_httpClient->deleteMedia(filePath);
            
          } else {
            QMessageBox::warning(this, tr("Delete Failed"),
                               tr("Not connected to DWARF II."));
          }
        }
      });

  lightbox->show();
}

void MainWindow::onMediaItemActivated(QListWidgetItem *item) {
  if (!item)
    return;

  QVariant data = item->data(Qt::UserRole);
  if (!data.isValid())
    return;

  QJsonObject obj = data.toJsonObject();
  QString filePath = obj.value(QStringLiteral("filePath")).toString();
  QString fileName = obj.value(QStringLiteral("fileName")).toString();
  int mediaType = obj.value(QStringLiteral("mediaType")).toInt(0);

  if (fileName.isEmpty()) {
    QFileInfo fi(filePath);
    fileName = fi.fileName();
  }

  if (filePath.isEmpty()) {
    statusBar()->showMessage(tr("No file path available"), 3000);
    return;
  }

  QString ip = m_ipInput ? m_ipInput->text().trimmed() : QString();
  if (ip.isEmpty()) {
    statusBar()->showMessage(tr("No DWARF IP configured"), 3000);
    return;
  }

  if (m_downloadDir.isEmpty()) {
    statusBar()->showMessage(tr("No download folder configured"), 3000);
    return;
  }

  // Create subdirectory based on media type
  QString subDir;
  switch (mediaType) {
  case 1:
    subDir = QStringLiteral("Photos");
    break;
  case 2:
    subDir = QStringLiteral("Videos");
    break;
  case 3:
    subDir = QStringLiteral("Burst");
    break;
  case 4:
    subDir = QStringLiteral("Astro");
    break;
  case 5:
    subDir = QStringLiteral("Panorama");
    break;
  default:
    subDir = QStringLiteral("Other");
    break;
  }

  QString targetDir = QDir(m_downloadDir).filePath(subDir);
  QDir().mkpath(targetDir);

  if (!m_ftpDownloader) {
    m_ftpDownloader = new DwarfFtpDownloader(this);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadStarted, this,
            &MainWindow::onDownloadStarted);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadFinished, this,
            &MainWindow::onDownloadFinished);
    connect(m_ftpDownloader, &DwarfFtpDownloader::downloadError, this,
            &MainWindow::onDownloadError);
  }

  m_ftpDownloader->downloadFile(ip, filePath, targetDir);
}

void MainWindow::onDownloadStarted(const QString &fileName) {
  statusBar()->showMessage(tr("Downloading %1...").arg(fileName));
}

void MainWindow::onDownloadFinished(const QString &fileName,
                                    const QString &localPath) {
  Q_UNUSED(localPath);
  statusBar()->showMessage(tr("Downloaded %1").arg(fileName), 5000);
}

void MainWindow::onDownloadError(const QString &fileName,
                                 const QString &error) {
  statusBar()->showMessage(tr("Download failed: %1 - %2").arg(fileName, error),
                           5000);
}

void MainWindow::onMediaListError(const QString &error) {
  if (m_openGalleryButton)
    m_openGalleryButton->setEnabled(true);
  if (!m_mediaPhotoList)
    return;

  m_mediaPhotoList->addItem(tr("Error: %1").arg(error));
}

void MainWindow::onPipStreamClicked() {
  qWarning() << "[MainWindow] PiP double-click: BEFORE switch mainStream="
             << (m_mainStream == CameraStream::Tele ? "Tele" : "Wide")
             << "pipStream="
             << (m_pipStream == CameraStream::Tele ? "Tele" : "Wide");

  std::swap(m_mainStream, m_pipStream);

  qWarning() << "[MainWindow] PiP double-click: AFTER swap mainStream="
             << (m_mainStream == CameraStream::Tele ? "Tele" : "Wide")
             << "pipStream="
             << (m_pipStream == CameraStream::Tele ? "Tele" : "Wide");

  updateCameraStreamViews();
  qWarning()
      << "[MainWindow] PiP double-click: updateCameraStreamViews finished";
}

void MainWindow::updateOverlayPositions() {
  if (m_mainVideoWidget && m_motorOverlay && m_mainStreamView) {
    // Calculate position relative to central widget (including sidebar offset)
    QPoint videoPos = m_mainStreamView->mapTo(centralWidget(), QPoint(0, 0));
    int videoWidth = m_mainVideoWidget->width();
    
    // Position motor overlay on right side of the video area
    int overlayX = videoPos.x() + videoWidth - m_motorOverlay->width() - 20;
    int overlayY = videoPos.y() + 20;
    
    // Ensure it doesn't go off screen left
    if (overlayX < 0) overlayX = 20;
    
    m_motorOverlay->move(overlayX, overlayY);
    m_motorOverlay->raise();
  }

  if (m_mainVideoWidget && m_paramsOverlay && m_mainStreamView) {
    const QPoint videoPos = m_mainStreamView->mapTo(centralWidget(), QPoint(0, 0));
    const int videoWidth = m_mainVideoWidget->width();
    const int videoHeight = m_mainVideoWidget->height();

    // Default: under motor overlay (right-aligned)
    int overlayX = videoPos.x() + videoWidth - m_paramsOverlay->width() - 20;
    int overlayY = videoPos.y() + 20;
    if (m_motorOverlay) {
      overlayX = m_motorOverlay->x();
      overlayY = m_motorOverlay->y() + m_motorOverlay->height() + 12;
    }

    // Bounds inside the stream area
    const int minX = videoPos.x() + 20;
    const int minY = videoPos.y() + 20;
    const int maxX = videoPos.x() + videoWidth - m_paramsOverlay->width() - 20;
    const int maxY = videoPos.y() + videoHeight - m_paramsOverlay->height() - 20;

    // qBound asserts if (max < min), which can happen when the overlay is
    // larger than the available stream area (e.g. small window).
    if (maxX < minX)
      overlayX = minX;
    else
      overlayX = qBound(minX, overlayX, maxX);

    if (maxY < minY)
      overlayY = minY;
    else
      overlayY = qBound(minY, overlayY, maxY);

    m_paramsOverlay->move(overlayX, overlayY);
    m_paramsOverlay->raise();
  }

  if (m_starMapOverlayContainer && m_mainStreamView) {
    QPoint streamPos = m_mainStreamView->mapTo(centralWidget(), QPoint(0, 0));
    m_starMapOverlayContainer->move(streamPos);
    m_starMapOverlayContainer->resize(m_mainStreamView->size());
    if (m_starMapOverlayEnabled) {
      m_starMapOverlayContainer->raise();
    }
  }

  if (m_astroTabsOverlayContainer && m_mainStreamView) {
    const QPoint streamPos = m_mainStreamView->mapTo(centralWidget(), QPoint(0, 0));
    const int tabW = m_astroTabsOverlayBar ? m_astroTabsOverlayBar->sizeHint().width() : 320;
    const int tabH = m_astroTabsOverlayBar ? m_astroTabsOverlayBar->sizeHint().height() : 32;
    m_astroTabsOverlayContainer->move(streamPos + QPoint(10, 10));
    m_astroTabsOverlayContainer->resize(tabW, tabH);
    if (m_astroTabsOverlayContainer->isVisible()) {
      m_astroTabsOverlayContainer->raise();
    }
  }

  if (m_galleryOverlayContainer && m_mainStreamView) {
    const QPoint streamPos = m_mainStreamView->mapTo(centralWidget(), QPoint(0, 0));
    m_galleryOverlayContainer->move(streamPos);
    m_galleryOverlayContainer->resize(m_mainStreamView->size());
    if (m_galleryOverlayEnabled)
      m_galleryOverlayContainer->raise();
  }
}

void MainWindow::onStarMapOverlayRequested(bool enabled) {
  m_starMapOverlayEnabled = enabled;

  updateOverlayVisibility();

  if (!m_starMapOverlayContainer || !m_astroPanel)
    return;

  QWidget *starMapContent = m_astroPanel->starMapContentWidget();
  if (!starMapContent)
    return;

  auto *overlayLayout = qobject_cast<QVBoxLayout *>(m_starMapOverlayContainer->layout());
  if (!overlayLayout) {
    overlayLayout = new QVBoxLayout(m_starMapOverlayContainer);
    overlayLayout->setContentsMargins(0, 0, 0, 0);
    overlayLayout->setSpacing(0);
  }

  const bool astroSelected = m_contentStack && (m_contentStack->currentWidget() == m_astroPanel);
  if (m_astroTabsOverlayContainer) {
    m_astroTabsOverlayContainer->setVisible(astroSelected && enabled);
    if (astroSelected && enabled)
      m_astroTabsOverlayContainer->raise();
  }

  if (enabled) {
    if (m_contentStack)
      m_contentStack->setVisible(false);

    if (auto *starMapTabLayout = m_astroPanel->starMapTabLayout()) {
      starMapTabLayout->removeWidget(starMapContent);
    }

    starMapContent->setParent(m_starMapOverlayContainer);
    overlayLayout->addWidget(starMapContent);
    starMapContent->show();

    m_starMapOverlayContainer->setVisible(true);
    updateOverlayPositions();
    m_starMapOverlayContainer->raise();
    if (m_astroTabsOverlayContainer && m_astroTabsOverlayContainer->isVisible())
      m_astroTabsOverlayContainer->raise();
    return;
  }

  // Restore
  overlayLayout->removeWidget(starMapContent);
  starMapContent->setParent(m_astroPanel->starMapTabWidget());
  if (auto *starMapTabLayout = m_astroPanel->starMapTabLayout()) {
    starMapTabLayout->insertWidget(0, starMapContent, 1);
  }
  starMapContent->show();

  m_starMapOverlayContainer->setVisible(false);
  if (m_contentStack && !m_galleryOverlayEnabled)
    m_contentStack->setVisible(true);

  updateOverlayVisibility();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  updateOverlayPositions();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_mainVideoWidget && event->type() == QEvent::Resize) {
    updateOverlayPositions();
  }
  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onCameraTeleMessage(uint32_t cmd, const QByteArray &data) {
  qWarning() << "[MainWindow] onCameraTeleMessage cmd" << cmd << "data size"
             << data.size();

  // Forward to camera controller for parameter handling
  if (m_cameraController) {
    m_cameraController->handleCameraMessage(1, cmd, data);
  }

  if (cmd == 10000) { // CMD_CAMERA_TELE_OPEN_CAMERA
    dwarf::ComResponse res;
    if (res.ParseFromArray(data.data(), data.size())) {
      if (res.code() == 0 || res.code() == 374) {
        qDebug() << "Tele camera opened (code" << res.code()
                 << "), starting MJPEG stream...";
        QString ip = m_ipInput->text().trimmed();
        // Use MJPEG stream on port 8092 instead of RTSP on 554
        // RTSP port 554 is often closed/refused, while MJPEG works in Python
        // legacy app
        const QUrl teleUrl(QStringLiteral("http://%1:8092/mainstream").arg(ip));
        if (m_teleStream) {
          m_teleStream->start(teleUrl);
        }
        updateStreamRouting();
        // Ensure overlays stay on top after video starts
        if (m_streamNameOverlay)
          m_streamNameOverlay->raise();
        if (m_pipContainer)
          m_pipContainer->raise();

        // Fetch current camera parameters to sync UI
        if (m_cameraController) {
          m_cameraController->fetchAllParams(
              DwarfCameraController::CameraKind::Tele);
        }
      } else {
        qWarning() << "Failed to open Tele camera, code:" << res.code();
      }
    } else {
      qWarning() << "[MainWindow] Failed to parse Tele ComResponse for cmd"
                 << cmd;
    }
  }
}

void MainWindow::onCameraWideMessage(uint32_t cmd, const QByteArray &data) {
  qWarning() << "[MainWindow] onCameraWideMessage cmd" << cmd << "data size"
             << data.size();

  // Forward to camera controller for parameter handling
  if (m_cameraController) {
    m_cameraController->handleCameraMessage(2, cmd, data);
  }

  if (cmd == 12000) { // CMD_CAMERA_WIDE_OPEN_CAMERA
    dwarf::ComResponse res;
    if (res.ParseFromArray(data.data(), data.size())) {
      if (res.code() == 0 || res.code() == 374) {
        qDebug() << "Wide camera opened (code" << res.code()
                 << "), starting MJPEG stream...";
        QString ip = m_ipInput->text().trimmed();
        // Use MJPEG stream on port 8092 instead of RTSP on 554
        const QUrl wideUrl(
            QStringLiteral("http://%1:8092/secondstream").arg(ip));
        if (m_wideStream) {
          m_wideStream->start(wideUrl);
        }
        updateStreamRouting();
        // Ensure overlays stay on top after video starts
        if (m_streamNameOverlay)
          m_streamNameOverlay->raise();
        if (m_pipContainer)
          m_pipContainer->raise();

        // Fetch current camera parameters to sync UI
        if (m_cameraController) {
          m_cameraController->fetchAllParams(
              DwarfCameraController::CameraKind::Wide);
        }
      } else {
        qWarning() << "Failed to open Wide camera, code:" << res.code();
      }
    } else {
      qWarning() << "[MainWindow] Failed to parse Wide ComResponse for cmd"
                 << cmd;
    }
  }
}
