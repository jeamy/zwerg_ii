#pragma once

#include <QByteArray>
#include <QObject>
#include <cstdint>

class DwarfMessageDispatcher : public QObject {
  Q_OBJECT

public:
  explicit DwarfMessageDispatcher(QObject *parent = nullptr);

public slots:
  void dispatch(std::uint32_t moduleId, std::uint32_t cmd,
                const QByteArray &data);

signals:
  void astroMessage(std::uint32_t cmd, const QByteArray &data);
  void systemMessage(std::uint32_t cmd, const QByteArray &data);
  void rgbPowerMessage(std::uint32_t cmd, const QByteArray &data);
  void motorMessage(std::uint32_t cmd, const QByteArray &data);
  void trackMessage(std::uint32_t cmd, const QByteArray &data);
  void focusMessage(std::uint32_t cmd, const QByteArray &data);
  void notifyMessage(std::uint32_t cmd, const QByteArray &data);
  void panoramaMessage(std::uint32_t cmd, const QByteArray &data);
  void cameraTeleMessage(std::uint32_t cmd, const QByteArray &data);
  void cameraWideMessage(std::uint32_t cmd, const QByteArray &data);
  void unknownMessage(std::uint32_t moduleId, std::uint32_t cmd,
                      const QByteArray &data);

private:
  // Module IDs based on DWARF II API
  // Commands 10000-10999 = Camera Tele (Module 1)
  // Commands 11000-11999 = Astro (Module 8)
  // Commands 12000-12999 = Camera Wide (Module 2)
  // Commands 13000-13999 = System (Module 4)
  // Notifications 15000+ = Notify (Module 9)
  static constexpr std::uint32_t MODULE_CAMERA_TELE = 1;
  static constexpr std::uint32_t MODULE_CAMERA_WIDE = 2;
  static constexpr std::uint32_t MODULE_FOCUS = 3;        // Focus module
  static constexpr std::uint32_t MODULE_SYSTEM = 4;
  static constexpr std::uint32_t MODULE_RGB_POWER = 5;
  static constexpr std::uint32_t MODULE_MOTOR = 6;
  static constexpr std::uint32_t MODULE_TRACK = 7;
  static constexpr std::uint32_t MODULE_ASTRO = 8;        // Astro module (GOTO, Stacking, etc.)
  static constexpr std::uint32_t MODULE_NOTIFY = 9;
  static constexpr std::uint32_t MODULE_PANORAMA = 10;
  static constexpr std::uint32_t MODULE_NOTIFY_EXT = 15;
};
