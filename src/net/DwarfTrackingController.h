#pragma once

#include <QRect>
#include <QString>
#include <QVector>
#include <QObject>

class QByteArray;
class DwarfWebSocketClient;

class DwarfTrackingController : public QObject {
  Q_OBJECT

public:
  struct TrackResult {
    QRect rect;
    int id = -1;
  };

  explicit DwarfTrackingController(QObject *parent = nullptr);

  void setClient(DwarfWebSocketClient *client);

  void startTrack(const QRect &box);
  void stopTrack();
  void startSentryMode(int mode = 0);
  void stopSentryMode();
  void startMot();
  void motTrackOne(int id);
  void wideMotTrackOne(int id);
  void startUfoMode(int mode = 1);
  void stopUfoMode();
  void setWideTeleTrackSwitch(int mode);
  void setUfoHandAutoMode(int mode);

  QRect currentTrackRect() const { return m_currentTrackRect; }
  int currentTrackId() const { return m_currentTrackId; }
  QVector<TrackResult> multiTrackResults() const { return m_multiTrackResults; }
  int sentryState() const { return m_sentryState; }
  int ufoState() const { return m_ufoState; }

public slots:
  void handleTrackMessage(quint32 cmd, const QByteArray &data);
  void handleNotification(quint32 cmd, const QByteArray &data);

signals:
  void errorOccurred(const QString &message);
  void statusMessage(const QString &message);
  void trackSelectionStarted();
  void trackStopped();
  void trackResultChanged();
  void multiTrackResultsChanged();
  void sentryStateChanged(int state);
  void ufoStateChanged(int state);
  void motModeChanged(bool active);

private:
  void sendCommand(quint32 cmd, const QByteArray &data);
  void handleTrackResponse(quint32 cmd, const QByteArray &data);
  void clearResults();

  DwarfWebSocketClient *m_client = nullptr;
  QRect m_currentTrackRect;
  int m_currentTrackId = -1;
  QVector<TrackResult> m_multiTrackResults;
  int m_sentryState = 0;
  int m_ufoState = 0;
  bool m_motActive = false;
};
