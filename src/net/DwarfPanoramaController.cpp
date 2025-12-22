#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "base.pb.h"
#include "panorama.pb.h"

#include <QDebug>
#include <cmath>

namespace PanoramaCmd {
    constexpr quint32 START_GRID = 15500;
    constexpr quint32 STOP = 15501;
    constexpr quint32 NOTIFY_PROGRESS = 15219;  // Progress notification
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

void DwarfPanoramaController::startPanoramaGrid(int rows, int cols) {
    requested_rows = rows;
    requested_cols = cols;
    expected_tiles = rows * cols;
    estimated_completed_tiles = 0;
    pano_running = true;

    m_lastRows = rows;
    m_lastCols = cols;
    m_justCompleted = false;  // Reset for new panorama
    m_isRunning = true;
    m_lastProgressCompleted = 0;
    m_loggedProgressHexThisRun = false;

    dwarf::ReqStartPanoramaByGrid req;
    req.set_rows(static_cast<std::uint32_t>(rows));
    req.set_cols(static_cast<std::uint32_t>(cols));

    const QByteArray payload = QByteArray::fromStdString(req.SerializeAsString());
    qWarning() << "[DwarfPanoramaController] startPanoramaGrid rows=" << rows
               << "cols=" << cols << "payloadSize=" << payload.size();
    sendCommand(PanoramaCmd::START_GRID, payload);
}

void DwarfPanoramaController::stopPanorama() {
    dwarf::ReqStopPanorama req;
    const QByteArray payload = QByteArray::fromStdString(req.SerializeAsString());
    qWarning() << "[DwarfPanoramaController] stopPanorama (manual) payloadSize=" << payload.size();
    m_isRunning = false;  // Reset immediately on manual stop
    pano_running = false;
    sendCommand(PanoramaCmd::STOP, payload);
}

void DwarfPanoramaController::handleNotificationProgress(int total_count, int completed_count) {
    if (!pano_running || expected_tiles <= 0)
        return;

    constexpr float firmware_progress_max = 30.0f;
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
            qWarning() << "[PanoramaController] Stopping panorama";
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
            int dwarfTotal = progress.total_count();  // DWARF's reported total
            int expected = m_lastRows * m_lastCols;
            if (!m_loggedProgressHexThisRun) {
                m_loggedProgressHexThisRun = true;
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS raw hex:" << data.toHex();
                qWarning() << "[PanoramaController] NOTIFY_PROGRESS decoded total_count=" << dwarfTotal
                           << "completed_count=" << completed
                           << "requested rows/cols=" << m_lastRows << "x" << m_lastCols;
            }

            m_lastProgressCompleted = completed;

            if (dwarfTotal != expected && expected > 0 && dwarfTotal > 0) {
                qWarning() << "[PanoramaController] DWARF reported total=" << dwarfTotal
                           << "but requested grid was" << expected << "(" << m_lastRows << "x" << m_lastCols << ")";
            }

            handleNotificationProgress(dwarfTotal, completed);
            
            // Note: Don't auto-stop here - DWARF will send completion via cmd 15500
            // We just track progress and let the device signal completion
        } else {
            qWarning() << "[PanoramaController] Failed to parse progress notification, size:" << data.size();
            qWarning() << "[PanoramaController] Data hex:" << data.toHex();
        }
    }
}
