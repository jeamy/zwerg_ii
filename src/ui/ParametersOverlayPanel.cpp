#include "ParametersOverlayPanel.h"

#include "CameraSettingsPanel.h"
#include "../net/DwarfCameraController.h"

#include <QCursor>
#include <QMouseEvent>
#include <QVBoxLayout>
 #include <QPixmap>

ParametersOverlayPanel::ParametersOverlayPanel(QWidget *parent) : QWidget(parent) {
  setupUi();
}

void ParametersOverlayPanel::setupUi() {
  setObjectName("parametersOverlayPanel");

  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  m_panel = new CameraSettingsPanel(this);
  m_panel->setDisplayMode(CameraSettingsPanel::DisplayMode::Compact);
  mainLayout->addWidget(m_panel);
}

void ParametersOverlayPanel::setCameraController(DwarfCameraController *controller) {
  if (m_panel)
    m_panel->setCameraController(controller);
}

void ParametersOverlayPanel::setCameraMode(CameraSettingsPanel::CameraMode mode) {
  if (m_panel)
    m_panel->setCameraMode(mode);
}

void ParametersOverlayPanel::setCaptureStatusText(const QString &text) {
  if (m_panel)
    m_panel->setCaptureStatusText(text);
}

void ParametersOverlayPanel::setCapturePreview(const QPixmap &pixmap) {
  if (m_panel)
    m_panel->setCapturePreview(pixmap);
}

void ParametersOverlayPanel::clearCapturePreview() {
  if (m_panel)
    m_panel->clearCapturePreview();
}

void ParametersOverlayPanel::mousePressEvent(QMouseEvent *event) {
  m_dragOffset = QCursor::pos() - pos();
  QWidget::mousePressEvent(event);
}

void ParametersOverlayPanel::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    QPoint newPos = QCursor::pos() - m_dragOffset;
    if (parentWidget()) {
      newPos.setX(qBound(0, newPos.x(), parentWidget()->width() - width()));
      newPos.setY(qBound(0, newPos.y(), parentWidget()->height() - height()));
    }
    move(newPos);
  }
  QWidget::mouseMoveEvent(event);
}

void ParametersOverlayPanel::mouseReleaseEvent(QMouseEvent *event) {
  QWidget::mouseReleaseEvent(event);
}
