#include "MediaLightbox.h"
#include <QFileInfo>
#include <QHBoxLayout>
#include <QVBoxLayout>

MediaLightbox::MediaLightbox(QWidget *parent)
    : QDialog(parent), m_imageLabel(new QLabel(this)),
      m_infoLabel(new QLabel(this)), m_counterLabel(new QLabel(this)),
      m_prevButton(new QPushButton(tr("\u25c0 Prev"), this)),
      m_nextButton(new QPushButton(tr("Next \u25b6"), this)),
      m_downloadButton(new QPushButton(tr("Download"), this)),
      m_closeButton(new QPushButton(tr("Close"), this)), m_currentIndex(-1) {

  setWindowTitle(tr("Media Preview"));
  setMinimumSize(600, 500);

  // Image display
  m_imageLabel->setAlignment(Qt::AlignCenter);
  m_imageLabel->setMinimumSize(500, 350);
  m_imageLabel->setStyleSheet("QLabel { background-color: #1a1a1a; }");

  // Info label
  m_infoLabel->setWordWrap(true);
  m_infoLabel->setTextFormat(Qt::RichText);

  // Counter label
  m_counterLabel->setAlignment(Qt::AlignCenter);

  // Navigation buttons
  QHBoxLayout *navLayout = new QHBoxLayout;
  navLayout->addWidget(m_prevButton);
  navLayout->addStretch();
  navLayout->addWidget(m_counterLabel);
  navLayout->addStretch();
  navLayout->addWidget(m_nextButton);

  // Action buttons
  QHBoxLayout *buttonLayout = new QHBoxLayout;
  buttonLayout->addStretch();
  buttonLayout->addWidget(m_downloadButton);
  buttonLayout->addWidget(m_closeButton);

  // Main layout
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addLayout(navLayout);
  mainLayout->addWidget(m_imageLabel, 1);
  mainLayout->addWidget(m_infoLabel);
  mainLayout->addLayout(buttonLayout);

  connect(m_prevButton, &QPushButton::clicked, this,
          &MediaLightbox::navigatePrev);
  connect(m_nextButton, &QPushButton::clicked, this,
          &MediaLightbox::navigateNext);

  connect(m_downloadButton, &QPushButton::clicked, this, [this]() {
    emit downloadRequested(m_mediaInfo);
  });

  connect(m_closeButton, &QPushButton::clicked, this, &QDialog::reject);

  // Initially hide navigation if no list
  m_prevButton->setVisible(false);
  m_nextButton->setVisible(false);
  m_counterLabel->setVisible(false);
}

void MediaLightbox::setMedia(const QJsonObject &mediaInfo,
                             const QPixmap &thumbnail) {
  m_mediaInfo = mediaInfo;

  // Scale thumbnail to fit dialog
  if (!thumbnail.isNull()) {
    QPixmap scaled = thumbnail.scaled(m_imageLabel->size() - QSize(20, 20),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
    m_imageLabel->setPixmap(scaled);
  } else {
    m_imageLabel->setPixmap(QPixmap());
  }

  // Build info text
  QString filePath = mediaInfo.value("filePath").toString();
  QString fileName = QFileInfo(filePath).fileName();
  if (fileName.isEmpty())
    fileName = mediaInfo.value("fileName").toString();

  int mediaType = mediaInfo.value("mediaType").toInt(-1);
  QString typeStr;
  switch (mediaType) {
  case 1:
    typeStr = tr("Photo");
    break;
  case 2:
    typeStr = tr("Video");
    break;
  case 3:
    typeStr = tr("Burst");
    break;
  case 4:
    typeStr = tr("Astro");
    break;
  case 5:
    typeStr = tr("Panorama");
    break;
  default:
    typeStr = tr("Unknown");
    break;
  }

  int camId = mediaInfo.value("camId").toInt(-1);
  QString camStr = (camId == 0) ? tr("Tele") : (camId == 1) ? tr("Wide") : "";

  QString info = QStringLiteral("<b>%1</b><br>").arg(fileName);
  info += QStringLiteral("%1: %2").arg(tr("Type"), typeStr);
  if (!camStr.isEmpty())
    info += QStringLiteral(" | %1: %2").arg(tr("Camera"), camStr);

  m_infoLabel->setText(info);

  setWindowTitle(fileName);
}

void MediaLightbox::setMediaList(const QVector<QJsonObject> &list,
                                 int currentIndex,
                                 const QVector<QPixmap> &thumbnails) {
  m_mediaList = list;
  m_thumbnails = thumbnails;

  if (m_mediaList.isEmpty()) {
    m_currentIndex = -1;
    m_prevButton->setVisible(false);
    m_nextButton->setVisible(false);
    m_counterLabel->setVisible(false);
    return;
  }

  if (currentIndex < 0 || currentIndex >= m_mediaList.size())
    m_currentIndex = 0;
  else
    m_currentIndex = currentIndex;

  bool multi = (m_mediaList.size() > 1);
  m_prevButton->setVisible(multi);
  m_nextButton->setVisible(multi);
  m_counterLabel->setVisible(multi);

  updateCurrentItem();
}

void MediaLightbox::navigatePrev() {
  if (m_mediaList.isEmpty() || m_currentIndex <= 0)
    return;

  --m_currentIndex;
  updateCurrentItem();
}

void MediaLightbox::navigateNext() {
  if (m_mediaList.isEmpty() || m_currentIndex >= m_mediaList.size() - 1)
    return;

  ++m_currentIndex;
  updateCurrentItem();
}

void MediaLightbox::updateCurrentItem() {
  if (m_mediaList.isEmpty() || m_currentIndex < 0 ||
      m_currentIndex >= m_mediaList.size())
    return;

  const QJsonObject &obj = m_mediaList.at(m_currentIndex);

  QPixmap thumb;
  if (m_currentIndex >= 0 && m_currentIndex < m_thumbnails.size())
    thumb = m_thumbnails.at(m_currentIndex);

  setMedia(obj, thumb);

  // Update navigation state and counter
  m_prevButton->setEnabled(m_currentIndex > 0);
  m_nextButton->setEnabled(m_currentIndex < m_mediaList.size() - 1);
  m_counterLabel->setText(
      tr("%1 / %2").arg(m_currentIndex + 1).arg(m_mediaList.size()));
}

void MediaLightbox::keyPressEvent(QKeyEvent *event) {
  switch (event->key()) {
  case Qt::Key_Left:
    navigatePrev();
    break;
  case Qt::Key_Right:
    navigateNext();
    break;
  case Qt::Key_Escape:
    reject();
    break;
  default:
    QDialog::keyPressEvent(event);
  }
}

