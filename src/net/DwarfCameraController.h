#pragma once

#include "DwarfWebSocketClient.h"
#include "camera.pb.h"
#include <QObject>
#include <QSettings>
 #include <QString>

#include <memory>

class DwarfCameraController : public QObject {
  Q_OBJECT

public:
  enum class CameraKind { Tele, Wide };

  explicit DwarfCameraController(QObject *parent = nullptr);

  void setClient(DwarfWebSocketClient *client);

  // High level camera actions
  void openCamera(CameraKind kind, bool binning, int rtspEncodeType);
  void closeCamera(CameraKind kind);
  void takePhoto(CameraKind kind);
  void startRecord(CameraKind kind);
  void stopRecord(CameraKind kind);

  // Parameter setters (will internally send ReqSetAllParams)
  void setExposureMode(CameraKind kind, int mode);
  void setExposureIndex(CameraKind kind, int index);
  void setGainMode(CameraKind kind, int mode);
  void setGainIndex(CameraKind kind, int index);
  void setIrCut(CameraKind kind, int value);
  void setWhiteBalanceMode(CameraKind kind, int mode);
  void setWhiteBalanceByTemperature(CameraKind kind, int ctIndex);
  void setBrightness(CameraKind kind, int value);
  void setContrast(CameraKind kind, int value);
  void setHue(CameraKind kind, int value);
  void setSaturation(CameraKind kind, int value);
  void setSharpness(CameraKind kind, int value);

  // Current parameter getters (used by UI to sync state)
  int exposureMode(CameraKind kind) const;
  int exposureIndex(CameraKind kind) const;
  int gainMode(CameraKind kind) const;
  int gainIndex(CameraKind kind) const;
  int whiteBalanceMode(CameraKind kind) const;
  int whiteBalanceTemperatureIndex(CameraKind kind) const;

  // Additional getters for UI + persistence (UI scale where applicable)
  bool irCutEnabled(CameraKind kind) const;
  int brightness(CameraKind kind) const;   // 0-100
  int contrast(CameraKind kind) const;     // 0-100
  int saturation(CameraKind kind) const;   // 0-100
  int hue(CameraKind kind) const;          // 0-100
  int sharpness(CameraKind kind) const;    // 0-100

  // Fetch all parameters from device (async, emits allParamsReceived when done)
  void fetchAllParams(CameraKind kind);

  // Handle incoming camera messages from WebSocket
  void handleCameraMessage(quint32 moduleId, quint32 cmd, const QByteArray &data);

signals:
  void errorOccurred(const QString &message);
  void allParamsReceived(CameraKind kind);  // Emitted when GET_ALL_PARAMS response arrives
  void photoTaken(CameraKind kind);  // Emitted when photo is successfully taken

  void photoCaptureFinished(CameraKind kind, bool success, int code,
                            const QString &fileName);
  void recordFinished(CameraKind kind, bool recording, bool success, int code);

private:
  DwarfWebSocketClient *m_client;

  dwarf::ReqSetAllParams m_teleParams;
  dwarf::ReqSetAllParams m_wideParams;

  bool m_teleCameraOpenRequested = false;
  bool m_wideCameraOpenRequested = false;

  bool m_teleLastBinning = true;
  int m_teleLastRtspEncodeType = 0;
  bool m_wideLastBinning = false;
  int m_wideLastRtspEncodeType = 0;

  bool m_teleRecordModeOpenAttempted = false;
  bool m_module15ImageParamsInitialized = false;

  std::unique_ptr<QSettings> m_config;

  void ensureConfigLoaded();
  void loadConfig();
  void saveConfig(CameraKind kind);

  void sendSetAllParams(CameraKind kind);
  void sendSingleInt32(CameraKind kind, quint32 cmd, int value);
  void ensureModule15ImageParamsInitialized();
  void sendModule15Cmd16703(quint64 selector);
  void sendModule15Cmd16703(quint64 selector, int value);
  quint32 moduleIdFor(CameraKind kind) const;
  quint32 cmdSetAllParamsFor(CameraKind kind) const;
  quint32 cmdOpenCameraFor(CameraKind kind) const;
  quint32 cmdCloseCameraFor(CameraKind kind) const;
  quint32 cmdPhotoFor(CameraKind kind) const;
  quint32 cmdStartRecordFor(CameraKind kind) const;
  quint32 cmdStopRecordFor(CameraKind kind) const;
  // Individual parameter commands
  quint32 cmdSetExpModeFor(CameraKind kind) const;
  quint32 cmdSetExpFor(CameraKind kind) const;
  quint32 cmdSetGainModeFor(CameraKind kind) const;
  quint32 cmdSetGainFor(CameraKind kind) const;
  quint32 cmdSetBrightnessFor(CameraKind kind) const;
  quint32 cmdSetContrastFor(CameraKind kind) const;
  quint32 cmdSetSaturationFor(CameraKind kind) const;
  quint32 cmdSetHueFor(CameraKind kind) const;
  quint32 cmdSetSharpnessFor(CameraKind kind) const;
  quint32 cmdSetIrCutFor(CameraKind kind) const;
};
