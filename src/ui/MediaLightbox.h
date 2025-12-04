#pragma once

#include <QDialog>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVector>

class MediaLightbox : public QDialog {
  Q_OBJECT

public:
  explicit MediaLightbox(QWidget *parent = nullptr);

  void setMedia(const QJsonObject &mediaInfo, const QPixmap &thumbnail);
  void setMediaList(const QVector<QJsonObject> &list, int currentIndex,
                    const QVector<QPixmap> &thumbnails = {});

signals:
  void downloadRequested(const QJsonObject &mediaInfo);

protected:
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void navigatePrev();
  void navigateNext();
  void updateCurrentItem();

private:
  QLabel *m_imageLabel;
  QLabel *m_infoLabel;
  QLabel *m_counterLabel;
  QPushButton *m_prevButton;
  QPushButton *m_nextButton;
  QPushButton *m_downloadButton;
  QPushButton *m_closeButton;
  QJsonObject m_mediaInfo;
  QVector<QJsonObject> m_mediaList;
  QVector<QPixmap> m_thumbnails;
  int m_currentIndex;
};
