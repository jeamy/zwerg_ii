#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QByteArray>
#include <QQueue>
#include <functional>

/**
 * @brief Minimal cross-platform FTP client for DWARF II.
 *
 * Uses QTcpSocket for control and data connections.
 * Supports anonymous login and passive mode (PASV).
 * Works on Linux, Windows, and macOS.
 */
class DwarfFtpClient : public QObject {
  Q_OBJECT

public:
  explicit DwarfFtpClient(QObject *parent = nullptr);
  ~DwarfFtpClient();

  /**
   * @brief Download a file from the DWARF FTP server.
   * @param host DWARF IP address
   * @param remotePath Remote file path (e.g. /DWARF_II/Normal_Photos/x.jpg)
   * @param localPath Local file path to save to
   */
  void downloadFile(const QString &host, const QString &remotePath,
                    const QString &localPath);

  /**
   * @brief Download file content to memory (for thumbnails).
   * @param host DWARF IP address
   * @param remotePath Remote file path
   * @param callback Called with file data when complete
   */
  void downloadToMemory(const QString &host, const QString &remotePath,
                        std::function<void(const QByteArray &)> callback);

  bool isBusy() const { return m_state != State::Idle; }

signals:
  void downloadStarted(const QString &remotePath);
  void downloadProgress(const QString &remotePath, qint64 received,
                        qint64 total);
  void downloadFinished(const QString &remotePath, const QString &localPath);
  void downloadDataReady(const QString &remotePath, const QByteArray &data);
  void errorOccurred(const QString &remotePath, const QString &error);

private slots:
  void onControlConnected();
  void onControlReadyRead();
  void onControlError(QAbstractSocket::SocketError error);
  void onDataConnected();
  void onDataReadyRead();
  void onDataDisconnected();
  void onDataError(QAbstractSocket::SocketError error);

private:
  enum class State {
    Idle,
    Connecting,
    WaitingWelcome,
    SendingUser,
    SendingPass,
    SendingType,
    SendingPasv,
    SendingRetr,
    Downloading,
    Done
  };

  struct Job {
    QString host;
    QString remotePath;
    QString localPath; // empty for memory download
    std::function<void(const QByteArray &)> memoryCallback;
    bool toMemory;
  };

  void tryStartNextJob();
  void startNextJob();
  void sendCommand(const QString &cmd);
  void handleResponse(int code, const QString &line);
  void connectToDataPort(const QString &pasvResponse);
  void finishDownload(bool success, const QString &error = QString());
  void cleanup();

  QTcpSocket *m_controlSocket;
  QTcpSocket *m_dataSocket;
  State m_state;
  QQueue<Job> m_queue;
  Job m_currentJob;
  QByteArray m_dataBuffer;
  QByteArray m_controlBuffer;
  QString m_dataHost;
  quint16 m_dataPort;
  qint64 m_fileSize;

  static constexpr int FTP_PORT = 21;
  static constexpr int TIMEOUT_MS = 60000; // 60 seconds for slow connections
};
