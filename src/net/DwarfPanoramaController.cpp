#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "base.pb.h"
#include "panorama.pb.h"

#include <QDebug>

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
    m_lastRows = rows;
    m_lastCols = cols;

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
    qWarning() << "[DwarfPanoramaController] stopPanorama payloadSize=" << payload.size();
    sendCommand(PanoramaCmd::STOP, payload);
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
            emit panoramaStarted(m_lastRows, m_lastCols);
            break;
        case PanoramaCmd::STOP:
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
            
            qWarning() << "[PanoramaController] Progress:" << completed << "/" << total;
            emit panoramaProgress(completed, total);
            
            // Auto-emit stop when completed
            if (completed >= total && total > 0) {
                qWarning() << "[PanoramaController] Panorama completed!";
                emit panoramaStopped();
            }
        } else {
            qWarning() << "[PanoramaController] Failed to parse progress notification, size:" << data.size();
            qWarning() << "[PanoramaController] Data hex:" << data.toHex();
        }
    }
}
