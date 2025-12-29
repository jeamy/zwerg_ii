#pragma once

#include <QWidget>

#include "CameraSettingsPanel.h"

class CameraSettingsPanel;
class DwarfCameraController;
class QPixmap;

class ParametersOverlayPanel : public QWidget {
  Q_OBJECT

public:
  explicit ParametersOverlayPanel(QWidget *parent = nullptr);

  void setCameraController(DwarfCameraController *controller);
  void setCameraMode(CameraSettingsPanel::CameraMode mode);
  void setClientMode(bool enabled);
  void setCaptureStatusText(const QString &text);
  void setCapturePreview(const QPixmap &pixmap);
  void clearCapturePreview();
  void applyDefaultParamsConfig(const QJsonDocument &document);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  void setupUi();

  CameraSettingsPanel *m_panel = nullptr;
  QPoint m_dragOffset;
};
