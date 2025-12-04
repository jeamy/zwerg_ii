#include "DwarfCameraController.h"
#include "ProtobufHelper.h"

#include <QDebug>
#include <algorithm>

using dwarf::ReqOpenCamera;
using dwarf::ReqPhoto;
using dwarf::ReqStartRecord;
using dwarf::ReqStopRecord;
using dwarf::ReqSetAllParams;

namespace {
inline int clampInt(int value, int minV, int maxV) {
  return std::max(minV, std::min(maxV, value));
}

inline int scale01To255(int value01) {
  int v = clampInt(value01, 0, 100);
  return v * 255 / 100;
}
} // namespace

DwarfCameraController::DwarfCameraController(QObject *parent)
    : QObject(parent), m_client(nullptr) {
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
  m_wideParams.set_exp_index(8);     // Mid-range for wide
  m_wideParams.set_gain_mode(0);
  m_wideParams.set_gain_index(5);    // Mid-range for wide (0-100)
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
}

void DwarfCameraController::setClient(DwarfWebSocketClient *client) {
  m_client = client;
  qWarning() << "[DwarfCameraController] setClient called with"
             << (client ? "valid" : "null") << "client";
}

void DwarfCameraController::openCamera(CameraKind kind, bool binning,
                                       int rtspEncodeType) {
  qWarning() << "[DwarfCameraController] openCamera kind" << static_cast<int>(kind)
             << "binning" << binning << "rtspEncodeType" << rtspEncodeType;
  if (!m_client || !m_client->isConnected()) {
    qWarning() << "[DwarfCameraController] Cannot open camera, client not connected";
    emit errorOccurred("Camera client not connected");
    return;
  }

  ReqOpenCamera req;
  req.set_binning(binning);
  req.set_rtsp_encode_type(rtspEncodeType);

  qDebug() << "Sending OpenCamera for kind" << (int)kind << "binning" << binning;

  const QByteArray data = ProtobufHelper::serialize(req);
  m_client->sendCommand(moduleIdFor(kind), cmdOpenCameraFor(kind), data);
}

void DwarfCameraController::closeCamera(CameraKind kind) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

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
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_exp_mode(mode);
  sendSetAllParams(kind);
}

void DwarfCameraController::setExposureIndex(CameraKind kind, int index) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_exp_index(index);
  sendSetAllParams(kind);
}

void DwarfCameraController::setGainMode(CameraKind kind, int mode) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_gain_mode(mode);
  sendSetAllParams(kind);
}

void DwarfCameraController::setGainIndex(CameraKind kind, int index) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_gain_index(index);
  sendSetAllParams(kind);
}

void DwarfCameraController::setIrCut(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  value = clampInt(value, 0, 1);
  p.set_ircut_value(value);
  sendSetAllParams(kind);
}

void DwarfCameraController::setWhiteBalanceMode(CameraKind kind, int mode) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_wb_mode(mode);
  sendSetAllParams(kind);
}

void DwarfCameraController::setWhiteBalanceByTemperature(CameraKind kind,
                                                         int ctIndex) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_wb_index_type(0); // 0: color temperature
  p.set_wb_index(ctIndex);
  sendSetAllParams(kind);
}

void DwarfCameraController::setBrightness(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_brightness(scale01To255(value));
  sendSetAllParams(kind);
}

void DwarfCameraController::setContrast(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_contrast(scale01To255(value));
  sendSetAllParams(kind);
}

void DwarfCameraController::setHue(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_hue(scale01To255(value));
  sendSetAllParams(kind);
}

void DwarfCameraController::setSaturation(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_saturation(scale01To255(value));
  sendSetAllParams(kind);
}

void DwarfCameraController::setSharpness(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  value = clampInt(value, 0, 100);
  p.set_sharpness(value);
  sendSetAllParams(kind);
}

void DwarfCameraController::sendSetAllParams(CameraKind kind) {
  if (!m_client || !m_client->isConnected()) {
    emit errorOccurred("Camera client not connected");
    return;
  }

  const ReqSetAllParams &p =
      (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;

  QByteArray data = ProtobufHelper::serialize(p);
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
