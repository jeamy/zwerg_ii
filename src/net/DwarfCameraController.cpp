#include "DwarfCameraController.h"

#include "ProtobufHelper.h"

#include <QTimer>
#include <QDir>
#include <QSettings>
#include <QStandardPaths>

using dwarf::ReqOpenCamera;
using dwarf::ReqPhoto;
using dwarf::ReqSetAllParams;
using dwarf::ReqStartRecord;
using dwarf::ReqStopRecord;
using dwarf::ResGetAllParams;
using dwarf::ReqSetExpMode;
using dwarf::ReqSetGain;
using dwarf::ReqSetGainMode;
using dwarf::ReqSetHue;
using dwarf::ReqSetIRCut;
using dwarf::ReqSetBrightness;
using dwarf::ReqSetContrast;
using dwarf::ReqSetExp;
using dwarf::ReqSetSaturation;
using dwarf::ReqSetSharpness;
using dwarf::ReqSetWBMode;

namespace {
inline int clampInt(int value, int minV, int maxV) {
  return std::max(minV, std::min(maxV, value));
}

// Scale UI value (0-100) to API value (0-255) for
// brightness/contrast/saturation Formula: B = (A + 100) * 255.0 / 200, where A
// is -100 to 100 For our 0-100 slider: map 0->0, 50->127.5, 100->255
inline int scaleToApi255(int value0to100) {
  int v = clampInt(value0to100, 0, 100);
  // Map 0-100 to -100 to 100, then apply formula
  int a = (v * 2) - 100; // 0->-100, 50->0, 100->100
  return static_cast<int>((a + 100) * 255.0 / 200.0);
}

// Scale UI value (0-100) to API value for hue
// Formula: B = (A + 180) * 255.0 / 360, where A is -180 to 180
inline int scaleHueToApi(int value0to100) {
  int v = clampInt(value0to100, 0, 100);
  // Map 0-100 to -180 to 180
  int a = static_cast<int>((v * 3.6) - 180);
  return static_cast<int>((a + 180) * 255.0 / 360.0);
}

inline int scaleFromApi255(int value0to255) {
  int b = clampInt(value0to255, 0, 255);
  // Reverse of: B = (A + 100) * 255 / 200, with A in [-100, 100]
  // A = B * 200 / 255 - 100
  double a = (static_cast<double>(b) * 200.0 / 255.0) - 100.0;
  // Our UI uses 0-100 mapped to A=-100..100 via A=(v*2-100)
  double v = (a + 100.0) / 2.0;
  return clampInt(static_cast<int>(std::lround(v)), 0, 100);
}

inline int scaleHueFromApi(int value0to255) {
  int b = clampInt(value0to255, 0, 255);
  // Reverse of: B = (A + 180) * 255 / 360, with A in [-180, 180]
  // A = B * 360 / 255 - 180
  double a = (static_cast<double>(b) * 360.0 / 255.0) - 180.0;
  // UI maps 0-100 to A=-180..180 via A=(v*3.6-180)
  double v = (a + 180.0) / 3.6;
  return clampInt(static_cast<int>(std::lround(v)), 0, 100);
}

inline QString kindKey(DwarfCameraController::CameraKind kind) {
  return (kind == DwarfCameraController::CameraKind::Tele) ? QStringLiteral("tele")
                                                          : QStringLiteral("wide");
}
} // namespace

DwarfCameraController::DwarfCameraController(QObject *parent)
    : QObject(parent), m_client(nullptr), m_teleCameraOpenRequested(false), m_wideCameraOpenRequested(false) {
  qWarning() << "[DwarfCameraController] Created with" << (parent ? "valid" : "null") << "parent";
  // Initialize with sensible defaults
  // Tele camera defaults
  m_teleParams.set_exp_mode(0);      // Auto exposure
  m_teleParams.set_exp_index(10);    // Mid-range exposure
  m_teleParams.set_gain_mode(0);     // Auto gain
  m_teleParams.set_gain_index(15);   // Mid-range gain
  m_teleParams.set_ircut_value(0);   // IR-Cut ON (day mode)
  m_teleParams.set_wb_mode(0);       // Auto white balance
  m_teleParams.set_wb_index_type(0); // Color temperature
  m_teleParams.set_wb_index(5);      // ~5500K
  m_teleParams.set_brightness(128);  // 50%
  m_teleParams.set_contrast(128);    // 50%
  m_teleParams.set_hue(128);         // 50%
  m_teleParams.set_saturation(128);  // 50%
  m_teleParams.set_sharpness(50);    // 50%
  m_teleParams.set_jpg_quality(90);  // High quality

  // Wide camera defaults (same as tele)
  m_wideParams.set_exp_mode(0);
  m_wideParams.set_exp_index(8); // Mid-range for wide
  m_wideParams.set_gain_mode(0);
  m_wideParams.set_gain_index(5); // Mid-range for wide (0-100)
  m_wideParams.set_ircut_value(0);
  m_wideParams.set_wb_mode(0);
  m_wideParams.set_wb_index_type(0);
  m_wideParams.set_wb_index(5);
  m_wideParams.set_brightness(128);
  m_wideParams.set_contrast(128);
  m_wideParams.set_hue(128);
  m_wideParams.set_saturation(128);
  m_wideParams.set_sharpness(50);
  m_wideParams.set_jpg_quality(90);

  ensureConfigLoaded();
}

void DwarfCameraController::ensureConfigLoaded() {
  if (m_config)
    return;

  const QString baseDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  QDir().mkpath(baseDir);
  const QString path = QDir(baseDir).filePath(QStringLiteral("DwarfController.ini"));

  m_config = std::make_unique<QSettings>(path, QSettings::IniFormat);
  loadConfig();
}

void DwarfCameraController::loadConfig() {
  if (!m_config)
    return;

  auto loadKind = [this](CameraKind kind) {
    ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
    const QString group = QStringLiteral("camera/") + kindKey(kind);
    m_config->beginGroup(group);

    p.set_exp_mode(m_config->value(QStringLiteral("exp_mode"), p.exp_mode()).toInt());
    p.set_exp_index(m_config->value(QStringLiteral("exp_index"), p.exp_index()).toInt());
    p.set_gain_mode(m_config->value(QStringLiteral("gain_mode"), p.gain_mode()).toInt());
    p.set_gain_index(m_config->value(QStringLiteral("gain_index"), p.gain_index()).toInt());
    p.set_ircut_value(m_config->value(QStringLiteral("ircut_value"), p.ircut_value()).toInt());
    p.set_wb_mode(m_config->value(QStringLiteral("wb_mode"), p.wb_mode()).toInt());
    p.set_wb_index_type(m_config->value(QStringLiteral("wb_index_type"), p.wb_index_type()).toInt());
    p.set_wb_index(m_config->value(QStringLiteral("wb_index"), p.wb_index()).toInt());
    p.set_brightness(m_config->value(QStringLiteral("brightness"), p.brightness()).toInt());
    p.set_contrast(m_config->value(QStringLiteral("contrast"), p.contrast()).toInt());
    p.set_saturation(m_config->value(QStringLiteral("saturation"), p.saturation()).toInt());
    p.set_hue(m_config->value(QStringLiteral("hue"), p.hue()).toInt());
    p.set_sharpness(m_config->value(QStringLiteral("sharpness"), p.sharpness()).toInt());
    p.set_jpg_quality(m_config->value(QStringLiteral("jpg_quality"), p.jpg_quality()).toInt());

    m_config->endGroup();
  };

  loadKind(CameraKind::Tele);
  loadKind(CameraKind::Wide);
}

void DwarfCameraController::saveConfig(CameraKind kind) {
  ensureConfigLoaded();
  if (!m_config)
    return;

  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;

  const QString group = QStringLiteral("camera/") + kindKey(kind);
  m_config->beginGroup(group);
  m_config->setValue(QStringLiteral("exp_mode"), p.exp_mode());
  m_config->setValue(QStringLiteral("exp_index"), p.exp_index());
  m_config->setValue(QStringLiteral("gain_mode"), p.gain_mode());
  m_config->setValue(QStringLiteral("gain_index"), p.gain_index());
  m_config->setValue(QStringLiteral("ircut_value"), p.ircut_value());
  m_config->setValue(QStringLiteral("wb_mode"), p.wb_mode());
  m_config->setValue(QStringLiteral("wb_index_type"), p.wb_index_type());
  m_config->setValue(QStringLiteral("wb_index"), p.wb_index());
  m_config->setValue(QStringLiteral("brightness"), p.brightness());
  m_config->setValue(QStringLiteral("contrast"), p.contrast());
  m_config->setValue(QStringLiteral("saturation"), p.saturation());
  m_config->setValue(QStringLiteral("hue"), p.hue());
  m_config->setValue(QStringLiteral("sharpness"), p.sharpness());
  m_config->setValue(QStringLiteral("jpg_quality"), p.jpg_quality());
  m_config->endGroup();
  m_config->sync();
}

void DwarfCameraController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  if (!m_client) {
    m_teleCameraOpenRequested = false;
    m_wideCameraOpenRequested = false;
    m_teleRecordModeOpenAttempted = false;
  }
  qWarning() << "[DwarfCameraController] setClient called with"
             << (client ? "valid" : "null") << "client";
}

void DwarfCameraController::openCamera(CameraKind kind, bool binning,
                                       int rtspEncodeType) {
  qWarning() << "[DwarfCameraController] openCamera kind"
             << static_cast<int>(kind) << "binning" << binning
             << "rtspEncodeType" << rtspEncodeType;
  if (!m_client || !m_client->isConnected()) {
    qWarning()
        << "[DwarfCameraController] Cannot open camera, client not connected";
    emit errorOccurred("Camera client not connected");
    return;
  }

  ReqOpenCamera req;
  req.set_binning(binning);
  req.set_rtsp_encode_type(rtspEncodeType);

  if (kind == CameraKind::Tele) {
    m_teleCameraOpenRequested = true;
    m_teleLastBinning = binning;
    m_teleLastRtspEncodeType = rtspEncodeType;
  } else {
    m_wideCameraOpenRequested = true;
    m_wideLastBinning = binning;
    m_wideLastRtspEncodeType = rtspEncodeType;
  }

  qDebug() << "Sending OpenCamera for kind" << (int)kind << "binning"
           << binning;

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleIdFor(kind), cmdOpenCameraFor(kind), data);

  // Fetch initial parameters to sync local state
  fetchAllParams(kind);
}

void DwarfCameraController::closeCamera(CameraKind kind) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

  if (kind == CameraKind::Tele)
    m_teleCameraOpenRequested = false;
  else
    m_wideCameraOpenRequested = false;

  if (kind == CameraKind::Tele)
    m_teleRecordModeOpenAttempted = false;

  m_client->sendCommand(moduleIdFor(kind), cmdCloseCameraFor(kind),
                        QByteArray());
}

void DwarfCameraController::takePhoto(CameraKind kind) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

  ReqPhoto req;
  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleIdFor(kind), cmdPhotoFor(kind), data);
}

void DwarfCameraController::startRecord(CameraKind kind) {
  if (kind != CameraKind::Tele) {
    qWarning() << "Video recording is only supported for TELE camera";
    emit errorOccurred("Video recording is only supported for TELE camera");
    return;
  }
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

  // Firmware hint: recording may require TELE camera opened with rtsp_encode_type=1 (1080p).
  // Try switching once per connection/session to avoid flicker loops.
  if (!m_teleRecordModeOpenAttempted && m_teleLastRtspEncodeType != 1) {
    m_teleRecordModeOpenAttempted = true;
    openCamera(CameraKind::Tele, m_teleLastBinning, 1);
    QTimer::singleShot(600, this, [this]() {
      if (!m_client || !m_client->isConnected())
        return;
      ReqStartRecord req;
      const QByteArray data = ProtobufHelper::serialize(req);
      m_client->sendCommand(moduleIdFor(CameraKind::Tele),
                            cmdStartRecordFor(CameraKind::Tele), data);
    });
    return;
  }

  // Some firmware variants require the TELE camera to have been opened
  // before startRecord works. Do this once if needed (no repeated reopen).
  if (!m_teleCameraOpenRequested) {
    openCamera(CameraKind::Tele, m_teleLastBinning, m_teleLastRtspEncodeType);
    QTimer::singleShot(600, this, [this]() {
      if (!m_client || !m_client->isConnected())
        return;
      ReqStartRecord req;
      const QByteArray data = ProtobufHelper::serialize(req);
      m_client->sendCommand(moduleIdFor(CameraKind::Tele),
                            cmdStartRecordFor(CameraKind::Tele), data);
    });
    return;
  }

  ReqStartRecord req;
  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleIdFor(kind), cmdStartRecordFor(kind), data);
}

void DwarfCameraController::stopRecord(CameraKind kind) {
  if (kind != CameraKind::Tele) {
    qWarning() << "Video recording is only supported for TELE camera";
    emit errorOccurred("Video recording is only supported for TELE camera");
    return;
  }
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

  ReqStopRecord req;
  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleIdFor(kind), cmdStopRecordFor(kind), data);
}

void DwarfCameraController::setExposureMode(CameraKind kind, int mode) {
  qWarning() << "[DwarfCameraController] setExposureMode" << mode << "for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_exp_mode(mode);
  // Use sendSingleInt32 which always serializes the field (even for value 0)
  sendSingleInt32(kind, cmdSetExpModeFor(kind), mode);

  saveConfig(kind);

  // Fetch params to sync UI with actual values (especially when switching
  // Auto->Manual) Add a small delay to allow the camera to process the mode
  // change
  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setExposureIndex(CameraKind kind, int index) {
  qWarning() << "[DwarfCameraController] setExposureIndex" << index << "for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_exp_index(index);
  sendSingleInt32(kind, cmdSetExpFor(kind), index);

  saveConfig(kind);

  // Refresh from device to see actual/clamped exposure index
  QTimer::singleShot(250, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setGainMode(CameraKind kind, int mode) {
  qWarning() << "[DwarfCameraController] setGainMode" << mode << "for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_gain_mode(mode);
  sendSingleInt32(kind, cmdSetGainModeFor(kind), mode);

  saveConfig(kind);

  // Fetch params to sync UI
  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });

  saveConfig(kind);
}

void DwarfCameraController::setGainIndex(CameraKind kind, int index) {
  qWarning() << "[DwarfCameraController] setGainIndex" << index << "for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_gain_index(index);
  sendSingleInt32(kind, cmdSetGainFor(kind), index);

  saveConfig(kind);
}

void DwarfCameraController::setIrCut(CameraKind kind, int value) {
  if (kind != CameraKind::Tele)
    return;

  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  value = clampInt(value, 0, 1);
  p.set_ircut_value(value);
  sendSingleInt32(kind, cmdSetIrCutFor(kind), value);

  saveConfig(kind);
}

void DwarfCameraController::setWhiteBalanceMode(CameraKind kind, int mode) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_wb_mode(mode);

  if (kind == CameraKind::Tele) {
    // Use specific command for Tele (10025)
    sendSingleInt32(kind, 10025u, mode);
  } else {
    // Fallback for Wide (ID unknown/unverified)
    sendSetAllParams(kind);
  }

  // Fetch params to sync UI
  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setWhiteBalanceByTemperature(CameraKind kind,
                                                         int ctIndex) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_wb_index_type(0); // 0: color temperature
  p.set_wb_index(ctIndex);

  if (kind == CameraKind::Tele) {
    // Use specific command for Tele (10029)
    sendSingleInt32(kind, 10029u, ctIndex);
  } else {
    // Fallback for Wide
    sendSetAllParams(kind);
  }

  saveConfig(kind);
}

void DwarfCameraController::setBrightness(CameraKind kind, int value) {
  qWarning() << "[DwarfCameraController] setBrightness" << value << "for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_brightness(scaledValue);
  sendSingleInt32(kind, cmdSetBrightnessFor(kind), scaledValue);

  saveConfig(kind);

  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setContrast(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_contrast(scaledValue);
  sendSingleInt32(kind, cmdSetContrastFor(kind), scaledValue);

  saveConfig(kind);

  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setHue(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleHueToApi(value);
  p.set_hue(scaledValue);
  sendSingleInt32(kind, cmdSetHueFor(kind), scaledValue);

  saveConfig(kind);

  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setSaturation(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_saturation(scaledValue);
  sendSingleInt32(kind, cmdSetSaturationFor(kind), scaledValue);

  saveConfig(kind);

  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

void DwarfCameraController::setSharpness(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  value = clampInt(value, 0, 100);
  p.set_sharpness(value);
  sendSingleInt32(kind, cmdSetSharpnessFor(kind), value);

  saveConfig(kind);

  QTimer::singleShot(200, this, [this, kind]() { fetchAllParams(kind); });
}

bool DwarfCameraController::irCutEnabled(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.ircut_value() != 0;
}

int DwarfCameraController::brightness(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return scaleFromApi255(p.brightness());
}

int DwarfCameraController::contrast(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return scaleFromApi255(p.contrast());
}

int DwarfCameraController::saturation(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return scaleFromApi255(p.saturation());
}

int DwarfCameraController::hue(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return scaleHueFromApi(p.hue());
}

int DwarfCameraController::sharpness(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return clampInt(p.sharpness(), 0, 100);
}

int DwarfCameraController::exposureMode(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.exp_mode();
}

int DwarfCameraController::exposureIndex(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.exp_index();
}

int DwarfCameraController::gainMode(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.gain_mode();
}

int DwarfCameraController::gainIndex(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.gain_index();
}

int DwarfCameraController::whiteBalanceMode(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.wb_mode();
}

int DwarfCameraController::whiteBalanceTemperatureIndex(CameraKind kind) const {
  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  return p.wb_index();
}

void DwarfCameraController::fetchAllParams(CameraKind kind) {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfCameraController] Cannot fetch params: not connected";
    emit errorOccurred("Camera client not connected");
    return;
  }

  // CMD_CAMERA_TELE_GET_ALL_PARAMS = 10036, WIDE = 12029
  quint32 cmd = (kind == CameraKind::Tele) ? 10036u : 12029u;
  qWarning() << "[DwarfCameraController] Fetching all params for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide");

  // Empty request body for GET_ALL_PARAMS
  m_client->sendCommand(moduleIdFor(kind), cmd, QByteArray());
}

void DwarfCameraController::handleCameraMessage(quint32 moduleId, quint32 cmd,
                                                const QByteArray &data) {
  // Determine camera kind from module ID
  CameraKind kind = (moduleId == 1) ? CameraKind::Tele : CameraKind::Wide;

  if ((moduleId == 1 || moduleId == 2) && (cmd == 10038 || cmd == 12038)) {
    dwarf::ResGetAllFeatureParams response;
    if (response.ParseFromArray(data.data(), data.size())) {
      qWarning() << "[DwarfCameraController] Received all FEATURE params for"
                 << (kind == CameraKind::Tele ? "Tele" : "Wide")
                 << "code=" << response.code()
                 << "param_count=" << response.all_feature_params_size();

      const int limit = std::min(response.all_feature_params_size(), 80);
      for (int i = 0; i < limit; ++i) {
        const auto &param = response.all_feature_params(i);
        if (param.id() == 6 || param.id() == 7) {
          qWarning() << "[DwarfCameraController] FEATURE Param id=" << param.id()
                     << "auto_mode=" << param.auto_mode()
                     << "mode_index=" << param.mode_index()
                     << "index=" << param.index()
                     << "continue_value=" << param.continue_value();
        }
      }

      qWarning() << "[DwarfCameraController] FEATURE params dump (first" << limit << ")"
                 << (kind == CameraKind::Tele ? "Tele" : "Wide");
      for (int i = 0; i < limit; ++i) {
        const auto &param = response.all_feature_params(i);
        qWarning() << "[DwarfCameraController] FEATURE Param id=" << param.id()
                   << "auto_mode=" << param.auto_mode()
                   << "mode_index=" << param.mode_index()
                   << "index=" << param.index()
                   << "continue_value=" << param.continue_value();
      }
    } else {
      qWarning() << "[DwarfCameraController] Failed to parse FEATURE params response for"
                 << (kind == CameraKind::Tele ? "Tele" : "Wide")
                 << ", data size:"
                 << data.size();
    }
    return;
  }

  // Handle SET_FEATURE_PARAM response (CMD 10037 on both Tele/Wide modules)
  if ((moduleId == 1 || moduleId == 2) && (cmd == 10037 || cmd == 12037)) {
    dwarf::ComResponse res;
    const bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    qWarning() << "[DwarfCameraController] SetFeatureParam response for"
               << (kind == CameraKind::Tele ? "Tele" : "Wide")
               << "cmd=" << cmd
               << "ok=" << ok << "code=" << code
               << "data_size=" << data.size();
    return;
  }

  // Handle SET_EXP response (Tele: 10009, Wide: 12004)
  if ((moduleId == 1 && cmd == 10009) || (moduleId == 2 && cmd == 12004)) {
    dwarf::ComResponse res;
    const bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    qWarning() << "[DwarfCameraController] SetExposure response for"
               << (kind == CameraKind::Tele ? "Tele" : "Wide")
               << "ok=" << ok << "code=" << code;
    return;
  }

  // Handle PHOTO response (10002 for Tele, 12022 for Wide)
  if ((moduleId == 1 && cmd == 10002) || (moduleId == 2 && cmd == 12022)) {
    dwarf::ComResponse res;
    bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    const bool success = ok && (code == 0);

    qWarning() << "[DwarfCameraController] Photo response for"
               << (kind == CameraKind::Tele ? "Tele" : "Wide")
               << "success=" << success << "code=" << code;

    if (success)
      emit photoTaken(kind);
    emit photoCaptureFinished(kind, success, code, QString());
    return;
  }

  // Handle START_RECORD response (Tele only: 10005)
  if (moduleId == 1 && cmd == 10005) {
    dwarf::ComResponse res;
    bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    const bool success = ok && (code == 0);

    qWarning() << "[DwarfCameraController] StartRecord response success="
               << success << "code=" << code;
    emit recordFinished(CameraKind::Tele, true, success, code);
    return;
  }

  // Some firmwares also respond on the WIDE camera module with the same cmd.
  // We log it for diagnostics but do not treat it as recording state.
  if (moduleId == 2 && cmd == 10005) {
    dwarf::ComResponse res;
    bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    qWarning() << "[DwarfCameraController] Wide module returned cmd 10005 (ignored), code="
               << code;
    return;
  }

  // Handle STOP_RECORD response (Tele only: 10006)
  if (moduleId == 1 && cmd == 10006) {
    dwarf::ComResponse res;
    bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    const bool success = ok && (code == 0 || code == -10518);

    qWarning() << "[DwarfCameraController] StopRecord response success="
               << success << "code=" << code;
    emit recordFinished(CameraKind::Tele, false, success, code);
    return;
  }

  if (moduleId == 2 && cmd == 10006) {
    dwarf::ComResponse res;
    bool ok = res.ParseFromArray(data.data(), data.size());
    const int code = ok ? res.code() : -1;
    qWarning() << "[DwarfCameraController] Wide module returned cmd 10006 (ignored), code="
               << code;
    return;
  }

  // Handle GET_ALL_PARAMS response (10036 for Tele, 12029 for Wide)
  if ((moduleId == 1 && cmd == 10036) || (moduleId == 2 && cmd == 12029)) {
    dwarf::ResGetAllParams response;
    if (response.ParseFromArray(data.data(), data.size())) {
      qWarning() << "[DwarfCameraController] Received all params for"
                 << (kind == CameraKind::Tele ? "Tele" : "Wide")
                 << "code=" << response.code()
                 << "param_count=" << response.all_params_size();

      // Update local params from response
      // The response contains repeated CommonParam with id, auto_mode, index
      // fields
      ReqSetAllParams &p =
          (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;

      for (const auto &param : response.all_params()) {
        int id = param.id();
        int autoMode = param.auto_mode(); // 0=Auto, 1=Manual
        int index = param.index();

        // Keep logging focused; full spam makes debugging harder.
        if (id == 1 || id == 2) {
          qWarning() << "[DwarfCameraController] Param id=" << id
                     << "auto_mode=" << autoMode << "index=" << index;
        }

        // Map parameter IDs to our local params
        // Based on typical DWARF parameter IDs:
        switch (id) {
        case 1: // Exposure
          p.set_exp_mode(autoMode);
          p.set_exp_index(index);
          break;
        case 2: // Gain
          p.set_gain_mode(autoMode);
          p.set_gain_index(index);
          break;
        case 3: // Brightness
          p.set_brightness(index);
          break;
        case 4: // Contrast
          p.set_contrast(index);
          break;
        case 5: // Saturation
          p.set_saturation(index);
          break;
        case 6: // Hue
          p.set_hue(index);
          break;
        case 7: // Sharpness
          p.set_sharpness(index);
          break;
        case 8: // White Balance
          p.set_wb_mode(autoMode);
          p.set_wb_index_type(param.mode_index());
          p.set_wb_index(index);
          break;
        case 9: // IR-Cut
          p.set_ircut_value(index);
          break;
        default:
          qWarning() << "[DwarfCameraController] Unknown param id:" << id;
          break;
        }
      }

      emit allParamsReceived(kind);
    } else {
      qWarning() << "[DwarfCameraController] Failed to parse ResGetAllParams, "
                    "data size:"
                 << data.size();
    }
  }
}

void DwarfCameraController::sendSetAllParams(CameraKind kind) {
  qWarning() << "[DwarfCameraController] sendSetAllParams called for"
             << (kind == CameraKind::Tele ? "Tele" : "Wide")
             << "m_client=" << m_client
             << "isConnected=" << (m_client ? m_client->isConnected() : false);

  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfCameraController] ERROR: Camera client not connected!";
    emit errorOccurred("Camera client not connected");
    return;
  }

  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;

  QByteArray data = ProtobufHelper::serialize(p);
  qWarning() << "[DwarfCameraController] Sending SetAllParams module="
             << moduleIdFor(kind) << "cmd=" << cmdSetAllParamsFor(kind)
             << "dataSize=" << data.size();
  m_client->sendCommand(moduleIdFor(kind), cmdSetAllParamsFor(kind), data);
}

quint32 DwarfCameraController::moduleIdFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 1u : 2u; // MODULE_CAMERA_TELE / _WIDE
}

quint32 DwarfCameraController::cmdSetAllParamsFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10035u : 12028u;
}

quint32 DwarfCameraController::cmdOpenCameraFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10000u : 12000u;
}

quint32 DwarfCameraController::cmdCloseCameraFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10001u : 12001u;
}

quint32 DwarfCameraController::cmdPhotoFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10002u : 12022u;
}

quint32 DwarfCameraController::cmdStartRecordFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10005u : 0u;
}

quint32 DwarfCameraController::cmdStopRecordFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10006u : 0u;
}

// Individual parameter command IDs (SET commands, not GET!)
// Tele: SET = odd numbers, GET = even numbers
// Wide: similar pattern starting at 12000
quint32 DwarfCameraController::cmdSetExpModeFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10007u : 12002u; // SET_EXP_MODE
}

quint32 DwarfCameraController::cmdSetExpFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10009u : 12004u; // SET_EXP
}

quint32 DwarfCameraController::cmdSetGainModeFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10011u : 12005u; // SET_GAIN_MODE
}

quint32 DwarfCameraController::cmdSetGainFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10013u : 12006u; // SET_GAIN
}

quint32 DwarfCameraController::cmdSetBrightnessFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10015u : 12008u; // SET_BRIGHTNESS
}

quint32 DwarfCameraController::cmdSetContrastFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10017u : 12010u; // SET_CONTRAST
}

quint32 DwarfCameraController::cmdSetSaturationFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10019u : 12012u; // SET_SATURATION
}

quint32 DwarfCameraController::cmdSetHueFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10021u : 12014u; // SET_HUE
}

quint32 DwarfCameraController::cmdSetSharpnessFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10023u : 12016u; // SET_SHARPNESS
}

quint32 DwarfCameraController::cmdSetIrCutFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10031u : 12018u; // SET_IRCUT
}

void DwarfCameraController::sendSingleInt32(CameraKind kind, quint32 cmd,
                                            int value) {
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfCameraController] ERROR: Camera client not connected!";
    emit errorOccurred("Camera client not connected");
    return;
  }

  // Create a simple protobuf message with just an int32 value field
  // The DWARF API expects: message { int32 value = 1; }
  QByteArray data;
  // Protobuf encoding: field 1, wire type 0 (varint) = 0x08, then varint value
  data.append(static_cast<char>(0x08)); // field 1, varint
  // Encode value as varint
  uint32_t uval = static_cast<uint32_t>(value);
  while (uval >= 0x80) {
    data.append(static_cast<char>((uval & 0x7F) | 0x80));
    uval >>= 7;
  }
  data.append(static_cast<char>(uval));

  qWarning() << "[DwarfCameraController] Sending single param module="
             << moduleIdFor(kind) << "cmd=" << cmd << "value=" << value
             << "dataSize=" << data.size();
  m_client->sendCommand(moduleIdFor(kind), cmd, data);
}
