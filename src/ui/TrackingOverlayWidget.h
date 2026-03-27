#pragma once

#include <QRect>
#include <QVector>
#include <QWidget>

class DwarfMjpegView;

class TrackingOverlayWidget : public QWidget {
  Q_OBJECT

public:
  struct TrackBox {
    QRect rect;
    int id = -1;
  };

  explicit TrackingOverlayWidget(QWidget *parent = nullptr);

  void setSelectionEnabled(bool enabled);
  bool selectionEnabled() const { return m_selectionEnabled; }
  void clearSelection();
  void setTrackBoxes(const QVector<TrackBox> &boxes);

signals:
  void selectionFinished(const QRect &imageRect);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  QRect imageRectToViewRect(const QRect &imageRect) const;
  QRect selectionRectInImageSpace() const;
  DwarfMjpegView *view() const;

  bool m_selectionEnabled = false;
  bool m_selecting = false;
  QPoint m_dragStart;
  QPoint m_dragEnd;
  QVector<TrackBox> m_boxes;
};
