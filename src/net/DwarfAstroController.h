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
    void startLiveStacking(bool useDarks = true);
    void stopLiveStacking();
    void startWideLiveStacking(bool useDarks = true);
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
    
    // Stacking state getters
    bool isStacking() const { return m_stackingState != 0; }
    int stackingState() const { return m_stackingState; }
    void getStackingProgress(int &current, int &total, int &stacked, int &rejected) const;
    void getStackingSettings(int &expIndex, int &gainIndex, int &binIndex) const;
    
    // Handle incoming astro messages
    void handleAstroMessage(quint32 cmd, const QByteArray &data);
    
    // Handle notification messages (Module 9)
    void handleNotification(quint32 cmd, const QByteArray &data);

signals:
    void calibrationStarted();
    void calibrationProgress(int step, int total);
    void calibrationCompleted(bool success);
    void calibrationFailed(const QString &error, int code = 0);
    
    void gotoStarted(const QString &targetName);
    void gotoProgress(int step);  // 10, 20, 30, 40
    void gotoCompleted();
    void gotoFailed(const QString &error, int code = 0);
    
    void stackingStarted();
    void stackingProgress(int currentFrame, int totalFrames, int stackedFrames, int rejectedFrames);
    void stackingStateChanged(int state);  // 0=idle, 1=capturing, 2=stacking
    void stackingStopped();
    void stackingFailed(const QString &error, int code = 0);
    
    void darkFrameProgress(int current, int total);
    void darkFrameListReceived(const QList<QVariantMap> &frames);
    
    void eqSolvingResult(double aziError, double altError);
    
    void specialTrackingStarted(int index);
    void specialTrackingStopped();
    
    void batteryChanged(int percent);
    void temperatureChanged(int celsius);
    void sdCardInfoReceived(float totalGB, float freeGB);

private:
    void sendCommand(quint32 cmd, const QByteArray &data);
    
    DwarfWebSocketClient *m_client = nullptr;
    int m_currentSpecialTargetIndex = -1;
    
    // Stacking state tracking
    int m_stackingState = 0; // 0=idle, 1=capturing, 2=stacking
    int m_stackingCurrentFrame = 0;
    int m_stackingTotalFrames = 0;
    int m_stackingStackedFrames = 0;
    int m_stackingRejectedFrames = 0;
    int m_stackingExpIndex = -1;
    int m_stackingGainIndex = -1;
    int m_stackingBinIndex = -1;
};
