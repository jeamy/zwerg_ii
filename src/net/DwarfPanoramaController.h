#pragma once

#include <QObject>

class DwarfWebSocketClient;

class DwarfPanoramaController : public QObject {
    Q_OBJECT

public:
    explicit DwarfPanoramaController(QObject *parent = nullptr);

    int requested_rows = 0;
    int requested_cols = 0;
    int expected_tiles = 0;

    int estimated_completed_tiles = 0;
    bool pano_running = false;

    void setClient(DwarfWebSocketClient *client);

    void setPanoramaGrid(int rows, int cols);

    void sendPanoramaUiOpen();

    void startPanoramaGrid(int rows, int cols);
    void stopPanorama();

public slots:
    void handlePanoramaMessage(quint32 cmd, const QByteArray &data);
    void handleNotification(quint32 cmd, const QByteArray &data);
    void handlePanoramaUiMessage(quint32 cmd, const QByteArray &data);

signals:
    void panoramaStarted(int rows, int cols);
    void panoramaProgress(int completed, int total);
    void panoramaFinished();
    void panoramaStopped();
    void panoramaFailed(const QString &error);

private:
    void sendCommand(quint32 cmd, const QByteArray &data);
    void sendCommandModule(quint32 moduleId, quint32 cmd, const QByteArray &data);
    QByteArray buildGridCommand(quint8 selector, int value);
    void handleNotificationProgress(int total_count, int completed_count);

    DwarfWebSocketClient *m_client = nullptr;
    int m_lastRows = 0;
    int m_lastCols = 0;
    bool m_isRunning = false;
    bool m_justCompleted = false;  // Prevent restart from late response
    int m_lastProgressCompleted = 0;
    bool m_loggedProgressHexThisRun = false;
};
