#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "base.pb.h"
#include "panorama.pb.h"

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

DwarfPanoramaController::DwarfPanoramaController(QObject *parent)
    : QObject(parent) {
}

void DwarfPanoramaController::setClient(DwarfWebSocketClient *client) {
    m_client = client;
}

void DwarfPanoramaController::sendCommand(quint32 cmd, const QByteArray &data) {
    if (!m_client) {
        qWarning() << "PanoramaController: No client set";
        return;
    }

    // Module 10 = Panorama
    qWarning() << "[DwarfPanoramaController] sendCommand module=10 cmd=" << cmd
               << "payloadSize=" << data.size();
    m_client->sendCommand(10, cmd, data);
}

void DwarfPanoramaController::sendCommandModule20(quint32 cmd, const QByteArray &data) {
    if (!m_client) {
        qWarning() << "PanoramaController: No client set";
        return;
    }

    // Module 20 = Camera/Feature control (used for grid parameters)
    qWarning() << "[DwarfPanoramaController] sendCommand module=20 cmd=" << cmd
               << "payloadSize=" << data.size() << "hex=" << data.toHex();
    m_client->sendCommand(20, cmd, data);
}

QByteArray DwarfPanoramaController::buildGridCommand(quint8 selector, int value) {
    // Build 64-byte grid command based on Android PCAP reverse engineering
    // EXACT structure from ctrl_20251226_113958.pcapng byte-by-byte
    
    QByteArray payload;
    payload.reserve(64);
    
    // Bytes 0-14: Base template header + field markers
    const quint8 header[] = {
        0x08, 0x01, 0x10, 0x14, 0x18, 0x01, 0x20, 0x0f, 
        0x28, 0xbf, 0x82, 0x01,  // WsPacket header
        0x3a, 0x0c,              // Field tag + length
        0x08                     // Nested field start
    };
    payload.append(reinterpret_cast<const char*>(header), sizeof(header));
    
    // Byte 15: Selector (0x9c = row, 0x9d = col)
    payload.append(static_cast<char>(selector));
    
    // Bytes 16-22: Selector varint continuation (CRITICAL: FIVE 0x80 bytes!)
    // Android PCAP shows: 80 80 80 80 80 bc 81
    payload.append(static_cast<char>(0x80));  // byte 16
    payload.append(static_cast<char>(0x80));  // byte 17
    payload.append(static_cast<char>(0x80));  // byte 18
    payload.append(static_cast<char>(0x80));  // byte 19
    payload.append(static_cast<char>(0x80));  // byte 20 <- WAS MISSING!
    payload.append(static_cast<char>(0xbc));  // byte 21
    payload.append(static_cast<char>(0x81));  // byte 22
    
    // Byte 23: Field 2 tag (value field)
    payload.append(static_cast<char>(0x07));  // byte 23
    
    // Byte 24: Field 2, varint type
    payload.append(static_cast<char>(0x10));  // byte 24
    
    // Byte 25: Value (number of rows/cols)
    payload.append(static_cast<char>(value));  // byte 25
    
    // Byte 26: Field 8 tag (UUID field, length-delimited)
    payload.append(static_cast<char>(0x42));  // byte 26
    
    // Byte 27: UUID string length (36 bytes)
    payload.append(static_cast<char>(0x24));  // byte 27
    
    // Bytes 28-63: Static device UUID string (36 bytes, NO null terminator)
    const char* uuid = "2099d7b9-257a-41fc-a1ab-7e51ae2f0300";
    payload.append(uuid, 36);  // Explicitly 36 bytes, no \0
    
    // Verify exact size
    if (payload.size() != 64) {
        qWarning() << "[DwarfPanoramaController] ERROR: buildGridCommand created payload of size"
                   << payload.size() << "instead of 64 bytes!";
    }
    
    qWarning() << "[DwarfPanoramaController] buildGridCommand selector=0x" << QString::number(selector, 16)
               << "value=" << value << "payloadSize=" << payload.size()
               << "hex=" << payload.toHex();
    
    return payload;
}

void DwarfPanoramaController::setPanoramaGrid(int rows, int cols) {
    // Set panorama grid parameters using reverse-engineered Android protocol
    // Based on PCAP analysis from ctrl_20251226_113958.pcapng
    
    qWarning() << "[DwarfPanoramaController] setPanoramaGrid rows=" << rows << "cols=" << cols;
    
    // Step 1: Send Panorama UI Open (Module 20, CMD 16402)
    // Android sends this before setting grid parameters and waits for 4 responses
    qWarning() << "[DwarfPanoramaController] Sending Panorama UI Open...";
    QByteArray uiOpenPayload = QByteArray::fromHex("0807");
    sendCommandModule20(PanoramaCmd::UI_OPEN, uiOpenPayload);
    
    // Android waits for 4 responses here - we use longer delay
    QThread::msleep(200);
    
    // Step 2: Set ROW parameter (Module 20, CMD 16703)
    // Selector: 0x9c (row selector from PCAP)
    // Android waits for 2 responses after this
    qWarning() << "[DwarfPanoramaController] Setting ROW=" << rows;
    QByteArray rowPayload = buildGridCommand(0x9c, rows);
    sendCommandModule20(PanoramaCmd::GRID_PARAM, rowPayload);
    
    // Android waits for 2 responses here
    QThread::msleep(200);
    
    // Step 3: Set COL parameter (Module 20, CMD 16703)
    // Selector: 0x9d (col selector from PCAP)
    // Android waits for 2 responses after this
    qWarning() << "[DwarfPanoramaController] Setting COL=" << cols;
    QByteArray colPayload = buildGridCommand(0x9d, cols);
    sendCommandModule20(PanoramaCmd::GRID_PARAM, colPayload);
    
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
    if (!pano_running || expected_tiles <= 0)
        return;

    const float firmware_progress_max = (total_count > 0) ? static_cast<float>(total_count) : 30.0f;
    if (firmware_progress_max <= 0.0f)
        return;

    float firmware_ratio = static_cast<float>(completed_count) / firmware_progress_max;

    if (firmware_ratio < 0.0f)
        firmware_ratio = 0.0f;
    if (firmware_ratio > 1.0f)
        firmware_ratio = 1.0f;

    int mapped_tiles = static_cast<int>(std::lround(firmware_ratio * static_cast<float>(expected_tiles)));

    if (mapped_tiles < estimated_completed_tiles)
        mapped_tiles = estimated_completed_tiles;
    if (mapped_tiles > expected_tiles)
        mapped_tiles = expected_tiles;

    estimated_completed_tiles = mapped_tiles;
    emit panoramaProgress(estimated_completed_tiles, expected_tiles);

    if (estimated_completed_tiles >= expected_tiles) {
        pano_running = false;
        emit panoramaFinished();
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
            int expected = m_lastRows * m_lastCols;

            if (!m_loggedProgressHexThisRun) {
                m_loggedProgressHexThisRun = true;
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS raw hex:" << data.toHex();
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS decoded total_count=" << total
                           << "completed_count=" << completed
                           << "requested rows/cols=" << m_lastRows << "x" << m_lastCols;
            }

            // DWARF always reports total_count=30 (constant), ignore it and use expected
            if (total != expected && expected > 0) {
                qWarning() << "[PanoramaController] DWARF reported total=" << total
                           << "but expected" << expected << "(" << m_lastRows << "x" << m_lastCols << ")";
                qWarning() << "[PanoramaController] Using expected count (DWARF total is always 30)";
                total = expected;  // Override with correct value
            }
            
            qWarning() << "[PanoramaController] Progress:" << completed << "/" << total;
            emit panoramaProgress(completed, total);

            m_lastProgressCompleted = completed;
            handleNotificationProgress(total, completed);
            
            // Auto-stop when completed reaches expected tile count
            if (completed >= total && total > 0) {
                qWarning() << "[PanoramaController] Panorama completed!";
                m_isRunning = false;
                m_justCompleted = true;
                emit panoramaStopped();
            }
            
            // Note: Don't auto-stop here - DWARF will send completion via cmd 15500
            // We just track progress and let the device signal completion
        } else {
            qWarning() << "[PanoramaController] Failed to parse progress notification, size:" << data.size();
            qWarning() << "[PanoramaController] Data hex:" << data.toHex();
        }
    }
}
