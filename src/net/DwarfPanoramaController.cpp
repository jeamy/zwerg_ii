#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "../proto/base.pb.h"
#include "../proto/camera.pb.h"
#include "../proto/panorama.pb.h"

#include <QDebug>

namespace PanoramaCmd {
    constexpr quint32 START_GRID = 15500;
    constexpr quint32 STOP = 15501;
    constexpr quint32 NOTIFY_PROGRESS = 15219;  // Progress notification
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

void DwarfPanoramaController::setPanoramaRows(int rows) {
    requested_rows = rows;
    qWarning() << "[DwarfPanoramaController] setPanoramaRows rows=" << rows;
    sendFeatureParam(6, toGridIndexValue(rows));

    if (m_client) {
        dwarf::ReqSetFeatureParams req;
        fillLegacyPanoFeatureParam(req, 6, rows);
        const QByteArray payload = QByteArray::fromStdString(req.SerializeAsString());
        // Panorama uses the WIDE camera. Send to both modules for compatibility.
        m_client->sendCommand(2, CameraCmd::WIDE_SET_FEATURE_PARAM, payload);
        m_client->sendCommand(2, CameraCmd::SET_FEATURE_PARAM, payload);
        m_client->sendCommand(1, CameraCmd::SET_FEATURE_PARAM, payload);

        // Ask the device to echo current feature params so we can see whether it applied.
        m_client->sendCommand(2, CameraCmd::WIDE_GET_ALL_FEATURE_PARAMS, QByteArray());
        m_client->sendCommand(2, CameraCmd::GET_ALL_FEATURE_PARAMS, QByteArray());
    }
}

void DwarfPanoramaController::setPanoramaCols(int cols) {
    requested_cols = cols;
    qWarning() << "[DwarfPanoramaController] setPanoramaCols cols=" << cols;
    sendFeatureParam(7, toGridIndexValue(cols));

    if (m_client) {
        dwarf::ReqSetFeatureParams req;
        fillLegacyPanoFeatureParam(req, 7, cols);
        const QByteArray payload = QByteArray::fromStdString(req.SerializeAsString());
        // Panorama uses the WIDE camera. Send to both modules for compatibility.
        m_client->sendCommand(2, CameraCmd::WIDE_SET_FEATURE_PARAM, payload);
        m_client->sendCommand(2, CameraCmd::SET_FEATURE_PARAM, payload);
        m_client->sendCommand(1, CameraCmd::SET_FEATURE_PARAM, payload);

        // Ask the device to echo current feature params so we can see whether it applied.
        m_client->sendCommand(2, CameraCmd::WIDE_GET_ALL_FEATURE_PARAMS, QByteArray());
        m_client->sendCommand(2, CameraCmd::GET_ALL_FEATURE_PARAMS, QByteArray());
    }
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

void DwarfPanoramaController::sendFeatureParam(int id, int value) {
    if (!m_client) {
        qWarning() << "[DwarfPanoramaController] No client for sendFeatureParam";
        return;
    }

    // Captures show rows/cols are set via a compact 2-field protobuf payload sent as:
    // WsPacket{major=1,minor=20,device_id=1,module_id=15,cmd=16703,data=<payload>,client_id=<uuid>}
    // The inner payload is:
    //   field1(varint) = selector (row/col)
    //   field2(varint) = value/index
    // Where selector differs only in the lowest byte:
    //   row: ...9c...
    //   col: ...9d...
    static const QByteArray kSelectorRow = QByteArray::fromHex("9c80808080bc8107");
    static const QByteArray kSelectorCol = QByteArray::fromHex("9d80808080bc8107");

    const QByteArray selector = (id == 6) ? kSelectorRow : (id == 7 ? kSelectorCol : QByteArray());
    if (selector.isEmpty()) {
        qWarning() << "[DwarfPanoramaController] Unsupported feature id for pano grid:" << id;
        return;
    }

    auto encodeVarint = [](quint64 v) {
        QByteArray out;
        while (v >= 0x80) {
            out.append(static_cast<char>((v & 0x7F) | 0x80));
            v >>= 7;
        }
        out.append(static_cast<char>(v & 0x7F));
        return out;
    };

    QByteArray payload;
    payload.append(static_cast<char>(0x08));
    payload.append(selector);
    payload.append(static_cast<char>(0x10));
    payload.append(encodeVarint(static_cast<quint64>(value)));

    qWarning() << "[DwarfPanoramaController] sendFeatureParam(grid) id=" << id
               << "value=" << value << "payloadHex=" << payload.toHex();

    m_client->sendCommand(PanoramaFeatureCmd::MODULE_ID, PanoramaFeatureCmd::SET_GRID_PARAM, payload);
}

void DwarfPanoramaController::startPanoramaGrid(int rows, int cols) {
    requested_rows = rows;
    requested_cols = cols;
    expected_tiles = rows * cols;
    estimated_completed_tiles = 0;
    pano_running = true;

    m_lastRows = rows;
    m_lastCols = cols;
    m_justCompleted = false;  // Reset for new panorama
    // Don't set m_isRunning here - let START_GRID response set it
    // Otherwise the response is treated as completion and panoramaStarted is never emitted
    m_lastProgressCompleted = 0;
    m_loggedProgressHexThisRun = false;

    qWarning() << "[DwarfPanoramaController] Starting panorama grid via feature params + START_GRID: rows="
               << rows << "cols=" << cols;

    // dwarfium apiV2 sends the three packets as a batch without waiting for ACKs.
    // Mirror that here: set row/col feature params and immediately start grid.
    // Captures indicate the transmitted value is an index (offset 25), not the raw count.
    // Observed pattern fits: index = count*2 - 1 (e.g. 3->5, 4->7, 5->9, 6->0b).
    sendFeatureParam(6, toGridIndexValue(rows));
    sendFeatureParam(7, toGridIndexValue(cols));
    sendStartGrid();
}

void DwarfPanoramaController::sendStartGrid() {
    // Captures show START_GRID is sent as a WsPacket without additional data payload.
    // Rows/cols are applied via the feature-param commands above.
    qWarning() << "[DwarfPanoramaController] Sending START_GRID (no body) rows=" << m_lastRows
               << "cols=" << m_lastCols;
    sendCommand(PanoramaCmd::START_GRID, QByteArray());
}

void DwarfPanoramaController::stopPanorama() {
    qWarning() << "[DwarfPanoramaController] stopPanorama() called";
    qWarning() << "[DwarfPanoramaController] State before STOP: m_isRunning=" << m_isRunning
               << "pano_running=" << pano_running << "m_justCompleted=" << m_justCompleted
               << "m_lastRows=" << m_lastRows << "m_lastCols=" << m_lastCols
               << "m_lastProgressCompleted=" << m_lastProgressCompleted;

    // Captures show STOP is sent as a WsPacket without additional data payload.
    qWarning() << "[DwarfPanoramaController] Sending STOP (no body)";
    sendCommand(PanoramaCmd::STOP, QByteArray());

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
