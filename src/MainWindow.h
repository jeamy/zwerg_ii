#pragma once

#include "net/DwarfFinder.h"
#include "net/DwarfMessageDispatcher.h"
#include "net/DwarfWebSocketClient.h"
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
#include <QStackedWidget>
#include <QTabWidget>
#include <QTimer>
#include <QToolButton>
#include <QVideoWidget>
#include <QWidget>

class DwarfCameraController;
class DwarfMotorController;
class DwarfFocusController;
class DwarfAstroController;
class DwarfMjpegStream;
class MediaLightbox;
class DwarfMjpegView;
class QPointF;
class DwarfHttpClient;
class DwarfFtpDownloader;
class CameraSettingsPanel;
class AstroNavigationPanel;
class VirtualJoystick;
class MotorControlPanel;

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
  void onChangeDownloadDirClicked();
  void onMediaItemClicked(QListWidgetItem *item);
  void onMediaItemActivated(QListWidgetItem *item);
  void onDownloadStarted(const QString &fileName);
  void onDownloadFinished(const QString &fileName, const QString &localPath);
  void onDownloadError(const QString &fileName, const QString &error);

private:
  void setupUi();
  void loadThumbnails();
  void setItemThumbnail(QListWidgetItem *item, const QByteArray &data);
  void syncTimeWithDevice();

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
  DwarfWebSocketClient *m_wsClient;
  DwarfMessageDispatcher *m_dispatcher;
  
  // Overlays
  MotorControlPanel *m_motorOverlay = nullptr;

private:
  void updateOverlayPositions();
  
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
  DwarfAstroController *m_astroController;

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

  DwarfHttpClient *m_httpClient;
  QPushButton *m_openGalleryButton;
  QTabWidget *m_mediaTabs;
  QListWidget *m_mediaPhotoList;
  QListWidget *m_mediaVideoList;
  QListWidget *m_mediaBurstList;
  QListWidget *m_mediaAstroList;
  QListWidget *m_mediaPanoList;
  QLineEdit *m_downloadDirEdit;
  QPushButton *m_changeDownloadDirButton;
  QString m_downloadDir;
  DwarfFtpDownloader *m_ftpDownloader;
  struct PendingThumbnail {
    QPointer<QListWidget> list;
    int row;
    QString path;
  };
  QList<PendingThumbnail> m_pendingThumbnails;
  int m_thumbnailsLoading;
  QPointer<MediaLightbox> m_currentLightbox;
  static constexpr int THUMBNAIL_SIZE = 120;
};
