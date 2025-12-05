#include "DwarfFtpClient.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QTimer>

DwarfFtpClient::DwarfFtpClient(QObject *parent)
    : QObject(parent), m_controlSocket(new QTcpSocket(this)),
      m_dataSocket(new QTcpSocket(this)), m_state(State::Idle), m_dataPort(0),
      m_fileSize(-1) {

  connect(m_controlSocket, &QTcpSocket::connected, this,
          &DwarfFtpClient::onControlConnected);
  connect(m_controlSocket, &QTcpSocket::readyRead, this,
          &DwarfFtpClient::onControlReadyRead);
  connect(m_controlSocket, &QTcpSocket::errorOccurred, this,
          &DwarfFtpClient::onControlError);

  connect(m_dataSocket, &QTcpSocket::connected, this,
          &DwarfFtpClient::onDataConnected);
  connect(m_dataSocket, &QTcpSocket::readyRead, this,
          &DwarfFtpClient::onDataReadyRead);
  connect(m_dataSocket, &QTcpSocket::disconnected, this,
          &DwarfFtpClient::onDataDisconnected);
  connect(m_dataSocket, &QTcpSocket::errorOccurred, this,
          &DwarfFtpClient::onDataError);
}

DwarfFtpClient::~DwarfFtpClient() { cleanup(); }

void DwarfFtpClient::downloadFile(const QString &host, const QString &remotePath,
                                  const QString &localPath) {
  Job job;
  job.host = host;
  job.remotePath = remotePath;
  job.localPath = localPath;
  job.type = JobType::Download;

  m_queue.enqueue(job);
  qDebug() << "[FtpClient] Queued file download:" << remotePath
           << "queue size:" << m_queue.size() << "state:" << static_cast<int>(m_state);

  tryStartNextJob();
}

void DwarfFtpClient::downloadToMemory(
    const QString &host, const QString &remotePath,
    std::function<void(const QByteArray &)> callback) {
  Job job;
  job.host = host;
  job.remotePath = remotePath;
  job.memoryCallback = callback;
  job.type = JobType::DownloadToMemory;

  m_queue.enqueue(job);
  qDebug() << "[FtpClient] Queued memory download:" << remotePath
           << "queue size:" << m_queue.size() << "state:" << static_cast<int>(m_state);

  tryStartNextJob();
}

void DwarfFtpClient::deleteFile(
    const QString &host, const QString &remotePath,
    std::function<void(bool success, const QString &error)> callback) {
  Job job;
  job.host = host;
  QFileInfo fi(remotePath);
  job.remotePath = fi.fileName();
  job.deleteDir = fi.path();
  job.deleteCallback = callback;
  job.type = JobType::Delete;

  m_queue.enqueue(job);
  qDebug() << "[FtpClient] Queued file delete:" << remotePath
           << "dir:" << job.deleteDir << "name:" << job.remotePath
           << "queue size:" << m_queue.size() << "state:" << static_cast<int>(m_state);

  tryStartNextJob();
}

void DwarfFtpClient::tryStartNextJob() {
  // Only start if truly idle and sockets are disconnected
  if (m_state != State::Idle)
    return;
  if (m_controlSocket->state() != QAbstractSocket::UnconnectedState)
    return;
  if (m_dataSocket->state() != QAbstractSocket::UnconnectedState)
    return;

  startNextJob();
}

void DwarfFtpClient::startNextJob() {
  if (m_queue.isEmpty()) {
    m_state = State::Idle;
    return;
  }

  // Ensure sockets are fully disconnected before starting new connection
  if (m_controlSocket->state() != QAbstractSocket::UnconnectedState) {
    m_controlSocket->abort();
  }
  if (m_dataSocket->state() != QAbstractSocket::UnconnectedState) {
    m_dataSocket->abort();
  }

  m_currentJob = m_queue.dequeue();
  m_dataBuffer.clear();
  m_controlBuffer.clear();
  m_fileSize = -1;

  qDebug() << "[FtpClient] Starting job:" << m_currentJob.remotePath
           << "type" << static_cast<int>(m_currentJob.type);
  if (m_currentJob.type == JobType::Download) {
    emit downloadStarted(m_currentJob.remotePath);
  }

  m_state = State::Connecting;
  m_controlSocket->connectToHost(m_currentJob.host, FTP_PORT);

  // Timeout
  QTimer::singleShot(TIMEOUT_MS, this, [this]() {
    if (m_state != State::Idle && m_state != State::Done) {
      finishDownload(false, tr("Connection timeout"));
    }
  });
}

void DwarfFtpClient::sendCommand(const QString &cmd) {
  qDebug() << "[FtpClient] >>>" << cmd.trimmed();
  m_controlSocket->write((cmd + "\r\n").toUtf8());
}

void DwarfFtpClient::onControlConnected() {
  qDebug() << "[FtpClient] Control connection established";
  m_state = State::WaitingWelcome;
}

void DwarfFtpClient::onControlReadyRead() {
  m_controlBuffer.append(m_controlSocket->readAll());

  // Process complete lines
  while (true) {
    int idx = m_controlBuffer.indexOf("\r\n");
    if (idx < 0)
      idx = m_controlBuffer.indexOf("\n");
    if (idx < 0)
      break;

    QString line = QString::fromUtf8(m_controlBuffer.left(idx)).trimmed();
    m_controlBuffer.remove(0, idx + (m_controlBuffer.at(idx) == '\r' ? 2 : 1));

    if (line.isEmpty())
      continue;

    qDebug() << "[FtpClient] <<<" << line;

    // Parse response code
    if (line.length() >= 3) {
      bool ok;
      int code = line.left(3).toInt(&ok);
      if (ok) {
        // Check if this is a continuation line (e.g., "220-Welcome")
        if (line.length() > 3 && line.at(3) == '-') {
          // Multi-line response, wait for final line
          continue;
        }
        handleResponse(code, line);
      }
    }
  }
}

void DwarfFtpClient::handleResponse(int code, const QString &line) {
  switch (m_state) {
  case State::WaitingWelcome:
    if (code == 220) {
      m_state = State::SendingUser;
      sendCommand("USER anonymous");
    } else {
      finishDownload(false, tr("Unexpected welcome: %1").arg(line));
    }
    break;

  case State::SendingUser:
    if (code == 331 || code == 230) {
      m_state = State::SendingPass;
      sendCommand("PASS anonymous@");
    } else {
      finishDownload(false, tr("USER failed: %1").arg(line));
    }
    break;

  case State::SendingPass:
    if (code == 230) {
      m_state = State::SendingType;
      sendCommand("TYPE I"); // Binary mode
    } else {
      finishDownload(false, tr("PASS failed: %1").arg(line));
    }
    break;

  case State::SendingType:
    if (code == 200) {
      if (m_currentJob.type == JobType::Delete) {
        // For delete, first change into the absolute directory, then delete
        m_state = State::SendingCwdForDelete;
        QString dir = m_currentJob.deleteDir;
        if (dir.isEmpty())
          dir = QStringLiteral("/");
        // Ensure absolute path starting with '/'
        if (!dir.startsWith('/'))
          dir.prepend('/');
        sendCommand(QString("CWD %1").arg(dir));
      } else {
        m_state = State::SendingPasv;
        sendCommand("PASV");
      }
    } else {
      finishDownload(false, tr("TYPE failed: %1").arg(line));
    }
    break;

  case State::SendingCwdForDelete:
    if (code == 250) {
      // Directory change ok, now delete file by name
      m_state = State::SendingDele;
      sendCommand(QString("DELE %1").arg(m_currentJob.remotePath));
    } else {
      finishDownload(false, tr("CWD failed: %1").arg(line));
    }
    break;

  case State::SendingDele:
    if (code == 250) {
      // Delete successful
      finishDownload(true);
    } else if (code == 550) {
      finishDownload(false, tr("File not found or cannot delete: %1").arg(m_currentJob.remotePath));
    } else {
      finishDownload(false, tr("DELE failed: %1").arg(line));
    }
    break;

  case State::SendingPasv:
    if (code == 227) {
      connectToDataPort(line);
    } else {
      finishDownload(false, tr("PASV failed: %1").arg(line));
    }
    break;

  case State::SendingRetr:
    if (code == 150 || code == 125) {
      // Transfer starting
      m_state = State::Downloading;
      // Try to parse file size from response like "150 Opening BINARY mode data
      // connection for file (12345 bytes)"
      QRegularExpression sizeRx("\\((\\d+)\\s*bytes?\\)");
      QRegularExpressionMatch match = sizeRx.match(line);
      if (match.hasMatch()) {
        m_fileSize = match.captured(1).toLongLong();
      }
    } else if (code == 550) {
      finishDownload(false, tr("File not found: %1").arg(m_currentJob.remotePath));
    } else {
      finishDownload(false, tr("RETR failed: %1").arg(line));
    }
    break;

  case State::Downloading:
    if (code == 226) {
      // Transfer complete
      finishDownload(true);
    }
    break;

  default:
    break;
  }
}

void DwarfFtpClient::connectToDataPort(const QString &pasvResponse) {
  // Parse PASV response: 227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)
  QRegularExpression rx("\\((\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)\\)");
  QRegularExpressionMatch match = rx.match(pasvResponse);

  if (!match.hasMatch()) {
    finishDownload(false, tr("Failed to parse PASV response"));
    return;
  }

  m_dataHost = QString("%1.%2.%3.%4")
                   .arg(match.captured(1))
                   .arg(match.captured(2))
                   .arg(match.captured(3))
                   .arg(match.captured(4));
  m_dataPort = match.captured(5).toUInt() * 256 + match.captured(6).toUInt();

  qDebug() << "[FtpClient] Data connection to" << m_dataHost << ":" << m_dataPort;
  m_dataSocket->connectToHost(m_dataHost, m_dataPort);
}

void DwarfFtpClient::onDataConnected() {
  qDebug() << "[FtpClient] Data connection established";
  m_state = State::SendingRetr;
  sendCommand(QString("RETR %1").arg(m_currentJob.remotePath));
}

void DwarfFtpClient::onDataReadyRead() {
  QByteArray data = m_dataSocket->readAll();
  m_dataBuffer.append(data);

  if (m_fileSize > 0) {
    emit downloadProgress(m_currentJob.remotePath, m_dataBuffer.size(),
                          m_fileSize);
  }
}

void DwarfFtpClient::onDataDisconnected() {
  qDebug() << "[FtpClient] Data connection closed, received"
           << m_dataBuffer.size() << "bytes";
  // Data transfer complete, wait for 226 response on control channel
}

void DwarfFtpClient::onDataError(QAbstractSocket::SocketError error) {
  if (m_state == State::Downloading && m_dataBuffer.size() > 0) {
    // Some servers close connection abruptly after transfer
    qDebug() << "[FtpClient] Data socket error (may be normal):" << error;
  } else {
    qWarning() << "[FtpClient] Data socket error:" << error;
  }
}

void DwarfFtpClient::onControlError(QAbstractSocket::SocketError error) {
  qWarning() << "[FtpClient] Control socket error:" << error
             << m_controlSocket->errorString();
  finishDownload(false, m_controlSocket->errorString());
}

void DwarfFtpClient::finishDownload(bool success, const QString &error) {
  if (m_state == State::Done || m_state == State::Idle)
    return;

  m_state = State::Done;

  // Handle delete operation
  if (m_currentJob.type == JobType::Delete) {
    qDebug() << "[FtpClient] Delete" << (success ? "successful" : "failed")
             << m_currentJob.remotePath;
    if (m_currentJob.deleteCallback) {
      m_currentJob.deleteCallback(success, error);
    }
    if (!success) {
      emit errorOccurred(m_currentJob.remotePath, error);
    }
  } else if (success && m_dataBuffer.size() > 0) {
    if (m_currentJob.type == JobType::DownloadToMemory) {
      qDebug() << "[FtpClient] Download to memory complete:"
               << m_dataBuffer.size() << "bytes";
      emit downloadDataReady(m_currentJob.remotePath, m_dataBuffer);
      if (m_currentJob.memoryCallback) {
        m_currentJob.memoryCallback(m_dataBuffer);
      }
    } else {
      // Save to file
      QFileInfo fi(m_currentJob.localPath);
      QDir dir = fi.absoluteDir();
      if (!dir.exists()) {
        dir.mkpath(".");
      }

      QFile file(m_currentJob.localPath);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(m_dataBuffer);
        file.close();
        qDebug() << "[FtpClient] Saved to" << m_currentJob.localPath;
        emit downloadFinished(m_currentJob.remotePath, m_currentJob.localPath);
      } else {
        emit errorOccurred(m_currentJob.remotePath,
                           tr("Failed to write file: %1").arg(file.errorString()));
      }
    }
  } else if (!success) {
    emit errorOccurred(m_currentJob.remotePath, error);
  }

  cleanup();

  // Process next job after a short delay to ensure sockets are fully closed
  QTimer::singleShot(200, this, &DwarfFtpClient::tryStartNextJob);
}

void DwarfFtpClient::cleanup() {
  // Disconnect and abort sockets
  m_controlSocket->abort();
  m_dataSocket->abort();
  
  // Clear buffers
  m_dataBuffer.clear();
  m_controlBuffer.clear();
  
  // Reset state
  m_state = State::Idle;
}
