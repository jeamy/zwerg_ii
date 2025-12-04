#pragma once

#include <QObject>
#include <QString>
#include <functional>

class DwarfFtpClient;

/**
 * @brief Asynchronous FTP downloader for DWARF II media files.
 *
 * Uses DwarfFtpClient (pure Qt, cross-platform) to download files.
 * The DWARF exposes files at ftp://IP/DWARF_II/... (without /sdcard prefix).
 * Works on Linux, Windows, and macOS.
 */
class DwarfFtpDownloader : public QObject {
  Q_OBJECT

public:
  explicit DwarfFtpDownloader(QObject *parent = nullptr);
  ~DwarfFtpDownloader();

  /**
   * @brief Queue a file for download.
   * @param ip DWARF IP address
   * @param remotePath Path from API (e.g. /sdcard/DWARF_II/Normal_Photos/x.jpg)
   * @param localDir Local directory to save to
   */
  void downloadFile(const QString &ip, const QString &remotePath,
                    const QString &localDir);

  /**
   * @brief Download a thumbnail to memory.
   * @param ip DWARF IP address
   * @param thumbnailPath Path from API (thumbnailPath field)
   * @param callback Called with image data when complete
   */
  void downloadThumbnail(const QString &ip, const QString &thumbnailPath,
                         std::function<void(const QByteArray &)> callback);

  /**
   * @brief Delete a file from the DWARF.
   * @param ip DWARF IP address
   * @param remotePath Path from API (e.g. /sdcard/DWARF_II/Normal_Photos/x.jpg)
   * @param callback Called with success status
   */
  void deleteFile(const QString &ip, const QString &remotePath,
                  std::function<void(bool success, const QString &error)> callback);

  bool isBusy() const;

signals:
  void downloadStarted(const QString &fileName);
  void downloadFinished(const QString &fileName, const QString &localPath);
  void downloadError(const QString &fileName, const QString &error);
  void downloadProgress(const QString &fileName, qint64 received, qint64 total);

private slots:
  void onFtpDownloadStarted(const QString &remotePath);
  void onFtpDownloadFinished(const QString &remotePath, const QString &localPath);
  void onFtpDownloadProgress(const QString &remotePath, qint64 received, qint64 total);
  void onFtpError(const QString &remotePath, const QString &error);

private:
  QString convertToFtpPath(const QString &apiPath) const;
  QString extractFileName(const QString &path) const;

  DwarfFtpClient *m_ftpClient;
  QString m_currentFileName;
};
