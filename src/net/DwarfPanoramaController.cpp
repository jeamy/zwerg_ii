#include "DwarfPanoramaController.h"

#include "DwarfWebSocketClient.h"
#include "base.pb.h"
#include "panorama.pb.h"

#include <QDebug>

namespace PanoramaCmd {
    constexpr quint32 START_GRID = 15500;
    constexpr quint32 STOP = 15501;
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
    m_client->sendCommand(10, cmd, data);
}

void DwarfPanoramaController::startPanoramaGrid(int rows, int cols) {
    m_lastRows = rows;
    m_lastCols = cols;

    dwarf::ReqStartPanoramaByGrid req;
    req.set_rows(static_cast<std::uint32_t>(rows));
    req.set_cols(static_cast<std::uint32_t>(cols));

    sendCommand(PanoramaCmd::START_GRID, QByteArray::fromStdString(req.SerializeAsString()));
}

void DwarfPanoramaController::stopPanorama() {
    dwarf::ReqStopPanorama req;
    sendCommand(PanoramaCmd::STOP, QByteArray::fromStdString(req.SerializeAsString()));
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
