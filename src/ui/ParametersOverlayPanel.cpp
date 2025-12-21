#include "ParametersOverlayPanel.h"

#include "CameraSettingsPanel.h"
#include "../net/DwarfCameraController.h"

#include <QCursor>
#include <QMouseEvent>
#include <QVBoxLayout>
 #include <QPixmap>
 #include <QGroupBox>
 #include <QStyleOptionGroupBox>

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

void ParametersOverlayPanel::applyDefaultParamsConfig(const QJsonDocument &document) {
  if (m_panel)
    m_panel->applyDefaultParamsConfig(document);
}

void ParametersOverlayPanel::mousePressEvent(QMouseEvent *event) {
  if (!m_panel) {
    QWidget::mousePressEvent(event);
    return;
  }

  const QPoint localPos = event->pos();
  const QPoint panelPos = m_panel->mapFrom(this, localPos);
  QWidget *child = m_panel->childAt(panelPos);
  const QGroupBox *gb = nullptr;
  for (QWidget *w = child; w; w = w->parentWidget()) {
    gb = qobject_cast<const QGroupBox *>(w);
    if (gb)
      break;
    if (w == m_panel)
      break;
  }

  bool allowDrag = false;
  if (gb) {
    QStyleOptionGroupBox opt;
    opt.initFrom(gb);
    opt.text = gb->title();
    opt.subControls = QStyle::SC_GroupBoxLabel;
    const QRect labelRect =
        style()->subControlRect(QStyle::CC_GroupBox, &opt, QStyle::SC_GroupBoxLabel, gb);
    const QPoint gbPos = gb->mapFrom(m_panel, panelPos);
    allowDrag = labelRect.adjusted(-6, -6, 6, 6).contains(gbPos);
  }

  if (allowDrag)
    m_dragOffset = QCursor::pos() - pos();
  else
    m_dragOffset = QPoint();

  QWidget::mousePressEvent(event);
}

void ParametersOverlayPanel::mouseMoveEvent(QMouseEvent *event) {
  if ((event->buttons() & Qt::LeftButton) && !m_dragOffset.isNull()) {
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
  m_dragOffset = QPoint();
  QWidget::mouseReleaseEvent(event);
}
