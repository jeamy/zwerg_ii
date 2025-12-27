#include "DwarfMessageDispatcher.h"

#include <QDebug>

DwarfMessageDispatcher::DwarfMessageDispatcher(QObject *parent)
    : QObject(parent) {}

void DwarfMessageDispatcher::dispatch(std::uint32_t moduleId,
                                      std::uint32_t cmd,
                                      const QByteArray &data) {
  qWarning() << "[DISPATCHER] Module:" << moduleId << "Cmd:" << cmd
             << "Size:" << data.size();

  switch (moduleId) {
  case MODULE_CAMERA_TELE:
    emit cameraTeleMessage(cmd, data);
    break;
  case MODULE_CAMERA_WIDE:
    emit cameraWideMessage(cmd, data);
    break;
  case MODULE_ASTRO:
    emit astroMessage(cmd, data);
    break;
  case MODULE_SYSTEM:
    emit systemMessage(cmd, data);
    break;
  case MODULE_RGB_POWER:
    emit rgbPowerMessage(cmd, data);
    break;
  case MODULE_MOTOR:
    emit motorMessage(cmd, data);
    break;
  case MODULE_TRACK:
    emit trackMessage(cmd, data);
    break;
  case MODULE_FOCUS:
    emit focusMessage(cmd, data);
    break;
  case MODULE_NOTIFY:
    emit notifyMessage(cmd, data);
    break;
  case MODULE_NOTIFY_EXT:
    emit notifyMessage(cmd, data);
    break;
  case MODULE_PANORAMA:
    emit panoramaMessage(cmd, data);
    break;
  case MODULE_PANORAMA_UI:
    emit panoramaUiMessage(cmd, data);
    break;
  default:
    emit unknownMessage(moduleId, cmd, data);
    break;
  }
}
