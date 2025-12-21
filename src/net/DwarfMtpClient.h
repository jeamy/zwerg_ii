#pragma once

#include <QObject>
#include <QString>
#include <QProcess>
#include <functional>

/**
 * @brief Cross-platform MTP client for deleting files on DWARF II via USB.
 * 
 * Uses platform-specific MTP tools:
 * - Windows: PowerShell with WPD/MTP
 * - Linux: mtp-tools (mtp-delfile)
 * - macOS: Limited support (requires Android File Transfer or libmtp)
 * 
 * Install requirements:
 * - Linux: sudo apt-get install mtp-tools
 * - Windows: Built-in MTP support
 * - macOS: brew install libmtp (optional)
 */
class DwarfMtpClient : public QObject {
    Q_OBJECT

public:
    explicit DwarfMtpClient(QObject *parent = nullptr);
    ~DwarfMtpClient();

    /**
     * @brief Check if MTP is supported on this platform.
     */
    bool isSupported() const;

    /**
     * @brief Check if MTP tools are available.
     * @return true if tools are installed and accessible
     */
    bool checkTools();

    /**
     * @brief Delete a file via MTP.
     * @param filePath Path on device (e.g. /DWARF_II/Normal_Photos/IMG_001.jpg)
     * @param callback Called with success status and error message
     */
    void deleteFile(const QString &filePath,
                    std::function<void(bool success, const QString &error)> callback);

    /**
     * @brief Get list of connected MTP devices.
     * @return List of device names/IDs
     */
    QStringList listDevices();

signals:
    void toolCheckCompleted(bool available, const QString &message);
    void deleteStarted(const QString &filePath);
    void deleteFinished(bool success, const QString &filePath, const QString &error);

private:
    void deleteFileWindows(const QString &filePath,
                          std::function<void(bool, const QString &)> callback);
    void deleteFileLinux(const QString &filePath,
                        std::function<void(bool, const QString &)> callback);
    void deleteFileMacOS(const QString &filePath,
                        std::function<void(bool, const QString &)> callback);

    QString findMtpToolPath(const QString &toolName);
    
    bool m_toolsAvailable;
    QString m_toolsPath;
};
