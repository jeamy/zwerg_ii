#pragma once

#include "DwarfWebSocketClient.h"
#include "focus.pb.h"

#include <QObject>

class DwarfFocusController : public QObject {
  Q_OBJECT

public:
  explicit DwarfFocusController(QObject *parent = nullptr);

  void setClient(DwarfWebSocketClient *client);

  void autoFocusNormal();
  void manualStepNear();
  void manualStepFar();
  void startManualContinuousNear();
  void startManualContinuousFar();
  void stopManualContinuousFocus();
  void startAstroAutoFocus(bool fastMode = false);
  void stopAstroAutoFocus();

public slots:
  void handleFocusMessage(quint32 cmd, const QByteArray &data);
  void handleNotification(quint32 cmd, const QByteArray &data);

signals:
  void errorOccurred(const QString &message);
  void statusMessage(const QString &message);
  void focusPositionChanged(int position);

private:
  DwarfWebSocketClient *m_client;

  void sendCommand(quint32 cmd, const QByteArray &data,
                   const QString &errorContext);
  quint32 moduleId() const;
  quint32 cmdAutoFocus() const;
  quint32 cmdManualSingleStep() const;
  quint32 cmdManualContinuousStart() const;
  quint32 cmdManualContinuousStop() const;
  quint32 cmdAstroAutoFocusStart() const;
  quint32 cmdAstroAutoFocusStop() const;
};
