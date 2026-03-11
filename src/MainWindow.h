#pragma once

#include "net/DwarfFinder.h"
#include "net/DwarfMessageDispatcher.h"
#include "net/DwarfWebSocketClient.h"
 #include "net/DwarfCameraController.h"
#include <QCheckBox>
#include <QComboBox>
#include <QElapsedTimer>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QPointer>
#include <QPushButton>
#include <QSlider>
#include <QMessageBox>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QTabBar>
#include <QVideoWidget>
#include <QWidget>

class DwarfCameraController;
class DwarfMotorController;
class DwarfFocusController;
class DwarfSystemController;
class DwarfAstroController;
class DwarfPanoramaController;
class DwarfMjpegStream;
class MediaLightbox;
class DwarfMjpegView;
class QPointF;
class DwarfHttpClient;
class HelpWindow;
class DwarfFtpDownloader;
class DwarfMtpClient;
class CameraSettingsPanel;
class AstroNavigationPanel;
class VirtualJoystick;
class MotorControlPanel;
class StarMapWidget;
class ParametersOverlayPanel;
class QSpinBox;

class DraggablePiP : public QWidget {
  Q_OBJECT
public:
  explicit DraggablePiP(QWidget *parent = nullptr) : QWidget(parent) {
    setCursor(Qt::OpenHandCursor);
  }

signals:
  void doubleClicked();

protected:
  void mousePressEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      m_dragOffset = event->pos();
      setCursor(Qt::ClosedHandCursor);
    }
  }

  void mouseMoveEvent(QMouseEvent *event) override {
    if (event->buttons() & Qt::LeftButton) {
      QPoint newPos = mapToParent(event->pos() - m_dragOffset);
      if (parentWidget()) {
        newPos.setX(qBound(0, newPos.x(), parentWidget()->width() - width()));
        newPos.setY(qBound(0, newPos.y(), parentWidget()->height() - height()));
      }
      move(newPos);
    }
  }

  void mouseReleaseEvent(QMouseEvent *event) override {
    setCursor(Qt::OpenHandCursor);
  }

  void mouseDoubleClickEvent(QMouseEvent *event) override {
    if (event->button() == Qt::LeftButton) {
      emit doubleClicked();
    }
  }

private:
  QPoint m_dragOffset;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onConnectClicked();
  void onCancelConnectClicked();
  void onScanClicked();
  void onCancelScanClicked();
  void onSubnetTextChanged(const QString &text);
  void onWebSocketConnected();
  void onWebSocketDisconnected();
  void onWebSocketError(const QString &error);
  void onDeviceFound(const DwarfDeviceInfo &info);
  void onScanFinished();
  void onScanProgress(int percent);
  void onDeviceSelected(QListWidgetItem *item);
  void onDisconnectClicked();
  void onCameraTeleMessage(uint32_t cmd, const QByteArray &data);
  void onCameraWideMessage(uint32_t cmd, const QByteArray &data);
  void onPipStreamClicked();
  void onCameraSourceTele();
  void onCameraSourceWide();
  void onMotorLeftPressed();
  void onMotorLeftReleased();
  void onMotorRightPressed();
  void onMotorRightReleased();
  void onMotorUpPressed();
  void onMotorUpReleased();
  void onMotorDownPressed();
  void onMotorDownReleased();
  void onFocusMinusClicked();
  void onFocusPlusClicked();
  void onFocusAutoClicked();
  void onMotorSpeedSliderChanged(int value);
  void onMainViewPointClicked(const QPointF &normalizedPos);
  void onOpenGalleryClicked();
  void onMediaListReceived(const QJsonDocument &document);
  void onMediaListError(const QString &error);
  void onDefaultParamsConfigReceived(const QJsonDocument &document);
  void onChangeDownloadDirClicked();
  void onMediaItemClicked(QListWidgetItem *item);
  void onMediaItemActivated(QListWidgetItem *item);
  void onDownloadStarted(const QString &fileName);
  void onDownloadFinished(const QString &fileName, const QString &localPath);
  void onDownloadError(const QString &fileName, const QString &error);

  void onPhotoCaptureFinished(DwarfCameraController::CameraKind kind,
                              bool success, int code,
                              const QString &fileName);
  void onRecordFinished(DwarfCameraController::CameraKind kind, bool recording,
                        bool success, int code);

  void onStarMapOverlayRequested(bool enabled);
  void onGalleryOverlayRequested(bool enabled);
  void onHelpClicked();

private:
  void setupUi();
  void updateOverlayVisibility();
  void updateSidebarForConnectionState(bool connected);
  void loadThumbnails();
  void setItemThumbnail(QListWidgetItem *item, const QByteArray &data);
  void syncTimeWithDevice();
  void ensureHttpClientForCurrentIp();
  void setCaptureStatusTextAllPanels(const QString &text);
  void persistParamsConfig(const QJsonDocument &document);
  void applyCachedParamsConfig();
  void loadSettings();
  void saveSettings();

  QLineEdit *m_ipInput;
  QLineEdit *m_subnetInput;
  QPushButton *m_connectButton;
  QPushButton *m_cancelConnectButton;
  QPushButton *m_scanButton;
  QPushButton *m_cancelScanButton;
  QListWidget *m_deviceList;
  QLabel *m_statusLabel;
  QStackedWidget *m_contentStack;
  QWidget *m_sidebar;
  QButtonGroup *m_sidebarGroup;
  QCheckBox *m_clientModeCheck;
  DwarfWebSocketClient *m_wsClient;
  DwarfMessageDispatcher *m_dispatcher;
  
  // Overlays
  MotorControlPanel *m_motorOverlay = nullptr;
  ParametersOverlayPanel *m_paramsOverlay = nullptr;
  bool m_motorOverlayUserVisible = true;
  bool m_paramsOverlayUserVisible = true;
  QToolButton *m_motorOverlayToggleButton = nullptr;
  QToolButton *m_paramsOverlayToggleButton = nullptr;
  QWidget *m_starMapOverlayContainer = nullptr;
  StarMapWidget *m_starMapOverlayWidget = nullptr;
  bool m_starMapOverlayEnabled = false;

  QWidget *m_galleryOverlayContainer = nullptr;
  QWidget *m_galleryTab = nullptr;
  bool m_galleryOverlayEnabled = false;

  QWidget *m_astroTabsOverlayContainer = nullptr;
  QTabBar *m_astroTabsOverlayBar = nullptr;

private:
  void updateOverlayPositions();
  void updatePanoramaGridOverlayTarget();
  
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  DwarfFinder *m_finder;
  bool m_scanCancelled;
  void updateStatusStyle(const char *statusKey);
  DwarfCameraController *m_cameraController;
  DwarfMotorController *m_motorController;
  DwarfFocusController *m_focusController;
  DwarfSystemController *m_systemController;
  DwarfAstroController *m_astroController;
  DwarfPanoramaController *m_panoramaController;

  enum class CameraStream { Tele, Wide };

  QWidget *m_mainStreamView;
  QLabel *m_streamNameOverlay;
  DraggablePiP *m_pipContainer;
  DwarfMjpegView *m_mainVideoWidget;
  DwarfMjpegView *m_pipVideoWidget;
  CameraStream m_mainStream;
  CameraStream m_pipStream;
  void updateCameraStreamViews();
  void updateStreamRouting();
  void startStreaming(const QString &ip);
  void stopStreaming();

  CameraSettingsPanel *m_cameraSettingsPanel;
  AstroNavigationPanel *m_astroPanel;
  QSlider *m_motorSpeedSlider;
  QLabel *m_motorSpeedValueLabel;

  DwarfMjpegStream *m_teleStream;
  DwarfMjpegStream *m_wideStream;

  QTimer *m_recordTimer;
  QElapsedTimer m_recordElapsed;

  QWidget *m_panoTab = nullptr;
  QSpinBox *m_panoRowsSpin = nullptr;
  QSpinBox *m_panoColsSpin = nullptr;
  QWidget *m_panoGridOverlay = nullptr;
  QLabel *m_panoStatusLabel = nullptr;
  QPushButton *m_panoStartButton = nullptr;
  QPushButton *m_panoStopButton = nullptr;

  bool m_panoRunActive = false;
  int m_panoRunRows = 0;
  int m_panoRunCols = 0;

  bool m_panoStartPending = false;
  int m_panoStartPendingRows = 0;
  int m_panoStartPendingCols = 0;

  DwarfHttpClient *m_httpClient;
  QString m_httpClientIp;
  QPushButton *m_openGalleryButton;
  QTabWidget *m_mediaTabs;
  QListWidget *m_mediaPhotoList;
  QListWidget *m_mediaVideoList;
  QListWidget *m_mediaBurstList;
  QListWidget *m_mediaAstroList;
  QListWidget *m_mediaPanoList;
  QLineEdit *m_downloadDirEdit;
  QPushButton *m_changeDownloadDirButton;
  QLabel *m_batteryLabel = nullptr;
  QLabel *m_firmwareLabel = nullptr;
  QLabel *m_sdSpaceLabel = nullptr;
  QGroupBox *m_systemControlGroup = nullptr;
  QLineEdit *m_timezoneEdit = nullptr;
  QPushButton *m_syncTimeButton = nullptr;
  QPushButton *m_setTimezoneButton = nullptr;
  QCheckBox *m_mtpModeCheck = nullptr;
  QComboBox *m_cpuModeCombo = nullptr;
  QCheckBox *m_rgbLightCheck = nullptr;
  QCheckBox *m_powerIndicatorCheck = nullptr;
  QPushButton *m_powerDownButton = nullptr;
  QPushButton *m_rebootButton = nullptr;
  QLabel *m_systemControlStatusLabel = nullptr;
  QString m_downloadDir;
  DwarfFtpDownloader *m_ftpDownloader;
  DwarfMtpClient *m_mtpClient;
  HelpWindow *m_helpWindow = nullptr;
  struct PendingThumbnail {
    QPointer<QListWidget> list;
    int row;
    QString path;
  };
  QList<PendingThumbnail> m_pendingThumbnails;
  int m_thumbnailsLoading;
  QPointer<MediaLightbox> m_currentLightbox;
  static constexpr int THUMBNAIL_SIZE = 120;

  struct PendingCaptureLookup {
    bool active = false;
    int expectedMediaType = -1; // 1=photo, 2=video
    DwarfCameraController::CameraKind expectedKind =
        DwarfCameraController::CameraKind::Tele;
    QString prefix;
    QString thumbnailPath;
    int attempts = 0;
  };

  PendingCaptureLookup m_pendingCaptureLookup;
};
