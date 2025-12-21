#include "DwarfMtpClient.h"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>

DwarfMtpClient::DwarfMtpClient(QObject *parent)
    : QObject(parent), m_toolsAvailable(false) {
}

DwarfMtpClient::~DwarfMtpClient() {
}

bool DwarfMtpClient::isSupported() const {
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    return true;
#elif defined(Q_OS_MACOS)
    // macOS support is limited - requires libmtp
    return QFile::exists("/usr/local/bin/mtp-delfile") ||
           QFile::exists("/opt/homebrew/bin/mtp-delfile");
#else
    return false;
#endif
}

bool DwarfMtpClient::checkTools() {
#ifdef Q_OS_WIN
    // Windows has built-in MTP support via WPD/Shell
    m_toolsAvailable = true;
    emit toolCheckCompleted(true, "Windows MTP support available");
    return true;
    
#elif defined(Q_OS_LINUX)
    // Check for mtp-tools
    QString mtpDelfile = findMtpToolPath("mtp-delfile");
    
    if (mtpDelfile.isEmpty()) {
        m_toolsAvailable = false;
        QString msg = tr("MTP tools not found. Install with:\n"
                        "sudo apt-get install mtp-tools");
        emit toolCheckCompleted(false, msg);
        qWarning() << "[MTP]" << msg;
        return false;
    }
    
    m_toolsPath = QFileInfo(mtpDelfile).path();
    m_toolsAvailable = true;
    emit toolCheckCompleted(true, tr("MTP tools found: %1").arg(m_toolsPath));
    qDebug() << "[MTP] Tools available at:" << m_toolsPath;
    return true;
    
#elif defined(Q_OS_MACOS)
    // Check for libmtp via Homebrew
    QString mtpDelfile = findMtpToolPath("mtp-delfile");
    
    if (mtpDelfile.isEmpty()) {
        m_toolsAvailable = false;
        QString msg = tr("MTP tools not found. Install with:\n"
                        "brew install libmtp");
        emit toolCheckCompleted(false, msg);
        qWarning() << "[MTP]" << msg;
        return false;
    }
    
    m_toolsPath = QFileInfo(mtpDelfile).path();
    m_toolsAvailable = true;
    emit toolCheckCompleted(true, tr("MTP tools found: %1").arg(m_toolsPath));
    return true;
    
#else
    m_toolsAvailable = false;
    emit toolCheckCompleted(false, "Platform not supported");
    return false;
#endif
}

QString DwarfMtpClient::findMtpToolPath(const QString &toolName) {
    // Check common paths
    QStringList searchPaths;
    
#ifdef Q_OS_LINUX
    searchPaths << "/usr/bin" << "/usr/local/bin" << "/bin";
#elif defined(Q_OS_MACOS)
    searchPaths << "/usr/local/bin" << "/opt/homebrew/bin" << "/usr/bin";
#endif
    
    // Also check PATH
    QString pathEnv = qEnvironmentVariable("PATH");
    searchPaths.append(pathEnv.split(':', Qt::SkipEmptyParts));
    
    for (const QString &path : searchPaths) {
        QString fullPath = QDir(path).filePath(toolName);
        if (QFile::exists(fullPath)) {
            qDebug() << "[MTP] Found tool:" << fullPath;
            return fullPath;
        }
    }
    
    return QString();
}

QStringList DwarfMtpClient::listDevices() {
    QStringList devices;
    
#ifdef Q_OS_WIN
    // Use PowerShell to list MTP devices
    QProcess ps;
    ps.start("powershell", QStringList() 
        << "-Command"
        << "Get-PnpDevice -Class 'WPD' -Status 'OK' | Select-Object -ExpandProperty FriendlyName");
    
    if (ps.waitForFinished(3000)) {
        QString output = QString::fromLocal8Bit(ps.readAllStandardOutput());
        devices = output.split('\n', Qt::SkipEmptyParts);
        
        // Filter for DWARF
        devices = devices.filter(QRegularExpression("DWARF", QRegularExpression::CaseInsensitiveOption));
    }
    
#elif defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    QProcess proc;
    QString mtpDetect = findMtpToolPath("mtp-detect");
    
    if (!mtpDetect.isEmpty()) {
        proc.start(mtpDetect);
        
        if (proc.waitForFinished(5000)) {
            QString output = QString::fromUtf8(proc.readAllStandardOutput());
            
            // Parse device list
            QRegularExpression deviceRx("Friendly name: (.+)");
            QRegularExpressionMatchIterator it = deviceRx.globalMatch(output);
            
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                QString name = match.captured(1).trimmed();
                if (name.contains("DWARF", Qt::CaseInsensitive)) {
                    devices.append(name);
                }
            }
        }
    }
#endif
    
    qDebug() << "[MTP] Found devices:" << devices;
    return devices;
}

void DwarfMtpClient::deleteFile(const QString &filePath,
                                std::function<void(bool, const QString &)> callback) {
    
    if (!isSupported()) {
        callback(false, tr("MTP not supported on this platform"));
        return;
    }
    
    if (!m_toolsAvailable && !checkTools()) {
        callback(false, tr("MTP tools not available"));
        return;
    }
    
    emit deleteStarted(filePath);
    
#ifdef Q_OS_WIN
    deleteFileWindows(filePath, callback);
#elif defined(Q_OS_LINUX)
    deleteFileLinux(filePath, callback);
#elif defined(Q_OS_MACOS)
    deleteFileMacOS(filePath, callback);
#else
    callback(false, tr("Platform not supported"));
#endif
}

void DwarfMtpClient::deleteFileWindows(const QString &filePath,
                                      std::function<void(bool, const QString &)> callback) {
#ifdef Q_OS_WIN
    qDebug() << "[MTP] Windows: Deleting" << filePath;
    
    // Use PowerShell to delete via MTP
    // This is a simplified approach - full WPD API would be more robust
    
    QString psScript = QString(
        "$shell = New-Object -ComObject Shell.Application; "
        "$devices = $shell.Namespace(17).Items() | Where-Object { $_.Name -match 'DWARF' }; "
        "if ($devices) { "
        "  $device = $devices[0]; "
        "  $folder = $device.GetFolder(); "
        "  $file = $folder.Items() | Where-Object { $_.Path -match '%1' }; "
        "  if ($file) { "
        "    $folder.Remove($file); "
        "    Write-Output 'SUCCESS'; "
        "  } else { "
        "    Write-Output 'FILE_NOT_FOUND'; "
        "  } "
        "} else { "
        "  Write-Output 'DEVICE_NOT_FOUND'; "
        "}"
    ).arg(filePath);
    
    QProcess *ps = new QProcess(this);
    
    connect(ps, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ps, filePath, callback](int exitCode, QProcess::ExitStatus status) {
        Q_UNUSED(status);
        
        QString output = QString::fromLocal8Bit(ps->readAllStandardOutput()).trimmed();
        QString error = QString::fromLocal8Bit(ps->readAllStandardError());
        
        qDebug() << "[MTP] PowerShell exit code:" << exitCode;
        qDebug() << "[MTP] Output:" << output;
        if (!error.isEmpty()) {
            qDebug() << "[MTP] Error:" << error;
        }
        
        bool success = false;
        QString errorMsg;
        
        if (output.contains("SUCCESS")) {
            success = true;
        } else if (output.contains("FILE_NOT_FOUND")) {
            errorMsg = tr("File not found on device");
        } else if (output.contains("DEVICE_NOT_FOUND")) {
            errorMsg = tr("DWARF II device not found. Connect via USB.");
        } else {
            errorMsg = tr("Delete failed: %1").arg(error.isEmpty() ? output : error);
        }
        
        emit deleteFinished(success, filePath, errorMsg);
        callback(success, errorMsg);
        
        ps->deleteLater();
    });
    
    ps->start("powershell", QStringList() << "-Command" << psScript);
    
    // Timeout after 10 seconds
    QTimer::singleShot(10000, this, [ps, filePath, callback, this]() {
        if (ps->state() == QProcess::Running) {
            ps->kill();
            QString error = tr("Timeout - device not responding");
            emit deleteFinished(false, filePath, error);
            callback(false, error);
        }
    });
    
#else
    Q_UNUSED(filePath);
    callback(false, "Not on Windows");
#endif
}

void DwarfMtpClient::deleteFileLinux(const QString &filePath,
                                    std::function<void(bool, const QString &)> callback) {
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    qDebug() << "[MTP] Linux: Deleting" << filePath;
    
    // First, we need to find the file ID
    // mtp-files lists all files with their IDs
    
    QProcess *proc = new QProcess(this);
    QString mtpFiles = findMtpToolPath("mtp-files");
    
    if (mtpFiles.isEmpty()) {
        callback(false, tr("mtp-files tool not found"));
        return;
    }
    
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, filePath, callback](int exitCode, QProcess::ExitStatus status) {
        Q_UNUSED(status);
        
        QString output = QString::fromUtf8(proc->readAllStandardOutput());
        QString error = QString::fromUtf8(proc->readAllStandardError());
        
        qDebug() << "[MTP] mtp-files exit code:" << exitCode;
        
        if (exitCode != 0) {
            QString errorMsg = tr("Failed to list files: %1").arg(error);
            if (error.contains("No raw devices found")) {
                errorMsg = tr("DWARF II not connected via USB. Please connect device.");
            }
            emit deleteFinished(false, filePath, errorMsg);
            callback(false, errorMsg);
            proc->deleteLater();
            return;
        }
        
        // Parse output to find file ID
        // Format: "File ID: 12345\nFilename: IMG_001.jpg\n..."
        QString fileName = QFileInfo(filePath).fileName();
        QRegularExpression fileRx(QString("File ID: (\\d+).*Filename: %1")
                                  .arg(QRegularExpression::escape(fileName)),
                                  QRegularExpression::DotMatchesEverythingOption);
        
        QRegularExpressionMatch match = fileRx.match(output);
        
        if (!match.hasMatch()) {
            QString errorMsg = tr("File not found on device: %1").arg(fileName);
            emit deleteFinished(false, filePath, errorMsg);
            callback(false, errorMsg);
            proc->deleteLater();
            return;
        }
        
        QString fileId = match.captured(1);
        qDebug() << "[MTP] Found file ID:" << fileId << "for" << fileName;
        
        // Now delete the file
        QProcess *delProc = new QProcess(this);
        QString mtpDelfile = findMtpToolPath("mtp-delfile");
        
        connect(delProc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                this, [this, delProc, filePath, callback](int exitCode, QProcess::ExitStatus status) {
            Q_UNUSED(status);
            
            QString error = QString::fromUtf8(delProc->readAllStandardError());
            
            bool success = (exitCode == 0);
            QString errorMsg = success ? QString() : tr("Delete failed: %1").arg(error);
            
            qDebug() << "[MTP] mtp-delfile result:" << (success ? "SUCCESS" : "FAILED");
            if (!success) {
                qDebug() << "[MTP] Error:" << error;
            }
            
            emit deleteFinished(success, filePath, errorMsg);
            callback(success, errorMsg);
            
            delProc->deleteLater();
        });
        
        delProc->start(mtpDelfile, QStringList() << "-n" << fileId);
        
        // Timeout
        QTimer::singleShot(10000, this, [delProc, filePath, callback, this]() {
            if (delProc->state() == QProcess::Running) {
                delProc->kill();
                QString error = tr("Timeout during delete");
                emit deleteFinished(false, filePath, error);
                callback(false, error);
            }
        });
        
        proc->deleteLater();
    });
    
    proc->start(mtpFiles);
    
#else
    Q_UNUSED(filePath);
    callback(false, "Not on Linux");
#endif
}

void DwarfMtpClient::deleteFileMacOS(const QString &filePath,
                                    std::function<void(bool, const QString &)> callback) {
#ifdef Q_OS_MACOS
    // macOS uses same approach as Linux (libmtp tools)
    deleteFileLinux(filePath, callback);
#else
    Q_UNUSED(filePath);
    callback(false, "Not on macOS");
#endif
}
