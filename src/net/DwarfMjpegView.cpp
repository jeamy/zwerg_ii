#include "net/DwarfMjpegView.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPointF>

DwarfMjpegView::DwarfMjpegView(QWidget *parent)
    : QWidget(parent), m_image(nullptr) {
  setAttribute(Qt::WA_OpaquePaintEvent);
  setAutoFillBackground(false);
  m_lastImageRect = QRect();
}

void DwarfMjpegView::setSourceImage(const QImage *image) {
  m_image = image;
  update();
}

QRect DwarfMjpegView::imageRect() const {
  return m_lastImageRect;
}

QSize DwarfMjpegView::sourceImageSize() const {
  return m_image ? m_image->size() : QSize();
}

void DwarfMjpegView::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter p(this);
  p.fillRect(rect(), Qt::black);

  if (!m_image || m_image->isNull()) {
    m_lastImageRect = QRect();
    return;
  }

  QImage scaled =
      m_image->scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
  QPoint topLeft((width() - scaled.width()) / 2,
                 (height() - scaled.height()) / 2);
  p.drawImage(topLeft, scaled);

  m_lastImageRect = QRect(topLeft, scaled.size());
}

void DwarfMjpegView::mouseDoubleClickEvent(QMouseEvent *event) {
  QWidget::mouseDoubleClickEvent(event);

  if (!m_image || m_image->isNull())
    return;

  QImage scaled =
      m_image->scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
  const int imgW = scaled.width();
  const int imgH = scaled.height();
  const int x0 = (width() - imgW) / 2;
  const int y0 = (height() - imgH) / 2;

  const QPoint p = event->pos();
  if (p.x() < x0 || p.x() >= x0 + imgW || p.y() < y0 || p.y() >= y0 + imgH)
    return;

  const double u = double(p.x() - x0) / double(imgW);
  const double v = double(p.y() - y0) / double(imgH);

  const double nx = (u - 0.5) * 2.0;
  const double ny = (v - 0.5) * 2.0;

  emit pointClicked(QPointF(nx, ny));
}
