#pragma once

#include <QObject>

class DwarfWebSocketClient;

/**
 * @brief Controller for DWARF II astro operations (GOTO, calibration, stacking)
 */
class DwarfAstroController : public QObject {
    Q_OBJECT

public:
    explicit DwarfAstroController(QObject *parent = nullptr);
    
    void setClient(DwarfWebSocketClient *client);
    
    // Calibration
    void startCalibration();
    void stopCalibration();
    
    // GOTO Deep Sky Objects (no GPS needed)
    void gotoDSO(double ra, double dec, const QString &targetName);
    void oneClickGotoDSO(double ra, double dec, const QString &targetName);
    void stopGoto();
    
    // GOTO Solar System (GPS needed)
    void gotoSolarSystem(int index, double lon, double lat, const QString &targetName);
    void oneClickGotoSolarSystem(int index, double lon, double lat, const QString &targetName);
    
    // Special target tracking (Sun/Moon)
    void trackSpecialTarget(int index, double lon, double lat);  // 0=Sun, 1=Moon
    void stopTrackSpecialTarget();
    
    // Stacking
    void startLiveStacking();
    void stopLiveStacking();
    void startWideLiveStacking();
    void stopWideLiveStacking();
    
    // Dark frames
    void checkDarkFrame();
    void captureDarkFrame(bool reshoot = false);
    void captureDarkFrameWithParams(int expIndex, int gainIndex, int binIndex, int count);
    void stopCaptureDarkFrame();
    void getDarkFrameList();
    
    // EQ alignment
    void startEqSolving(double lon, double lat);
    void stopEqSolving();
    
    // Go Live
    void goLive();
    
    // Handle incoming astro messages
    void handleAstroMessage(quint32 cmd, const QByteArray &data);
    
    // Handle notification messages (Module 9)
    void handleNotification(quint32 cmd, const QByteArray &data);

signals:
    void calibrationStarted();
    void calibrationProgress(int progress);
    void calibrationCompleted(bool success);
    void calibrationFailed(const QString &error);
    
    void gotoStarted(const QString &targetName);
    void gotoProgress(int step);  // 10, 20, 30, 40
    void gotoCompleted();
    void gotoFailed(const QString &error);
    
    void stackingStarted();
    void stackingProgress(int currentFrame, int totalFrames, int stackedFrames, int rejectedFrames);
    void stackingStateChanged(int state);  // 0=idle, 1=capturing, 2=stacking
    void stackingStopped();
    void stackingFailed(const QString &error);
    
    void darkFrameProgress(int current, int total);
    void darkFrameListReceived(const QList<QVariantMap> &frames);
    
    void eqSolvingResult(double aziError, double altError);
    
    void batteryChanged(int percent);
    void temperatureChanged(int celsius);

private:
    void sendCommand(quint32 cmd, const QByteArray &data);
    
    DwarfWebSocketClient *m_client = nullptr;
};
