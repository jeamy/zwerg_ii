#include "DwarfFtpDownloader.h"
#include "DwarfFtpClient.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <functional>

DwarfFtpDownloader::DwarfFtpDownloader(QObject *parent)
    : QObject(parent), m_ftpClient(new DwarfFtpClient(this)) {

  connect(m_ftpClient, &DwarfFtpClient::downloadStarted, this,
          &DwarfFtpDownloader::onFtpDownloadStarted);
  connect(m_ftpClient, &DwarfFtpClient::downloadFinished, this,
          &DwarfFtpDownloader::onFtpDownloadFinished);
  connect(m_ftpClient, &DwarfFtpClient::downloadProgress, this,
          &DwarfFtpDownloader::onFtpDownloadProgress);
  connect(m_ftpClient, &DwarfFtpClient::errorOccurred, this,
          &DwarfFtpDownloader::onFtpError);
}

DwarfFtpDownloader::~DwarfFtpDownloader() {}

bool DwarfFtpDownloader::isBusy() const {
  return m_ftpClient && m_ftpClient->isBusy();
}

QString DwarfFtpDownloader::convertToFtpPath(const QString &apiPath) const {
  // API returns paths like /sdcard/DWARF_II/Normal_Photos/xxx.jpg
  // FTP server exposes /DWARF_II/... (without /sdcard)
  QString ftpPath = apiPath;
  if (ftpPath.startsWith(QStringLiteral("/sdcard"))) {
    ftpPath = ftpPath.mid(7); // Remove "/sdcard"
  }
  if (!ftpPath.startsWith('/')) {
    ftpPath = '/' + ftpPath;
  }
  return ftpPath;
}

QString DwarfFtpDownloader::extractFileName(const QString &path) const {
  QFileInfo fi(path);
  return fi.fileName();
}

void DwarfFtpDownloader::downloadFile(const QString &ip,
                                      const QString &remotePath,
                                      const QString &localDir) {
  QString ftpPath = convertToFtpPath(remotePath);
  QString fileName = extractFileName(remotePath);

  // Ensure local directory exists
  QDir dir(localDir);
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  QString localPath = dir.filePath(fileName);

  qDebug() << "[FtpDownloader] Queued file download:" << ftpPath << "->"
           << localPath;

  m_ftpClient->downloadFile(ip, ftpPath, localPath);
}

void DwarfFtpDownloader::downloadThumbnail(
    const QString &ip, const QString &thumbnailPath,
    std::function<void(const QByteArray &)> callback) {
  QString ftpPath = convertToFtpPath(thumbnailPath);

  qDebug() << "[FtpDownloader] Queued thumbnail download:" << ftpPath;

  m_ftpClient->downloadToMemory(ip, ftpPath, callback);
}

void DwarfFtpDownloader::onFtpDownloadStarted(const QString &remotePath) {
  // Don't emit signals for thumbnail downloads (memory downloads)
  if (remotePath.contains(QStringLiteral("thumbnail"), Qt::CaseInsensitive))
    return;

  m_currentFileName = extractFileName(remotePath);
  emit downloadStarted(m_currentFileName);
}

void DwarfFtpDownloader::onFtpDownloadFinished(const QString &remotePath,
                                               const QString &localPath) {
  // Don't emit signals for thumbnail downloads
  if (localPath.isEmpty())
    return;

  QString fileName = extractFileName(remotePath);
  emit downloadFinished(fileName, localPath);
}

void DwarfFtpDownloader::onFtpDownloadProgress(const QString &remotePath,
                                               qint64 received, qint64 total) {
  // Don't emit signals for thumbnail downloads
  if (remotePath.contains(QStringLiteral("thumbnail"), Qt::CaseInsensitive))
    return;

  QString fileName = extractFileName(remotePath);
  emit downloadProgress(fileName, received, total);
}

void DwarfFtpDownloader::onFtpError(const QString &remotePath,
                                    const QString &error) {
  // Don't emit signals for thumbnail downloads
  if (remotePath.contains(QStringLiteral("thumbnail"), Qt::CaseInsensitive))
    return;

  QString fileName = extractFileName(remotePath);
  qWarning() << "[FtpDownloader] Download error:" << fileName << error;
  emit downloadError(fileName, error);
}
