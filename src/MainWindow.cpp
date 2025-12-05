#include "MainWindow.h"
#include "net/DwarfCameraController.h"
#include "net/DwarfMotorController.h"
#include "net/DwarfFocusController.h"
#include "net/DwarfMjpegStream.h"
#include "net/DwarfMjpegView.h"
#include "net/DwarfHttpClient.h"
#include "net/DwarfFtpDownloader.h"
#include "ui/MediaLightbox.h"
#include "ui/CameraSettingsPanel.h"
#include "qnamespace.h"
#include <QDebug>
#include <QDockWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>
#include <QPixmap>
#include <QImage>
#include <QListView>
#include <QVector>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_wsClient(nullptr), m_dispatcher(nullptr),
      m_scanCancelled(false), m_cameraController(nullptr),
      m_motorController(nullptr), m_focusController(nullptr),
      m_mainVideoWidget(nullptr), m_pipVideoWidget(nullptr),
      m_cameraSettingsPanel(nullptr),
      m_teleStream(nullptr), m_wideStream(nullptr),
      m_recordTimer(nullptr),
      m_httpClient(nullptr), m_openGalleryButton(nullptr),
      m_mediaTabs(nullptr), m_mediaPhotoList(nullptr),
      m_mediaVideoList(nullptr), m_mediaBurstList(nullptr),
      m_mediaAstroList(nullptr), m_mediaPanoList(nullptr),
      m_downloadDirEdit(nullptr), m_changeDownloadDirButton(nullptr),
      m_ftpDownloader(nullptr), m_thumbnailsLoading(0) {
  m_mainStreamView = nullptr;
  m_pipStreamView = nullptr;

  m_recordTimer = new QTimer(this);
  m_recordTimer->setInterval(500);
  m_mainStream = CameraStream::Tele;
  m_pipStream = CameraStream::Wide;
  m_cameraController = new DwarfCameraController(this);
  m_motorController = new DwarfMotorController(this);
  m_focusController = new DwarfFocusController(this);
  m_teleStream = new DwarfMjpegStream(this);
  m_wideStream = new DwarfMjpegStream(this);
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

  setupUi();
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
  if (!m_mainStreamView || !m_pipStreamView)
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
  }

  // Ensure overlays stay on top
  m_streamNameOverlay->raise();
  m_pipStreamView->raise();

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
  resize(1280, 720);

  // Central Widget
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

  QWidget *viewportWidget = new QWidget(centralWidget);
  QGridLayout *viewportLayout = new QGridLayout(viewportWidget);
  viewportLayout->setContentsMargins(0, 0, 0, 0);

  m_mainStreamView = new QWidget(centralWidget);
  m_mainStreamView->setObjectName("mainStreamView");
  m_mainStreamView->setMinimumHeight(400);

  // Create Main Video Widget first (custom MJPEG view)
  m_mainVideoWidget = new DwarfMjpegView(m_mainStreamView);
  QVBoxLayout *mainVideoLayout = new QVBoxLayout(m_mainStreamView);
  mainVideoLayout->setContentsMargins(0, 0, 0, 0);
  mainVideoLayout->addWidget(m_mainVideoWidget);
  m_mainVideoWidget->show();

  // Overlay Label for Stream Name - Parented to Video Widget
  m_streamNameOverlay = new QLabel(m_mainVideoWidget);
  m_streamNameOverlay->setObjectName("streamNameOverlay");
  // m_streamNameOverlay->setAttribute(Qt::WA_NativeWindow); // Removed
  m_streamNameOverlay->setStyleSheet(
      "QLabel { color: white; font-size: 16px; font-weight: bold; "
      "background-color: rgba(0, 0, 0, 100); padding: 4px 8px; border-radius: "
      "4px; }");
  m_streamNameOverlay->setAlignment(Qt::AlignCenter);

  // PiP View - Parented to Video Widget
  m_pipStreamView = new ClickableLabel(m_mainVideoWidget);
  m_pipStreamView->setObjectName("pipStreamView");
  // m_pipStreamView->setAttribute(Qt::WA_NativeWindow); // Removed
  m_pipStreamView->setFixedSize(220, 124);

  // Layout for Overlays on top of Main Video
  QGridLayout *overlayLayout = new QGridLayout(m_mainVideoWidget);
  overlayLayout->setContentsMargins(10, 10, 10, 10);
  overlayLayout->addWidget(m_streamNameOverlay, 0, 0,
                           Qt::AlignTop | Qt::AlignHCenter);
  overlayLayout->addWidget(m_pipStreamView, 0, 0, Qt::AlignTop | Qt::AlignLeft);
  // Add a stretch to push everything up
  overlayLayout->setRowStretch(1, 1);

  // PiP Video Widget inside PiP View (custom MJPEG view)
  m_pipVideoWidget = new DwarfMjpegView(m_pipStreamView);
  m_pipVideoWidget->setAttribute(Qt::WA_TransparentForMouseEvents, true);
  // m_pipVideoWidget->setStyleSheet("background-color: black; border: none;");
  QVBoxLayout *pipVideoLayout = new QVBoxLayout(m_pipStreamView);
  pipVideoLayout->setContentsMargins(3, 3, 3, 3);
  pipVideoLayout->addWidget(m_pipVideoWidget);
  m_pipVideoWidget->show();

  viewportLayout->addWidget(m_mainStreamView, 0, 0);
  // Overlays are now inside m_mainStreamView -> m_mainVideoWidget, so we don't
  // add them to viewportLayout

  viewportWidget->setLayout(viewportLayout);
  mainLayout->addWidget(viewportWidget);

  // Ensure overlays are on top (though parenting should handle this)
  m_streamNameOverlay->raise();
  m_pipStreamView->raise();

  m_pipStreamView->setStyleSheet(
      "border: 2px solid white; background-color: black;");

  connect(m_pipStreamView, &ClickableLabel::clicked, this,
          &MainWindow::onPipStreamClicked);

  if (m_mainVideoWidget)
    connect(m_mainVideoWidget, &DwarfMjpegView::pointClicked, this,
            &MainWindow::onMainViewPointClicked);

  // Device List (Hidden by default or shown?)
  // Let's show it always for now, or maybe collapsible.
  // User wants to see found dwarfs.
  m_deviceList = new QListWidget(this);
  m_deviceList->setMaximumHeight(100);
  connect(m_deviceList, &QListWidget::itemClicked, this,
          &MainWindow::onDeviceSelected);

  // Grid Layout for Controls
  QGridLayout *gridLayout = new QGridLayout();
  gridLayout->setColumnStretch(1, 1); // Make input column stretch

  // Row 0: Scan
  QLabel *subnetLabel = new QLabel(tr("Scan Subnet:"), this);
  m_subnetInput = new QLineEdit(this);
  // Pre-fill with detected subnet
  QStringList subnets = m_finder->getLocalSubnets();
  if (!subnets.isEmpty()) {
    m_subnetInput->setText(subnets.first());
  } else {
    m_subnetInput->setText("192.168.88");
  }
  m_subnetInput->setPlaceholderText(tr("e.g. 192.168.1"));
  connect(m_subnetInput, &QLineEdit::textChanged, this,
          &MainWindow::onSubnetTextChanged);

  m_scanButton = new QPushButton(tr("Scan"), this);
  connect(m_scanButton, &QPushButton::clicked, this,
          &MainWindow::onScanClicked);

  m_cancelScanButton = new QPushButton(tr("Cancel"), this);
  m_cancelScanButton->setEnabled(false);
  connect(m_cancelScanButton, &QPushButton::clicked, this,
          &MainWindow::onCancelScanClicked);

  gridLayout->addWidget(subnetLabel, 0, 0);
  gridLayout->addWidget(m_subnetInput, 0, 1);
  gridLayout->addWidget(m_scanButton, 0, 2);
  gridLayout->addWidget(m_cancelScanButton, 0, 3);

  // Row 1: Connect
  QLabel *ipLabel = new QLabel(tr("DWARF II IP:"), this);
  m_ipInput = new QLineEdit(this);
  m_ipInput->setText("192.168.8.223");
  m_ipInput->setPlaceholderText(tr("Enter IP address"));

  m_connectButton = new QPushButton(tr("Connect"), this);
  connect(m_connectButton, &QPushButton::clicked, this,
          &MainWindow::onConnectClicked);

  m_cancelConnectButton = new QPushButton(tr("Cancel"), this);
  m_cancelConnectButton->setEnabled(false);
  connect(m_cancelConnectButton, &QPushButton::clicked, this,
          &MainWindow::onCancelConnectClicked);

  gridLayout->addWidget(ipLabel, 1, 0);
  gridLayout->addWidget(m_ipInput, 1, 1);
  gridLayout->addWidget(m_connectButton, 1, 2);
  gridLayout->addWidget(m_cancelConnectButton, 1, 3);

  // Status Label
  m_statusLabel = new QLabel(tr("Disconnected"), this);
  m_statusLabel->setObjectName("statusLabel");
  m_statusLabel->setAlignment(Qt::AlignCenter);
  updateStatusStyle("disconnected");

  QWidget *systemMediaTab = new QWidget(this);
  QVBoxLayout *systemMediaLayout = new QVBoxLayout(systemMediaTab);
  systemMediaLayout->addLayout(gridLayout);
  systemMediaLayout->addWidget(m_statusLabel);
  systemMediaLayout->addWidget(m_deviceList);

  QGroupBox *mediaGroup = new QGroupBox(tr("Media gallery"), systemMediaTab);
  QVBoxLayout *mediaLayout = new QVBoxLayout(mediaGroup);
  QHBoxLayout *downloadLayout = new QHBoxLayout();
  QLabel *downloadLabel = new QLabel(tr("Download folder:"), mediaGroup);
  m_downloadDirEdit = new QLineEdit(mediaGroup);
  m_downloadDirEdit->setReadOnly(true);
  m_changeDownloadDirButton = new QPushButton(tr("Change..."), mediaGroup);
  downloadLayout->addWidget(downloadLabel);
  downloadLayout->addWidget(m_downloadDirEdit);
  downloadLayout->addWidget(m_changeDownloadDirButton);
  m_openGalleryButton = new QPushButton(tr("Open gallery"), mediaGroup);
  m_mediaTabs = new QTabWidget(mediaGroup);
  m_mediaPhotoList = new QListWidget(m_mediaTabs);
  m_mediaVideoList = new QListWidget(m_mediaTabs);
  m_mediaBurstList = new QListWidget(m_mediaTabs);
  m_mediaAstroList = new QListWidget(m_mediaTabs);
  m_mediaPanoList = new QListWidget(m_mediaTabs);
  m_mediaTabs->addTab(m_mediaPhotoList, tr("Photo"));
  m_mediaTabs->addTab(m_mediaVideoList, tr("Video"));
  m_mediaTabs->addTab(m_mediaBurstList, tr("Burst"));
  m_mediaTabs->addTab(m_mediaAstroList, tr("Astro"));
  m_mediaTabs->addTab(m_mediaPanoList, tr("Panorama"));
  mediaLayout->addLayout(downloadLayout);
  mediaLayout->addWidget(m_openGalleryButton);
  mediaLayout->addWidget(m_mediaTabs);
  mediaGroup->setLayout(mediaLayout);
  systemMediaLayout->addWidget(mediaGroup);

  if (m_openGalleryButton)
    connect(m_openGalleryButton, &QPushButton::clicked, this,
            &MainWindow::onOpenGalleryClicked);

  // Update lightbox content when switching tabs
  connect(m_mediaTabs, &QTabWidget::currentChanged, this, [this](int index) {
    if (!m_currentLightbox)
      return;

    // Get the new list widget for this tab
    QListWidget *newList = nullptr;
    switch (index) {
    case 0:
      newList = m_mediaPhotoList;
      break;
    case 1:
      newList = m_mediaVideoList;
      break;
    case 2:
      newList = m_mediaBurstList;
      break;
    case 3:
      newList = m_mediaAstroList;
      break;
    case 4:
      newList = m_mediaPanoList;
      break;
    }

    if (!newList)
      return;

    const int count = newList->count();
    if (count <= 0) {
      m_currentLightbox->close();
      m_currentLightbox = nullptr;
      return;
    }

    QVector<QJsonObject> mediaList(count);
    QVector<QPixmap> thumbnails(count);
    for (int i = 0; i < count; ++i) {
      QListWidgetItem *item = newList->item(i);
      if (!item)
        continue;
      QVariant data = item->data(Qt::UserRole);
      if (data.isValid())
        mediaList[i] = data.toJsonObject();
      QIcon icon = item->icon();
      if (!icon.isNull())
        thumbnails[i] = icon.pixmap(500, 350);
    }

    int currentIndex = newList->currentRow();
    if (currentIndex < 0 || currentIndex >= count)
      currentIndex = 0;

    m_currentLightbox->setMediaList(mediaList, currentIndex, thumbnails);
  });

  systemMediaTab->setLayout(systemMediaLayout);

  // Tab widget for modules
  m_tabWidget = new QTabWidget(this);

  // Camera Settings Panel (Tab 1)
  m_cameraSettingsPanel = new CameraSettingsPanel(this);
  m_cameraSettingsPanel->setCameraController(m_cameraController);

  // Sync UI when camera parameters are fetched from device
  connect(m_cameraController, &DwarfCameraController::allParamsReceived, this,
          [this](DwarfCameraController::CameraKind kind) {
            qWarning() << "[MainWindow] allParamsReceived for"
                       << (kind == DwarfCameraController::CameraKind::Tele ? "Tele" : "Wide");
            if (m_cameraSettingsPanel) {
              m_cameraSettingsPanel->syncFromController();
            }
          });

  // Connect camera mode changes to stream switching
  connect(m_cameraSettingsPanel, &CameraSettingsPanel::cameraModeChanged, this,
          [this](CameraSettingsPanel::CameraMode mode) {
            if (mode == CameraSettingsPanel::CameraMode::Tele) {
              onCameraSourceTele();
            } else {
              onCameraSourceWide();
            }
          });

  // Photo feedback
  connect(m_cameraSettingsPanel, &CameraSettingsPanel::photoRequested, this,
          [this]() {
            statusBar()->showMessage(tr("Photo captured"), 3000);
          });

  // Video recording feedback with elapsed time
  connect(m_cameraSettingsPanel, &CameraSettingsPanel::recordRequested, this,
          [this](bool recording) {
            if (!m_recordTimer)
              return;

            if (recording) {
              m_recordElapsed.restart();
              m_recordTimer->start();
              // Initial message
              statusBar()->showMessage(tr("Recording video... 00:00"), 0);

              connect(m_recordTimer, &QTimer::timeout, this,
                      [this]() {
                        qint64 secs = m_recordElapsed.elapsed() / 1000;
                        int minutes = static_cast<int>(secs / 60);
                        int seconds = static_cast<int>(secs % 60);
                        QString timeStr =
                            QStringLiteral("%1:%2")
                                .arg(minutes, 2, 10, QLatin1Char('0'))
                                .arg(seconds, 2, 10, QLatin1Char('0'));
                        statusBar()->showMessage(
                            tr("Recording video... %1").arg(timeStr), 0);
                      });
            } else {
              m_recordTimer->stop();
              statusBar()->showMessage(tr("Video recording stopped"), 3000);
              // Disconnect timeout connections to avoid duplicates
              m_recordTimer->disconnect(this);
            }
          });

  updateCameraStreamViews();

  QWidget *astroTab = new QWidget(this);
  QVBoxLayout *astroLayout = new QVBoxLayout(astroTab);
  QLabel *astroLabel = new QLabel(tr("Astro & Navigation (TODO)"), astroTab);
  astroLabel->setAlignment(Qt::AlignCenter);
  astroLayout->addWidget(astroLabel);
  astroTab->setLayout(astroLayout);

  QWidget *motorFocusTab = new QWidget(this);
  QVBoxLayout *motorFocusLayout = new QVBoxLayout(motorFocusTab);
  QLabel *motorFocusLabel =
      new QLabel(tr("Motor & Focus controls"), motorFocusTab);
  motorFocusLabel->setAlignment(Qt::AlignCenter);
  motorFocusLayout->addWidget(motorFocusLabel);

  // Simple D-pad style motor controls
  QGroupBox *motorGroup = new QGroupBox(tr("Motor"), motorFocusTab);
  QGridLayout *motorGrid = new QGridLayout(motorGroup);

  QPushButton *btnUp = new QPushButton(tr("\u2191"), motorGroup);
  QPushButton *btnDown = new QPushButton(tr("\u2193"), motorGroup);
  QPushButton *btnLeft = new QPushButton(tr("\u2190"), motorGroup);
  QPushButton *btnRight = new QPushButton(tr("\u2192"), motorGroup);

  motorGrid->addWidget(btnUp, 0, 1);
  motorGrid->addWidget(btnLeft, 1, 0);
  motorGrid->addWidget(btnRight, 1, 2);
  motorGrid->addWidget(btnDown, 2, 1);

  connect(btnLeft, &QPushButton::pressed, this,
          &MainWindow::onMotorLeftPressed);
  connect(btnLeft, &QPushButton::released, this,
          &MainWindow::onMotorLeftReleased);
  connect(btnRight, &QPushButton::pressed, this,
          &MainWindow::onMotorRightPressed);
  connect(btnRight, &QPushButton::released, this,
          &MainWindow::onMotorRightReleased);
  connect(btnUp, &QPushButton::pressed, this, &MainWindow::onMotorUpPressed);
  connect(btnUp, &QPushButton::released, this,
          &MainWindow::onMotorUpReleased);
  connect(btnDown, &QPushButton::pressed, this,
          &MainWindow::onMotorDownPressed);
  connect(btnDown, &QPushButton::released, this,
          &MainWindow::onMotorDownReleased);

  motorFocusLayout->addWidget(motorGroup);

  QGroupBox *motorSpeedGroup = new QGroupBox(tr("Motor speed"), motorFocusTab);
  QHBoxLayout *speedLayout = new QHBoxLayout(motorSpeedGroup);
  QLabel *speedLabel = new QLabel(tr("Speed"), motorSpeedGroup);
  m_motorSpeedSlider = new QSlider(Qt::Horizontal, motorSpeedGroup);
  m_motorSpeedSlider->setRange(0, 4);
  m_motorSpeedSlider->setValue(2);
  m_motorSpeedSlider->setSingleStep(1);
  m_motorSpeedSlider->setPageStep(1);
  m_motorSpeedSlider->setTickPosition(QSlider::TicksBelow);
  m_motorSpeedSlider->setTickInterval(1);
  m_motorSpeedValueLabel = new QLabel(motorSpeedGroup);
  speedLayout->addWidget(speedLabel);
  speedLayout->addWidget(m_motorSpeedSlider);
  speedLayout->addWidget(m_motorSpeedValueLabel);
  motorSpeedGroup->setLayout(speedLayout);
  motorFocusLayout->addWidget(motorSpeedGroup);

  QGroupBox *focusGroup = new QGroupBox(tr("Focus"), motorFocusTab);
  QHBoxLayout *focusLayout = new QHBoxLayout(focusGroup);
  QPushButton *focusMinus = new QPushButton(tr("FOCUS -"), focusGroup);
  QPushButton *focusPlus = new QPushButton(tr("FOCUS +"), focusGroup);
  QPushButton *focusAuto = new QPushButton(tr("AUTO"), focusGroup);
  focusLayout->addWidget(focusMinus);
  focusLayout->addWidget(focusPlus);
  focusLayout->addWidget(focusAuto);
  focusGroup->setLayout(focusLayout);
  motorFocusLayout->addWidget(focusGroup);

  connect(focusMinus, &QPushButton::clicked, this,
          &MainWindow::onFocusMinusClicked);
  connect(focusPlus, &QPushButton::clicked, this,
          &MainWindow::onFocusPlusClicked);
  connect(focusAuto, &QPushButton::clicked, this,
          &MainWindow::onFocusAutoClicked);

  if (m_motorSpeedSlider)
    connect(m_motorSpeedSlider, &QSlider::valueChanged, this,
            &MainWindow::onMotorSpeedSliderChanged);

  if (m_motorSpeedSlider)
    onMotorSpeedSliderChanged(m_motorSpeedSlider->value());

  motorFocusLayout->addStretch();
  motorFocusTab->setLayout(motorFocusLayout);

  m_tabWidget->addTab(systemMediaTab, tr("System & Media"));
  m_tabWidget->addTab(motorFocusTab, tr("Motor & Focus"));
  m_tabWidget->addTab(m_cameraSettingsPanel, tr("Camera & Capture"));
  m_tabWidget->addTab(astroTab, tr("Astro & Navigation"));

  m_tabWidget->setTabPosition(QTabWidget::East);

  QDockWidget *controlDock = new QDockWidget(tr("Control Deck"), this);
  controlDock->setAllowedAreas(Qt::RightDockWidgetArea);
  controlDock->setFeatures(static_cast<QDockWidget::DockWidgetFeatures>(
      QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable |
      QDockWidget::DockWidgetFloatable));

  QWidget *dockContents = new QWidget(controlDock);
  QVBoxLayout *dockLayout = new QVBoxLayout(dockContents);
  dockLayout->addWidget(m_tabWidget);
  dockContents->setLayout(dockLayout);
  controlDock->setWidget(dockContents);
  addDockWidget(Qt::RightDockWidgetArea, controlDock);

  // Connect MJPEG streams to views for repaint on new frames
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

  QSettings settings("DwarfLab", "DwarfController");
  QString dir = settings.value("downloadDir").toString();
  if (dir.isEmpty()) {
    QString base = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
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

  if (m_changeDownloadDirButton)
    connect(m_changeDownloadDirButton, &QPushButton::clicked, this,
            &MainWindow::onChangeDownloadDirClicked);

  // Single click: open lightbox preview
  // Double click: start download
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
  onConnectClicked();
}

void MainWindow::onConnectClicked() {
  QString ip = m_ipInput->text().trimmed();

  if (ip.isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("Please enter an IP address"));
    return;
  }

  if (m_wsClient && m_wsClient->isConnected()) {
    m_wsClient->disconnect();
    delete m_wsClient;
    m_wsClient = nullptr;
    m_connectButton->setText(tr("Connect"));
    m_cancelConnectButton->setEnabled(false);
    m_statusLabel->setText(tr("Disconnected"));
    updateStatusStyle("disconnected");
    statusBar()->showMessage(tr("Disconnected"));
    if (m_cameraController) {
      m_cameraController->setClient(nullptr);
    }
    if (m_motorController) {
      m_motorController->setClient(nullptr);
    }
    if (m_focusController) {
      m_focusController->setClient(nullptr);
    }
    stopStreaming();
  } else {
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

    if (m_cameraController) {
      m_cameraController->setClient(m_wsClient);
    }
    if (m_motorController) {
      m_motorController->setClient(m_wsClient);
    }
    if (m_focusController) {
      m_focusController->setClient(m_wsClient);
    }

    m_wsClient->connectToDevice();
    m_connectButton->setEnabled(false); // Disable connect while connecting
    m_cancelConnectButton->setEnabled(true);
    m_statusLabel->setText(tr("Connecting..."));
    updateStatusStyle("connecting");
    statusBar()->showMessage(tr("Connecting to %1").arg(ip));
  }
}

void MainWindow::onCancelConnectClicked() {
  if (m_wsClient) {
    // If we are connecting, this should abort it.
    // DwarfWebSocketClient might not have an abort method exposed easily,
    // but deleting it or calling disconnect should work.
    m_wsClient->disconnect();
    delete m_wsClient;
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
  // If the subnet looks valid (e.g. 3 parts), update the IP input
  QStringList parts = text.split('.');
  if (parts.size() >= 3) {
    QString ip =
        QString("%1.%2.%3.1").arg(parts[0]).arg(parts[1]).arg(parts[2]);
    m_ipInput->setText(ip);
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

  // Start streaming now that we are connected
  QString ip = m_ipInput->text().trimmed();
  startStreaming(ip);
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
  m_connectButton->setEnabled(true);
  m_connectButton->setText(tr("Connect"));
  m_cancelConnectButton->setEnabled(false);
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
  m_motorController->runMotor(DwarfMotorController::Axis::Azimuth, true,
                              speed);
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

  QJsonArray files;
  if (document.isArray())
    files = document.array();
  else
    files = document.object().value(QStringLiteral("data")).toArray();

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
      display = QString::fromUtf8(
          QJsonDocument(obj).toJson(QJsonDocument::Compact));

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
        [this, capturedList, capturedRow, capturedPath](const QByteArray &data) {
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
    startDir = QStandardPaths::writableLocation(
        QStandardPaths::DownloadLocation);
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

  connect(lightbox, &MediaLightbox::deleteRequested, this,
          [this, lightbox](const QJsonObject &mediaInfo) {
            QString filePath = mediaInfo.value(QStringLiteral("filePath")).toString();
            QString fileName = mediaInfo.value(QStringLiteral("fileName")).toString();
            if (fileName.isEmpty()) {
              QFileInfo fi(filePath);
              fileName = fi.fileName();
            }

            // Confirm deletion
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Delete Media"),
                tr("Are you sure you want to delete '%1' from the DWARF II?").arg(fileName),
                QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes && m_ftpDownloader) {
              QString ip = m_ipInput->text().trimmed();
              statusBar()->showMessage(tr("Deleting %1...").arg(fileName), 0);

              // Use FTP for deletion
              m_ftpDownloader->deleteFile(ip, filePath,
                  [this, fileName, lightbox](bool success, const QString &error) {
                    if (success) {
                      statusBar()->showMessage(tr("Deleted %1").arg(fileName), 3000);
                      if (lightbox) {
                        lightbox->close();
                      }
                      // Refresh media list
                      if (m_httpClient) {
                        m_httpClient->fetchMediaList();
                      }
                    } else {
                      statusBar()->showMessage(tr("Delete failed: %1").arg(error), 5000);
                    }
                  });
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
  qWarning() << "[MainWindow] PiP double-click: updateCameraStreamViews finished";
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
        if (m_pipStreamView)
          m_pipStreamView->raise();
        
        // Fetch current camera parameters to sync UI
        if (m_cameraController) {
          m_cameraController->fetchAllParams(DwarfCameraController::CameraKind::Tele);
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
        if (m_pipStreamView)
          m_pipStreamView->raise();
        
        // Fetch current camera parameters to sync UI
        if (m_cameraController) {
          m_cameraController->fetchAllParams(DwarfCameraController::CameraKind::Wide);
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
