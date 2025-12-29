#include "DwarfAstroController.h"

#include "DwarfWebSocketClient.h"
#include "base.pb.h"
#include "astro.pb.h"
#include "notify.pb.h"

#include <QDebug>
#include <QHash>
#include <QTimer>
#include <QVector>

namespace {
void appendVarintU64(QByteArray &buf, quint64 value);
void appendInt64Varint(QByteArray &buf, qint64 value);
}

// Astro module command IDs (module 8)
namespace AstroCmd {
    constexpr quint32 START_CALIBRATION = 11000;
    constexpr quint32 STOP_CALIBRATION = 11001;
    constexpr quint32 GOTO_DSO = 11002;
    constexpr quint32 GOTO_SOLAR_SYSTEM = 11003;
    constexpr quint32 STOP_GOTO = 11004;
    constexpr quint32 CAPTURE_RAW_LIVE_STACKING = 11005;
    constexpr quint32 STOP_CAPTURE_RAW_LIVE_STACKING = 11006;
    constexpr quint32 CAPTURE_DARK_FRAME = 11007;
    constexpr quint32 STOP_CAPTURE_DARK_FRAME = 11008;
    constexpr quint32 CHECK_DARK_FRAME = 11009;
    constexpr quint32 GO_LIVE = 11010;
    constexpr quint32 TRACK_SPECIAL_TARGET = 11011;
    constexpr quint32 STOP_TRACK_SPECIAL_TARGET = 11012;
    constexpr quint32 ONE_CLICK_GOTO_DSO = 11013;
    constexpr quint32 ONE_CLICK_GOTO_SOLAR = 11014;
    constexpr quint32 STOP_ONE_CLICK_GOTO = 11015;
    constexpr quint32 CAPTURE_WIDE_RAW_LIVE_STACKING = 11016;
    constexpr quint32 STOP_CAPTURE_WIDE_RAW_LIVE_STACKING = 11017;
    constexpr quint32 START_EQ_SOLVING = 11018;
    constexpr quint32 STOP_EQ_SOLVING = 11019;
    constexpr quint32 CAPTURE_DARK_FRAME_WITH_PARAM = 11021;
    constexpr quint32 GET_DARK_FRAME_LIST = 11022;
    constexpr quint32 DEL_DARK_FRAME_LIST = 11023;
}

DwarfAstroController::DwarfAstroController(QObject *parent)
    : QObject(parent)
{
}

void DwarfAstroController::setClient(DwarfWebSocketClient *client) {
    m_client = client;
}

void DwarfAstroController::sendCommand(quint32 cmd, const QByteArray &data) {
    if (!m_client) {
        qWarning() << "AstroController: No client set";
        return;
    }
    
    qDebug() << "[AstroController] Sending command - Module: 3, Cmd:" << cmd << "Data size:" << data.size();
    // Module 3 = Astro module (not 8, that's Focus!)
    m_client->sendCommand(3, cmd, data);
}

// ============================================================================
// Calibration
// ============================================================================

void DwarfAstroController::startCalibration() {
    qDebug() << "Starting calibration...";
    dwarf::ReqStartCalibration req;
    sendCommand(AstroCmd::START_CALIBRATION, 
                QByteArray::fromStdString(req.SerializeAsString()));
    emit calibrationStarted();
}

void DwarfAstroController::stopCalibration() {
    qDebug() << "Stopping calibration...";
    dwarf::ReqStopCalibration req;
    sendCommand(AstroCmd::STOP_CALIBRATION,
                QByteArray::fromStdString(req.SerializeAsString()));
}

// ============================================================================
// GOTO
// ============================================================================

void DwarfAstroController::gotoDSO(double ra, double dec, const QString &targetName) {
    // Convert RA from degrees (0-360) to hours (0-24)
    double raHours = ra / 15.0;
    
    qDebug() << "GOTO DSO:" << targetName 
             << "RA:" << ra << "deg (" << raHours << "hours)"
             << "Dec:" << dec << "deg";
    
    dwarf::ReqGotoDSO req;
    req.set_ra(raHours);  // API expects hours
    req.set_dec(dec);     // Dec is already in degrees
    req.set_target_name(targetName.toStdString());
    
    sendCommand(AstroCmd::GOTO_DSO,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit gotoStarted(targetName);
}

void DwarfAstroController::oneClickGotoDSO(double ra, double dec, const QString &targetName) {
    // Convert RA from degrees (0-360) to hours (0-24)
    double raHours = ra / 15.0;
    
    qWarning() << "=== [GOTO DEBUG] Starting One-Click GOTO DSO ===";
    qWarning() << "    Target:" << targetName;
    qWarning() << "    Input RA (deg):" << ra;
    qWarning() << "    Input Dec (deg):" << dec;
    qWarning() << "    Sending RA (hours):" << raHours;
    
    dwarf::ReqOneClickGotoDSO req;
    req.set_ra(raHours);  // API expects hours
    req.set_dec(dec);     // Dec is already in degrees
    req.set_target_name(targetName.toStdString());
    
    sendCommand(AstroCmd::ONE_CLICK_GOTO_DSO,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit gotoStarted(targetName);
}

void DwarfAstroController::stopGoto() {
    qDebug() << "Stopping GOTO...";
    dwarf::ReqStopGoto req;
    sendCommand(AstroCmd::STOP_GOTO,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::gotoSolarSystem(int index, double lon, double lat, const QString &targetName) {
    qDebug() << "GOTO Solar System:" << targetName << "index:" << index << "lon:" << lon << "lat:" << lat;
    
    dwarf::ReqGotoSolarSystem req;
    req.set_index(index);
    req.set_lon(lon);
    req.set_lat(lat);
    req.set_target_name(targetName.toStdString());
    
    sendCommand(AstroCmd::GOTO_SOLAR_SYSTEM,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit gotoStarted(targetName);
}

void DwarfAstroController::oneClickGotoSolarSystem(int index, double lon, double lat, const QString &targetName) {
    qDebug() << "One-Click GOTO Solar System:" << targetName;
    
    dwarf::ReqOneClickGotoSolarSystem req;
    req.set_index(index);
    req.set_lon(lon);
    req.set_lat(lat);
    req.set_target_name(targetName.toStdString());
    
    sendCommand(AstroCmd::ONE_CLICK_GOTO_SOLAR,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit gotoStarted(targetName);
}

// ============================================================================
// Special Target Tracking (Sun/Moon)
// ============================================================================

void DwarfAstroController::trackSpecialTarget(int index, double lon, double lat) {
    qDebug() << "Track special target:" << (index == 0 ? "Sun" : "Moon");
    
    dwarf::ReqTrackSpecialTarget req;
    req.set_index(index);
    req.set_lon(lon);
    req.set_lat(lat);
    
    sendCommand(AstroCmd::TRACK_SPECIAL_TARGET,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::stopTrackSpecialTarget() {
    dwarf::ReqStopTrackSpecialTarget req;
    sendCommand(AstroCmd::STOP_TRACK_SPECIAL_TARGET,
                QByteArray::fromStdString(req.SerializeAsString()));
}

// ============================================================================
// Stacking
// ============================================================================

void DwarfAstroController::startLiveStacking() {
    qWarning() << "=== DwarfAstroController::startLiveStacking() called";
    qWarning() << "    Module: 3, Cmd: 11005 (CAPTURE_RAW_LIVE_STACKING)";
    QByteArray data;
    data.append(static_cast<char>(0x08));
    appendInt64Varint(data, static_cast<qint64>(-1));
    qWarning() << "    Payload size:" << data.size() << "bytes";
    sendCommand(AstroCmd::CAPTURE_RAW_LIVE_STACKING, data);
    qWarning() << "    Command sent, emitting stackingStarted signal";
    qWarning() << "    Waiting for response from Module 3 Cmd 11005...";
    qWarning() << "    If no progress notifications (15209) come within 5s, stacking failed to start!";
    emit stackingStarted();
}

void DwarfAstroController::stopLiveStacking() {
    qDebug() << "Stopping live stacking...";
    dwarf::ReqStopCaptureRawLiveStacking req;
    sendCommand(AstroCmd::STOP_CAPTURE_RAW_LIVE_STACKING,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit stackingStopped();
}

void DwarfAstroController::startWideLiveStacking() {
    QByteArray data;
    data.append(static_cast<char>(0x08));
    appendInt64Varint(data, static_cast<qint64>(-1));
    sendCommand(AstroCmd::CAPTURE_WIDE_RAW_LIVE_STACKING, data);
    emit stackingStarted();
}

void DwarfAstroController::stopWideLiveStacking() {
    dwarf::ReqStopCaptureWideRawLiveStacking req;
    sendCommand(AstroCmd::STOP_CAPTURE_WIDE_RAW_LIVE_STACKING,
                QByteArray::fromStdString(req.SerializeAsString()));
    emit stackingStopped();
}

// ============================================================================
// Dark Frames
// ============================================================================

void DwarfAstroController::checkDarkFrame() {
    dwarf::ReqCheckDarkFrame req;
    sendCommand(AstroCmd::CHECK_DARK_FRAME,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::captureDarkFrame(bool reshoot) {
    qDebug() << "Capturing dark frame, reshoot:" << reshoot;
    dwarf::ReqCaptureDarkFrame req;
    req.set_reshoot(reshoot ? 1 : 0);
    sendCommand(AstroCmd::CAPTURE_DARK_FRAME,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::captureDarkFrameWithParams(int expIndex, int gainIndex, int binIndex, int count) {
    qDebug() << "Capturing dark frame with params - exp:" << expIndex << "gain:" << gainIndex << "count:" << count;
    dwarf::ReqCaptureDarkFrameWithParam req;
    req.set_exp_index(expIndex);
    req.set_gain_index(gainIndex);
    req.set_bin_index(binIndex);
    req.set_cap_size(count);
    sendCommand(AstroCmd::CAPTURE_DARK_FRAME_WITH_PARAM,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::stopCaptureDarkFrame() {
    dwarf::ReqStopCaptureDarkFrame req;
    sendCommand(AstroCmd::STOP_CAPTURE_DARK_FRAME,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::getDarkFrameList() {
    dwarf::ReqGetDarkFrameList req;
    sendCommand(AstroCmd::GET_DARK_FRAME_LIST,
                QByteArray::fromStdString(req.SerializeAsString()));
}

// ============================================================================
// EQ Alignment
// ============================================================================

void DwarfAstroController::startEqSolving(double lon, double lat) {
    qDebug() << "Starting EQ solving at lon:" << lon << "lat:" << lat;
    dwarf::ReqStartEqSolving req;
    req.set_lon(lon);
    req.set_lat(lat);
    sendCommand(AstroCmd::START_EQ_SOLVING,
                QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfAstroController::stopEqSolving() {
    dwarf::ReqStopEqSolving req;
    sendCommand(AstroCmd::STOP_EQ_SOLVING,
                QByteArray::fromStdString(req.SerializeAsString()));
}

// ============================================================================
// Go Live
// ============================================================================

void DwarfAstroController::goLive() {
    qDebug() << "Go Live!";
    dwarf::ReqGoLive req;
    sendCommand(AstroCmd::GO_LIVE,
                QByteArray::fromStdString(req.SerializeAsString()));
}

// ============================================================================
// Message Handler
// ============================================================================

void DwarfAstroController::handleAstroMessage(quint32 cmd, const QByteArray &data) {
    qWarning() << "[AstroController::handleAstroMessage] Cmd:" << cmd << "Size:" << data.size();
    switch (cmd) {
        case AstroCmd::CHECK_DARK_FRAME: {
            dwarf::ResCheckDarkFrame res;
            if (res.ParseFromArray(data.data(), data.size())) {
                // progress is 0-10000 (2 decimal places), convert to percentage
                emit darkFrameProgress(res.progress() / 100, 100);
            }
            break;
        }
        
        case AstroCmd::ONE_CLICK_GOTO_DSO:
        case AstroCmd::ONE_CLICK_GOTO_SOLAR: {
            dwarf::ResOneClickGoto res;
            if (res.ParseFromArray(data.data(), data.size())) {
                qWarning() << "=== GOTO progress: step" << res.step() << "all_end" << res.all_end() << "code" << res.code();
                emit gotoProgress(res.step());
                if (res.all_end()) {
                    if (res.code() == 0) {
                        qWarning() << "    ✓ GOTO completed successfully";
                        emit gotoCompleted();
                    } else {
                        QString errorMsg;
                        switch (res.code()) {
                            case -10501:
                                errorMsg = "Camera is closed! GOTO requires an open camera for plate solving.\n\n"
                                          "Solution: Do NOT use 'Go Live' or stop stacking before GOTO.\n"
                                          "Restart the app if needed.";
                                break;
                            default:
                                errorMsg = QString("Error code: %1").arg(res.code());
                                break;
                        }
                        qCritical() << "    ✗ GOTO failed:" << errorMsg;
                        emit gotoFailed(errorMsg);
                    }
                }
            } else {
                qWarning() << "[GOTO] Failed to parse response, raw hex:" << data.toHex();
            }
            break;
        }
        
        case AstroCmd::START_EQ_SOLVING: {
            dwarf::ResStartEqSolving res;
            if (res.ParseFromArray(data.data(), data.size())) {
                if (res.code() == 0) {
                    emit eqSolvingResult(res.azi_err(), res.alt_err());
                }
            }
            break;
        }
        
        case AstroCmd::GET_DARK_FRAME_LIST: {
            dwarf::ResGetDarkFrameInfoList res;
            if (res.ParseFromArray(data.data(), data.size())) {
                QList<QVariantMap> frames;
                for (const auto &info : res.results()) {
                    QVariantMap frame;
                    frame["exp_index"] = info.exp_index();
                    frame["gain_index"] = info.gain_index();
                    frame["bin_index"] = info.bin_index();
                    frames.append(frame);
                }
                emit darkFrameListReceived(frames);
            }
            break;
        }
        
        case AstroCmd::CAPTURE_RAW_LIVE_STACKING: {
            // Response to stacking start command
            qWarning() << "=== STACKING START response received, data size:" << data.size();
            if (data.size() > 0) {
                // Parse as ComResponse to check error code
                dwarf::ComResponse res;
                if (res.ParseFromArray(data.data(), data.size())) {
                    qWarning() << "    Response code:" << res.code();
                    if (res.code() != 0) {
                        QString errorMsg;
                        switch (res.code()) {
                            case -11513:
                                errorMsg = "GOTO required! Please use GOTO to a target first, then start stacking.";
                                break;
                            case -11514:
                                errorMsg = "Parameters not suitable! Check exposure/gain settings.";
                                break;
                            case -11503:
                                errorMsg = "Dark frame not found! Capture dark frames first.";
                                break;
                            default:
                                errorMsg = QString("Error code: %1").arg(res.code());
                                break;
                        }
                        qCritical() << "✗ Stacking start FAILED:" << errorMsg;
                        emit stackingFailed(errorMsg);
                    } else {
                        qWarning() << "    ✓ Stacking started successfully";
                    }
                }
            }
            break;
        }
        
        case AstroCmd::STOP_CAPTURE_RAW_LIVE_STACKING: {
            qDebug() << "Stacking stop response received";
            break;
        }
        
        case AstroCmd::GO_LIVE: {
            qWarning() << "=== GO LIVE response received, data size:" << data.size();
            if (data.size() > 0) {
                dwarf::ComResponse res;
                if (res.ParseFromArray(data.data(), data.size())) {
                    qWarning() << "    Go Live response code:" << res.code();
                    if (res.code() != 0) {
                        qWarning() << "    ✗ Go Live FAILED with code:" << res.code();
                    } else {
                        qWarning() << "    ✓ Go Live activated successfully";
                    }
                }
            } else {
                qWarning() << "    ✓ Go Live command acknowledged (empty response)";
            }
            break;
        }
        
        default:
            qDebug() << "Unhandled astro command:" << cmd;
            break;
    }
}

// ============================================================================
// Notification Handler (Module 9)
// ============================================================================

namespace NotifyCmd {
    constexpr quint32 BATTERY = 15201;
    constexpr quint32 CHARGE_STATUS = 15202;
    constexpr quint32 SD_CARD_INFO = 15203;
    constexpr quint32 DARK_OPERATION_STATE = 15206;
    constexpr quint32 DARK_PROGRESS = 15207;
    constexpr quint32 STACKING_STATE = 15208;
    constexpr quint32 STACKING_PROGRESS = 15209;
    constexpr quint32 CALIBRATION_STATE = 15210;
    constexpr quint32 GOTO_STATE = 15211;
    constexpr quint32 TRACKING_STATE = 15212;
    constexpr quint32 ONE_CLICK_GOTO_STATE = 15233;
    constexpr quint32 WIDE_STACKING_STATE = 15236;
    constexpr quint32 WIDE_STACKING_PROGRESS = 15237;
    constexpr quint32 EQ_SOLVING_STATE = 15239;
    constexpr quint32 STOP_CAPTURE_WIDE_RAW_LIVE_STACKING = 11017;
    constexpr quint32 TEMPERATURE = 15243;
}

namespace {
bool readVarintU64(const QByteArray &buf, int *offset, quint64 *out) {
    if (!offset || !out)
        return false;
    quint64 value = 0;
    int shift = 0;
    int i = *offset;
    while (i < buf.size()) {
        const quint8 b = static_cast<quint8>(buf.at(i));
        i += 1;
        value |= (static_cast<quint64>(b & 0x7F) << shift);
        if ((b & 0x80) == 0) {
            *offset = i;
            *out = value;
            return true;
        }
        shift += 7;
        if (shift > 63)
            break;
    }
    return false;
}

void appendVarintU64(QByteArray &buf, quint64 value) {
    while (value >= 0x80) {
        buf.append(static_cast<char>((value & 0x7F) | 0x80));
        value >>= 7;
    }
    buf.append(static_cast<char>(value));
}

void appendInt64Varint(QByteArray &buf, qint64 value) {
    const quint64 v = static_cast<quint64>(value);
    appendVarintU64(buf, v);
}

QHash<quint32, QVector<quint64>> parseVarintFields(const QByteArray &buf) {
    QHash<quint32, QVector<quint64>> fields;
    int offset = 0;
    while (offset < buf.size()) {
        quint64 key = 0;
        if (!readVarintU64(buf, &offset, &key))
            break;

        const quint32 fieldNum = static_cast<quint32>(key >> 3);
        const quint32 wireType = static_cast<quint32>(key & 0x07);

        if (wireType == 0) {
            quint64 v = 0;
            if (!readVarintU64(buf, &offset, &v))
                break;
            fields[fieldNum].append(v);
        } else if (wireType == 2) {
            quint64 len = 0;
            if (!readVarintU64(buf, &offset, &len))
                break;
            if (len > static_cast<quint64>(buf.size() - offset))
                break;
            offset += static_cast<int>(len);
        } else {
            break;
        }
    }
    return fields;
}

int firstFieldOr(const QHash<quint32, QVector<quint64>> &fields, quint32 fieldNum,
                 int defaultValue) {
    const auto it = fields.find(fieldNum);
    if (it == fields.end() || it->isEmpty())
        return defaultValue;
    return static_cast<int>(it->at(0));
}
} // namespace

void DwarfAstroController::handleNotification(quint32 cmd, const QByteArray &data) {
    qWarning() << "[AstroController::handleNotification] Cmd:" << cmd << "Size:" << data.size();
    switch (cmd) {
        case NotifyCmd::BATTERY: {
            dwarf::ResNotifyBattery res;
            if (res.ParseFromArray(data.data(), data.size())) {
                emit batteryChanged(res.value());
            }
            break;
        }

        case NotifyCmd::SD_CARD_INFO: {
            dwarf::ResNotifySDcardInfo res;
            if (res.ParseFromArray(data.data(), data.size())) {
                // Values are in MB
                emit sdCardInfoReceived(res.total() / 1024.0f, res.available() / 1024.0f);
            }
            break;
        }
        
        case NotifyCmd::STACKING_STATE: {
            qDebug() << "=== STACKING_STATE notification, data size:" << data.size();
            if (data.size() == 0) {
                qWarning() << "Empty stacking state notification - device may not be ready";
                break;
            }
            dwarf::ResNotifyOperationState res;
            if (res.ParseFromArray(data.data(), data.size())) {
                qDebug() << "Stacking state:" << res.state();
                emit stackingStateChanged(res.state());
            } else {
                qWarning() << "Failed to parse stacking state notification";
            }
            break;
        }
        
        case NotifyCmd::STACKING_PROGRESS: {
            qDebug() << "=== STACKING_PROGRESS notification, data size:" << data.size();
            
            if (data.size() == 0) {
                qWarning() << "Empty stacking progress notification";
                break;
            }

            const auto fields = parseVarintFields(data);
            const int totalCount = firstFieldOr(fields, 1, 0);
            const int updateCountType = firstFieldOr(fields, 2, -1);
            const int currentCount = firstFieldOr(fields, 3, 0);
            const int stackedCountRaw = firstFieldOr(fields, 4, 0);
            const int expIndex = firstFieldOr(fields, 5, -1);
            const int gainIndex = firstFieldOr(fields, 6, -1);

            int stackedCount = stackedCountRaw;
            if (stackedCount <= 0)
                stackedCount = currentCount;

            qDebug() << "✓ Stacking progress: current=" << currentCount
                     << "total=" << totalCount << "stacked=" << stackedCount
                     << "updateType=" << updateCountType << "expIndex=" << expIndex
                     << "gainIndex=" << gainIndex;

            emit stackingProgress(currentCount, totalCount, stackedCount, 0);
            break;
        }

        case NotifyCmd::WIDE_STACKING_PROGRESS: {
            if (data.size() == 0)
                break;
            const auto fields = parseVarintFields(data);
            const int totalCount = firstFieldOr(fields, 1, 0);
            const int currentCount = firstFieldOr(fields, 3, 0);
            const int stackedCountRaw = firstFieldOr(fields, 4, 0);
            int stackedCount = stackedCountRaw;
            if (stackedCount <= 0)
                stackedCount = currentCount;
            emit stackingProgress(currentCount, totalCount, stackedCount, 0);
            break;
        }
        
        case NotifyCmd::CALIBRATION_STATE: {
            dwarf::ResNotifyStateAstroCalibration res;
            if (res.ParseFromArray(data.data(), data.size())) {
                qDebug() << "Calibration state:" << res.state() << "code:" << res.code();
                switch (res.state()) {
                    case 1: emit calibrationStarted(); break;
                    case 2: emit calibrationCompleted(true); break;
                    case 3: emit calibrationFailed(QString("Error code: %1").arg(res.code())); break;
                }
            }
            break;
        }
        
        case NotifyCmd::ONE_CLICK_GOTO_STATE: {
            dwarf::ResNotifyOneClickGotoState res;
            if (res.ParseFromArray(data.data(), data.size())) {
                qDebug() << "One-click GOTO: step" << res.step() << "state" << res.state();
                emit gotoProgress(res.step());
                if (res.state() == 1) {
                    emit gotoCompleted();
                } else if (res.state() == 2) {
                    emit gotoFailed(QString("Error code: %1").arg(res.code()));
                }
            }
            break;
        }
        
        case NotifyCmd::DARK_PROGRESS: {
            dwarf::ResNotifyProgressCaptureRawDark res;
            if (res.ParseFromArray(data.data(), data.size())) {
                emit darkFrameProgress(res.current_count(), res.total_count());
            }
            break;
        }
        
        case NotifyCmd::TEMPERATURE: {
            dwarf::ResNotifyTemperature res;
            if (res.ParseFromArray(data.data(), data.size())) {
                emit temperatureChanged(res.temperature());
            }
            break;
        }
        
        case NotifyCmd::EQ_SOLVING_STATE: {
            dwarf::ResNotifyEqSolvingState res;
            if (res.ParseFromArray(data.data(), data.size())) {
                emit eqSolvingResult(res.azi_err(), res.alt_err());
            }
            break;
        }
        
        default:
            // Many notifications we don't need to handle
            break;
    }
}
