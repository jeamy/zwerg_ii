#include "DwarfCameraController.h"
#include "ProtobufHelper.h"

#include <QDebug>
#include <algorithm>

using dwarf::ReqOpenCamera;
using dwarf::ReqPhoto;
using dwarf::ReqStartRecord;
using dwarf::ReqStopRecord;
using dwarf::ReqSetAllParams;
using dwarf::ResGetAllParams;
using dwarf::ReqSetExpMode;
using dwarf::ReqSetExp;
using dwarf::ReqSetGainMode;
using dwarf::ReqSetGain;
using dwarf::ReqSetBrightness;
using dwarf::ReqSetContrast;
using dwarf::ReqSetSaturation;
using dwarf::ReqSetHue;
using dwarf::ReqSetSharpness;
using dwarf::ReqSetIRCut;
using dwarf::ReqSetWBMode;

namespace {
inline int clampInt(int value, int minV, int maxV) {
  return std::max(minV, std::min(maxV, value));
}

// Scale UI value (0-100) to API value (0-255) for brightness/contrast/saturation
// Formula: B = (A + 100) * 255.0 / 200, where A is -100 to 100
// For our 0-100 slider: map 0->0, 50->127.5, 100->255
inline int scaleToApi255(int value0to100) {
  int v = clampInt(value0to100, 0, 100);
  // Map 0-100 to -100 to 100, then apply formula
  int a = (v * 2) - 100;  // 0->-100, 50->0, 100->100
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
  qWarning() << "[DwarfCameraController] setExposureMode" << mode
             << "for" << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_exp_mode(mode);
  // Use sendSingleInt32 which always serializes the field (even for value 0)
  sendSingleInt32(kind, cmdSetExpModeFor(kind), mode);
}

void DwarfCameraController::setExposureIndex(CameraKind kind, int index) {
  qWarning() << "[DwarfCameraController] setExposureIndex" << index
             << "for" << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_exp_index(index);
  sendSingleInt32(kind, cmdSetExpFor(kind), index);
}

void DwarfCameraController::setGainMode(CameraKind kind, int mode) {
  qWarning() << "[DwarfCameraController] setGainMode" << mode
             << "for" << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  mode = clampInt(mode, 0, 1);
  p.set_gain_mode(mode);
  sendSingleInt32(kind, cmdSetGainModeFor(kind), mode);
}

void DwarfCameraController::setGainIndex(CameraKind kind, int index) {
  qWarning() << "[DwarfCameraController] setGainIndex" << index
             << "for" << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  p.set_gain_index(index);
  sendSingleInt32(kind, cmdSetGainFor(kind), index);
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
  qWarning() << "[DwarfCameraController] setBrightness" << value
             << "for" << (kind == CameraKind::Tele ? "Tele" : "Wide");
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_brightness(scaledValue);
  sendSingleInt32(kind, cmdSetBrightnessFor(kind), scaledValue);
}

void DwarfCameraController::setContrast(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_contrast(scaledValue);
  sendSingleInt32(kind, cmdSetContrastFor(kind), scaledValue);
}

void DwarfCameraController::setHue(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleHueToApi(value);
  p.set_hue(scaledValue);
  sendSingleInt32(kind, cmdSetHueFor(kind), scaledValue);
}

void DwarfCameraController::setSaturation(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  int scaledValue = scaleToApi255(value);
  p.set_saturation(scaledValue);
  sendSingleInt32(kind, cmdSetSaturationFor(kind), scaledValue);
}

void DwarfCameraController::setSharpness(CameraKind kind, int value) {
  ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
  value = clampInt(value, 0, 100);
  p.set_sharpness(value);
  sendSingleInt32(kind, cmdSetSharpnessFor(kind), value);
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
  
  // Handle GET_ALL_PARAMS response (10036 for Tele, 12029 for Wide)
  if ((moduleId == 1 && cmd == 10036) || (moduleId == 2 && cmd == 12029)) {
    dwarf::ResGetAllParams response;
    if (response.ParseFromArray(data.data(), data.size())) {
      qWarning() << "[DwarfCameraController] Received all params for"
                 << (kind == CameraKind::Tele ? "Tele" : "Wide")
                 << "code=" << response.code()
                 << "param_count=" << response.all_params_size();
      
      // Update local params from response
      // The response contains repeated CommonParam with id, auto_mode, index fields
      ReqSetAllParams &p = (kind == CameraKind::Tele) ? m_teleParams : m_wideParams;
      
      for (const auto &param : response.all_params()) {
        int id = param.id();
        int autoMode = param.auto_mode();  // 0=Auto, 1=Manual
        int index = param.index();
        
        qWarning() << "[DwarfCameraController] Param id=" << id
                   << "auto_mode=" << autoMode << "index=" << index;
        
        // Map parameter IDs to our local params
        // Based on typical DWARF parameter IDs:
        switch (id) {
          case 1:  // Exposure
            p.set_exp_mode(autoMode);
            p.set_exp_index(index);
            break;
          case 2:  // Gain
            p.set_gain_mode(autoMode);
            p.set_gain_index(index);
            break;
          case 3:  // Brightness
            p.set_brightness(index);
            break;
          case 4:  // Contrast
            p.set_contrast(index);
            break;
          case 5:  // Saturation
            p.set_saturation(index);
            break;
          case 6:  // Hue
            p.set_hue(index);
            break;
          case 7:  // Sharpness
            p.set_sharpness(index);
            break;
          case 8:  // White Balance
            p.set_wb_mode(autoMode);
            p.set_wb_index(index);
            break;
          case 9:  // IR-Cut
            p.set_ircut_value(index);
            break;
          default:
            qWarning() << "[DwarfCameraController] Unknown param id:" << id;
            break;
        }
      }
      
      emit allParamsReceived(kind);
    } else {
      qWarning() << "[DwarfCameraController] Failed to parse ResGetAllParams, data size:" << data.size();
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
  return (kind == CameraKind::Tele) ? 10007u : 12002u;  // SET_EXP_MODE
}

quint32 DwarfCameraController::cmdSetExpFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10009u : 12004u;  // SET_EXP
}

quint32 DwarfCameraController::cmdSetGainModeFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10011u : 12005u;  // SET_GAIN_MODE
}

quint32 DwarfCameraController::cmdSetGainFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10013u : 12006u;  // SET_GAIN
}

quint32 DwarfCameraController::cmdSetBrightnessFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10015u : 12008u;  // SET_BRIGHTNESS
}

quint32 DwarfCameraController::cmdSetContrastFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10017u : 12010u;  // SET_CONTRAST
}

quint32 DwarfCameraController::cmdSetSaturationFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10019u : 12012u;  // SET_SATURATION
}

quint32 DwarfCameraController::cmdSetHueFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10021u : 12014u;  // SET_HUE
}

quint32 DwarfCameraController::cmdSetSharpnessFor(CameraKind kind) const {
  return (kind == CameraKind::Tele) ? 10023u : 12016u;  // SET_SHARPNESS
}

void DwarfCameraController::sendSingleInt32(CameraKind kind, quint32 cmd, int value) {
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
