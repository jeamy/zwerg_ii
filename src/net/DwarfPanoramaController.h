#pragma once

#include <QObject>

class DwarfWebSocketClient;

class DwarfPanoramaController : public QObject {
    Q_OBJECT

public:
    explicit DwarfPanoramaController(QObject *parent = nullptr);

    void setClient(DwarfWebSocketClient *client);

    void startPanoramaGrid(int rows, int cols);
    void stopPanorama();

    void handlePanoramaMessage(quint32 cmd, const QByteArray &data);
    void handleNotification(quint32 cmd, const QByteArray &data);

signals:
    void panoramaStarted(int rows, int cols);
    void panoramaProgress(int completed, int total);
    void panoramaStopped();
    void panoramaFailed(const QString &error);

private:
    void sendCommand(quint32 cmd, const QByteArray &data);

    DwarfWebSocketClient *m_client = nullptr;
    int m_lastRows = 0;
    int m_lastCols = 0;
    bool m_isRunning = false;
    bool m_justCompleted = false;  // Prevent restart from late response
};
