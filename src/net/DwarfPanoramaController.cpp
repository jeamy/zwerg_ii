#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "../proto/base.pb.h"
#include "../proto/camera.pb.h"
#include "../proto/panorama.pb.h"

#include <QDebug>
#include <QThread>
#include <cmath>

namespace PanoramaCmd {
    constexpr quint32 START_GRID = 15500;
    constexpr quint32 STOP = 15501;
    constexpr quint32 NOTIFY_PROGRESS = 15219;  // Progress notification
    
    // Module 20 commands (from Android PCAP reverse engineering)
    constexpr quint32 UI_OPEN = 16402;  // Panorama UI Open
    constexpr quint32 GRID_PARAM = 16703;  // Set grid parameters (row/col)
}

void DwarfPanoramaController::handlePanoramaUiMessage(quint32 cmd, const QByteArray &data) {
    qWarning() << "[PanoramaController::handlePanoramaUiMessage] Cmd:" << cmd
               << "Size:" << data.size() << "hex:" << data.toHex();
}

namespace CameraCmd {
    constexpr quint32 SET_FEATURE_PARAM = 10037;  // CMD_CAMERA_TELE_SET_FEATURE_PARAM
    constexpr quint32 GET_ALL_FEATURE_PARAMS = 10038;
    constexpr quint32 WIDE_SET_FEATURE_PARAM = 12037;
    constexpr quint32 WIDE_GET_ALL_FEATURE_PARAMS = 12038;
}

namespace PanoramaFeatureCmd {
    constexpr quint32 SET_GRID_PARAM = 16703; // observed: module 15, cmd 16703
    constexpr quint32 MODULE_ID = 15;
}

namespace PanoramaUiCmd {
    constexpr quint32 MODULE_ID = 14;
    constexpr quint32 OPEN = 16402; // observed Android app pano-open request
}

DwarfPanoramaController::DwarfPanoramaController(QObject *parent)
    : QObject(parent) {
}

static int toGridIndexValue(int count) {
    if (count <= 0)
        return 0;
    // TODO: Verify the exact DWARF firmware mapping for panorama grid row/col.
    // Current implementation mirrors the previously captured pattern used by our sendFeatureParam.
    return count * 2 - 1;
}

static void fillLegacyPanoFeatureParam(dwarf::ReqSetFeatureParams &req, int id, int count) {
    dwarf::CommonParam *param = req.mutable_param();
    param->set_has_auto(false);
    param->set_auto_mode(1);
    param->set_id(id);
    param->set_mode_index(1);
    param->set_index(0);
    param->set_continue_value(static_cast<double>(count));
}

void DwarfPanoramaController::setClient(DwarfWebSocketClient *client) {
    m_client = client;
}

void DwarfPanoramaController::sendPanoramaUiOpen() {
    if (!m_client) {
        qWarning() << "[DwarfPanoramaController] Cannot send pano UI open: no client";
        return;
    }
    // Android app sends module=14 cmd=16402 with payload 08 07 when opening pano UI.
    const QByteArray payload = QByteArray::fromHex("0807");
    qWarning() << "[DwarfPanoramaController] Sending pano UI open handshake (module=14 cmd=16402)";
    m_client->sendCommand(PanoramaUiCmd::MODULE_ID, PanoramaUiCmd::OPEN, payload);
}

void DwarfPanoramaController::sendCommand(quint32 cmd, const QByteArray &data) {
    if (!m_client) {
        qWarning() << "PanoramaController: No client set";
        return;
    }

    qWarning() << "[DwarfPanoramaController] sendCommand module=10 cmd=" << cmd
               << "payloadSize=" << data.size();
    m_client->sendCommand(10, cmd, data);
}

void DwarfPanoramaController::sendCommandModule(quint32 moduleId, quint32 cmd, const QByteArray &data) {
    if (!m_client) {
        qWarning() << "PanoramaController: No client set";
        return;
    }

    qWarning() << "[DwarfPanoramaController] sendCommand module=" << moduleId << "cmd=" << cmd
               << "payloadSize=" << data.size() << "hex=" << data.toHex();
    m_client->sendCommand(moduleId, cmd, data);
}

QByteArray DwarfPanoramaController::buildGridCommand(quint8 selector, int value) {
    // Build the inner `data` payload for module 15 / cmd 16703.
    // IMPORTANT: DwarfWebSocketClient wraps this data into WsPacket.
    // Android PCAP shows this data payload is exactly 12 bytes:
    // 08 <selector-varint(9 bytes)> 10 <value>
    QByteArray payload;
    payload.reserve(12);
    payload.append(static_cast<char>(0x08));  // field 1 tag
    payload.append(static_cast<char>(selector));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(0x80));
    payload.append(static_cast<char>(0xbc));
    payload.append(static_cast<char>(0x81));
    payload.append(static_cast<char>(0x07));
    payload.append(static_cast<char>(0x10));  // field 2 tag
    payload.append(static_cast<char>(value));

    qWarning() << "[DwarfPanoramaController] buildGridCommand selector=0x" << QString::number(selector, 16)
               << "value=" << value << "payloadSize=" << payload.size()
               << "hex=" << payload.toHex();
    
    return payload;
}

void DwarfPanoramaController::setPanoramaGrid(int rows, int cols) {
    // Set panorama grid parameters using reverse-engineered Android protocol
    // Based on PCAP analysis from ctrl_20251226_113958.pcapng
    
    qWarning() << "[DwarfPanoramaController] setPanoramaGrid rows=" << rows << "cols=" << cols;
    
    // Step 1: Send Panorama UI Open (Module 14, CMD 16402)
    // Android sends this before setting grid parameters and waits for 4 responses
    qWarning() << "[DwarfPanoramaController] Sending Panorama UI Open...";
    QByteArray uiOpenPayload = QByteArray::fromHex("0807");
    sendCommandModule(14, PanoramaCmd::UI_OPEN, uiOpenPayload);
    
    // Android waits for 4 responses here - we use longer delay
    QThread::msleep(200);
    
    // Step 2: Set ROW parameter (Module 15, CMD 16703)
    // Selector: 0x9c (row selector from PCAP)
    // Android waits for 2 responses after this
    qWarning() << "[DwarfPanoramaController] Setting ROW=" << rows;
    QByteArray rowPayload = buildGridCommand(0x9c, rows);
    sendCommandModule(15, PanoramaCmd::GRID_PARAM, rowPayload);
    
    // Android waits for 2 responses here
    QThread::msleep(200);
    
    // Step 3: Set COL parameter (Module 15, CMD 16703)
    // Selector: 0x9d (col selector from PCAP)
    // Android waits for 2 responses after this
    qWarning() << "[DwarfPanoramaController] Setting COL=" << cols;
    QByteArray colPayload = buildGridCommand(0x9d, cols);
    sendCommandModule(15, PanoramaCmd::GRID_PARAM, colPayload);
    
    // Final delay to let DWARF process the COL setting
    QThread::msleep(200);
    
    qWarning() << "[DwarfPanoramaController] Grid parameters set successfully";
}

void DwarfPanoramaController::startPanoramaGrid(int rows, int cols) {
    // Grid parameters should already be set by spinner valueChanged signals
    // Just send START command (Android app behavior)
    
    requested_rows = rows;
    requested_cols = cols;
    expected_tiles = rows * cols;
    estimated_completed_tiles = 0;
    pano_running = true;

    m_lastRows = rows;
    m_lastCols = cols;
    m_justCompleted = false;
    m_lastProgressCompleted = 0;
    m_loggedProgressHexThisRun = false;

    // Based on PCAP: Android sends empty START command
    // Grid params were already sent when user changed spinners
    // Module 10, CMD 15500, no payload
    const QByteArray emptyPayload;
    
    qWarning() << "[DwarfPanoramaController] Starting panorama";
    qWarning() << "[DwarfPanoramaController] Expected grid: rows=" << rows << "cols=" << cols;
    qWarning() << "[DwarfPanoramaController] State before START: m_isRunning=" << m_isRunning
               << "pano_running=" << pano_running << "m_justCompleted=" << m_justCompleted;
    
    sendCommand(PanoramaCmd::START_GRID, emptyPayload);
}

void DwarfPanoramaController::stopPanorama() {
    qWarning() << "[DwarfPanoramaController] stopPanorama() called";
    qWarning() << "[DwarfPanoramaController] State before STOP: m_isRunning=" << m_isRunning
               << "pano_running=" << pano_running << "m_justCompleted=" << m_justCompleted
               << "m_lastRows=" << m_lastRows << "m_lastCols=" << m_lastCols
               << "m_lastProgressCompleted=" << m_lastProgressCompleted;
    
    // Based on Android PCAP: STOP command has EMPTY payload (same as START)
    // Android sends: Module 10, CMD 15501, no data payload
    const QByteArray emptyPayload;
    qWarning() << "[DwarfPanoramaController] Sending STOP command (empty payload like Android)";
    
    // Don't reset pano_running here - let it be cleared when STOP response arrives
    // Otherwise we ignore progress updates after sending stop
    sendCommand(PanoramaCmd::STOP, emptyPayload);
    
    qWarning() << "[DwarfPanoramaController] STOP command sent";
}

void DwarfPanoramaController::handleNotificationProgress(int total_count, int completed_count) {
    if (!pano_running)
        return;

    // Report RAW firmware counters. On some firmwares total_count reflects the real
    // number of photos (e.g. 30 when the preset is 6x5). Do not remap.
    qWarning() << "[PanoramaController] Progress (raw):" << completed_count << "/" << total_count
               << "requested rows/cols=" << m_lastRows << "x" << m_lastCols
               << "(requested tiles=" << (m_lastRows * m_lastCols) << ")";

    emit panoramaProgress(completed_count, total_count);

    if (total_count > 0 && completed_count >= total_count) {
        qWarning() << "[PanoramaController] Panorama completed (raw progress reached total_count)";
        pano_running = false;
        m_isRunning = false;
        m_justCompleted = true;
        emit panoramaStopped();
    }
}

void DwarfPanoramaController::handlePanoramaMessage(quint32 cmd, const QByteArray &data) {
    dwarf::ComResponse res;
    if (data.size() > 0 && res.ParseFromArray(data.data(), data.size())) {
        if (res.code() != 0) {
            emit panoramaFailed(QStringLiteral("Panorama error code: %1").arg(res.code()));
            return;
        }
    }

    switch (cmd) {
        case PanoramaCmd::START_GRID:
            // DWARF sends cmd 15500 again (Type:3) when panorama finishes.
            // We don't get the WsPacket type here, so we use robust heuristics:
            // - if we're running, it's completion
            // - if we have seen any progress notifications, it's completion
            if (m_isRunning || m_lastProgressCompleted > 0) {
                qWarning() << "[PanoramaController] Received START_GRID while running - treating as completion";
                m_isRunning = false;
                m_justCompleted = true;
                m_lastProgressCompleted = 0;
                emit panoramaStopped();
            } else if (!m_justCompleted) {
                // Normal start: only emit if not just completed
                m_isRunning = true;
                qWarning() << "[PanoramaController] Starting panorama" << m_lastRows << "x" << m_lastCols;
                emit panoramaStarted(m_lastRows, m_lastCols);
            } else {
                qWarning() << "[PanoramaController] Ignoring late START_GRID response (just completed)";
                m_justCompleted = false;
            }
            break;
        case PanoramaCmd::STOP:
            m_isRunning = false;
            pano_running = false;
            qWarning() << "[PanoramaController] Received STOP response - panorama stopped";
            emit panoramaStopped();
            break;
        default:
            break;
    }
}

void DwarfPanoramaController::handleNotification(quint32 cmd, const QByteArray &data) {
    qWarning() << "[PanoramaController::handleNotification] Cmd:" << cmd << "Size:" << data.size();

    if (cmd == PanoramaCmd::NOTIFY_PROGRESS) {
        qWarning() << "[PanoramaController] Got NOTIFY_PROGRESS (15219)";
        dwarf::ResNotifyPanoramaProgress progress;
        if (data.size() > 0 && progress.ParseFromArray(data.data(), data.size())) {
            int completed = progress.completed_count();
            int total = progress.total_count();

            if (!m_loggedProgressHexThisRun) {
                m_loggedProgressHexThisRun = true;
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS raw hex:" << data.toHex();
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS decoded total_count=" << total
                           << "completed_count=" << completed
                           << "requested rows/cols=" << m_lastRows << "x" << m_lastCols;
            }

            m_lastProgressCompleted = completed;
            handleNotificationProgress(total, completed);
        } else {
            qWarning() << "[PanoramaController] Failed to parse progress notification, size:" << data.size();
            qWarning() << "[PanoramaController] Data hex:" << data.toHex();
        }
    } else {
        if (!data.isEmpty()) {
            auto readVarint = [](const QByteArray &buf, int &i, quint64 &out, QByteArray *raw) {
                out = 0;
                int shift = 0;
                if (raw)
                    raw->clear();
                while (i < buf.size() && shift < 64) {
                    const quint8 byte = static_cast<quint8>(buf.at(i++));
                    if (raw)
                        raw->append(static_cast<char>(byte));
                    out |= static_cast<quint64>(byte & 0x7F) << shift;
                    if ((byte & 0x80) == 0)
                        return true;
                    shift += 7;
                }
                return false;
            };

            int i = 0;
            quint64 selector = 0;
            quint64 value = 0;
            quint64 value2 = 0;
            bool haveSelector = false;
            bool haveValue = false;
            bool haveValue2 = false;

            QByteArray rawSelector;
            QByteArray rawValue;
            QByteArray rawValue2;

            while (i < data.size()) {
                quint64 tag = 0;
                if (!readVarint(data, i, tag, nullptr))
                    break;
                const quint32 field = static_cast<quint32>(tag >> 3);
                const quint32 wire = static_cast<quint32>(tag & 0x7);
                if (wire != 0) {
                    break;
                }
                quint64 v = 0;
                QByteArray rawV;
                if (!readVarint(data, i, v, &rawV))
                    break;
                if (field == 1) {
                    selector = v;
                    haveSelector = true;
                    rawSelector = rawV;
                } else if (field == 2) {
                    value = v;
                    haveValue = true;
                    rawValue = rawV;
                } else if (field == 3) {
                    value2 = v;
                    haveValue2 = true;
                    rawValue2 = rawV;
                }
            }

            if (haveSelector || haveValue || haveValue2) {
                qWarning() << "[PanoramaController] Notification decoded selector=" << selector
                           << "value=" << value << "value2=" << value2
                           << "raw1=" << rawSelector.toHex()
                           << "raw2=" << rawValue.toHex()
                           << "raw3=" << rawValue2.toHex()
                           << "hex=" << data.toHex();
            } else {
                qWarning() << "[PanoramaController] Notification hex:" << data.toHex();
            }
        }
    }
}
