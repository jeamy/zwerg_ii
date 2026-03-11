#include "TrackingOverlayWidget.h"

#include "net/DwarfMjpegView.h"

#include <QMouseEvent>
#include <QPainter>

TrackingOverlayWidget::TrackingOverlayWidget(QWidget *parent) : QWidget(parent) {
  setAttribute(Qt::WA_NoSystemBackground, true);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_TransparentForMouseEvents, true);
}

void TrackingOverlayWidget::setSelectionEnabled(bool enabled) {
  m_selectionEnabled = enabled;
  if (!enabled)
    m_selecting = false;
  setAttribute(Qt::WA_TransparentForMouseEvents, !enabled);
  update();
}

void TrackingOverlayWidget::clearSelection() {
  m_selecting = false;
  m_dragStart = QPoint();
  m_dragEnd = QPoint();
  update();
}

void TrackingOverlayWidget::setTrackBoxes(const QVector<TrackBox> &boxes) {
  m_boxes = boxes;
  update();
}

void TrackingOverlayWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  for (const TrackBox &box : m_boxes) {
    const QRect viewRect = imageRectToViewRect(box.rect);
    if (!viewRect.isValid())
      continue;
    painter.setPen(QPen(QColor(46, 204, 113), 2));
    painter.drawRect(viewRect);
    if (box.id >= 0) {
      painter.fillRect(QRect(viewRect.topLeft(), QSize(34, 18)),
                       QColor(46, 204, 113, 180));
      painter.setPen(Qt::black);
      painter.drawText(QRect(viewRect.topLeft(), QSize(34, 18)),
                       Qt::AlignCenter, QString::number(box.id));
    }
  }

  if (m_selectionEnabled || m_selecting) {
    const QRect selection = QRect(m_dragStart, m_dragEnd).normalized();
    if (selection.isValid()) {
      painter.setPen(QPen(QColor(241, 196, 15), 2, Qt::DashLine));
      painter.fillRect(selection, QColor(241, 196, 15, 35));
      painter.drawRect(selection);
    }
  }
}

void TrackingOverlayWidget::mousePressEvent(QMouseEvent *event) {
  if (!m_selectionEnabled || event->button() != Qt::LeftButton) {
    QWidget::mousePressEvent(event);
    return;
  }

  const DwarfMjpegView *imageView = view();
  if (!imageView) {
    QWidget::mousePressEvent(event);
    return;
  }
  const QRect imgRect = imageView->imageRect();
  if (!imgRect.contains(event->pos())) {
    QWidget::mousePressEvent(event);
    return;
  }

  m_selecting = true;
  m_dragStart = event->pos();
  m_dragEnd = event->pos();
  update();
}

void TrackingOverlayWidget::mouseMoveEvent(QMouseEvent *event) {
  if (!m_selecting) {
    QWidget::mouseMoveEvent(event);
    return;
  }
  m_dragEnd = event->pos();
  update();
}

void TrackingOverlayWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (!m_selecting || event->button() != Qt::LeftButton) {
    QWidget::mouseReleaseEvent(event);
    return;
  }

  m_dragEnd = event->pos();
  m_selecting = false;
  const QRect imageRect = selectionRectInImageSpace();
  setSelectionEnabled(false);
  if (imageRect.width() >= 8 && imageRect.height() >= 8)
    emit selectionFinished(imageRect);
  clearSelection();
}

QRect TrackingOverlayWidget::imageRectToViewRect(const QRect &imageRect) const {
  const DwarfMjpegView *imageView = view();
  if (!imageView)
    return QRect();

  const QRect displayRect = imageView->imageRect();
  const QSize sourceSize = imageView->sourceImageSize();
  if (!displayRect.isValid() || !sourceSize.isValid() || sourceSize.width() <= 0 ||
      sourceSize.height() <= 0) {
    return QRect();
  }

  const double sx = static_cast<double>(displayRect.width()) / sourceSize.width();
  const double sy = static_cast<double>(displayRect.height()) / sourceSize.height();
  return QRect(displayRect.left() + qRound(imageRect.x() * sx),
               displayRect.top() + qRound(imageRect.y() * sy),
               qRound(imageRect.width() * sx), qRound(imageRect.height() * sy));
}

QRect TrackingOverlayWidget::selectionRectInImageSpace() const {
  const DwarfMjpegView *imageView = view();
  if (!imageView)
    return QRect();

  const QRect displayRect = imageView->imageRect();
  const QSize sourceSize = imageView->sourceImageSize();
  if (!displayRect.isValid() || !sourceSize.isValid() || sourceSize.width() <= 0 ||
      sourceSize.height() <= 0) {
    return QRect();
  }

  QRect selection = QRect(m_dragStart, m_dragEnd).normalized().intersected(displayRect);
  if (!selection.isValid())
    return QRect();

  const double sx = static_cast<double>(sourceSize.width()) / displayRect.width();
  const double sy = static_cast<double>(sourceSize.height()) / displayRect.height();

  return QRect(qRound((selection.x() - displayRect.x()) * sx),
               qRound((selection.y() - displayRect.y()) * sy),
               qRound(selection.width() * sx),
               qRound(selection.height() * sy));
}

DwarfMjpegView *TrackingOverlayWidget::view() const {
  return qobject_cast<DwarfMjpegView *>(parentWidget());
}
