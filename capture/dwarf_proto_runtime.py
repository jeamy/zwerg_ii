#!/usr/bin/env python3
"""Laufzeit-Protobuf-Definitionen für die DWARF-II-API.

Diese Datei definiert die wichtigsten Messages (WsPacket, ComResponse, Kamera-,
Astro-, Motor- und System-Requests/Responses) dynamisch über
`google.protobuf.descriptor_pb2` und `MessageFactory`.

Es werden KEINE vorhandenen *_pb2.py-Module verwendet.
"""

from __future__ import annotations

from typing import Dict

from google.protobuf import descriptor_pb2, descriptor_pool, message_factory


def _build_file_descriptor() -> descriptor_pool.DescriptorPool:
    fd = descriptor_pb2.FileDescriptorProto()
    fd.name = "dwarf_runtime.proto"
    fd.package = "dwarf"

    # ------------------------------------------------------------------
    # base.proto: WsPacket, ComResponse
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "WsPacket"
    fields = [
        ("major_version", 1, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("minor_version", 2, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("device_id", 3, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("module_id", 4, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("cmd", 5, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("type", 6, descriptor_pb2.FieldDescriptorProto.TYPE_UINT32),
        ("data", 7, descriptor_pb2.FieldDescriptorProto.TYPE_BYTES),
        ("client_id", 8, descriptor_pb2.FieldDescriptorProto.TYPE_STRING),
    ]
    for name, num, ftype in fields:
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = ftype

    msg = fd.message_type.add()
    msg.name = "ComResponse"
    f = msg.field.add()
    f.name = "code"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    # ------------------------------------------------------------------
    # camera.proto – nur die im Capture-Skript verwendeten Messages
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqOpenCamera"
    f = msg.field.add()
    f.name = "binning"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_BOOL
    f = msg.field.add()
    f.name = "rtsp_encode_type"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqPhoto"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqBurst"
    f = msg.field.add()
    f.name = "count"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqStartRecord"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStopRecord"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqSetAllParams"
    cam_param_fields = [
        ("exp_mode", 1),
        ("exp_index", 2),
        ("gain_mode", 3),
        ("gain_index", 4),
        ("ircut_value", 5),
        ("wb_mode", 6),
        ("wb_index_type", 7),
        ("wb_index", 8),
        ("brightness", 9),
        ("contrast", 10),
        ("hue", 11),
        ("saturation", 12),
        ("sharpness", 13),
        ("jpg_quality", 14),
    ]
    for name, num in cam_param_fields:
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqGetAllParams"  # leer

    msg = fd.message_type.add()
    msg.name = "ResGetAllParams"
    for name, num in cam_param_fields:
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "code"
    f.number = 15
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqStartTimelapsePhoto"
    f = msg.field.add()
    f.name = "interval"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "count"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ResSystemWorkingState"
    f = msg.field.add()
    f.name = "code"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "state"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    # Feature Param (CommonParam + ReqSetFeatureParams)
    msg = fd.message_type.add()
    msg.name = "CommonParam"
    f = msg.field.add()
    f.name = "hasAuto"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_BOOL
    f = msg.field.add()
    f.name = "auto_mode"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "id"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "mode_index"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "index"
    f.number = 5
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "continue_value"
    f.number = 6
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE

    msg = fd.message_type.add()
    msg.name = "ReqSetFeatureParams"
    f = msg.field.add()
    f.name = "param"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE
    f.type_name = ".dwarf.CommonParam"

    # ------------------------------------------------------------------
    # focus.proto – genutzte Messages
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqNormalAutoFocus"
    f = msg.field.add()
    f.name = "mode"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "center_x"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "center_y"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    msg = fd.message_type.add()
    msg.name = "ReqAstroAutoFocus"
    f = msg.field.add()
    f.name = "mode"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    msg = fd.message_type.add()
    msg.name = "ReqManualSingleStepFocus"
    f = msg.field.add()
    f.name = "direction"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    msg = fd.message_type.add()
    msg.name = "ReqStartManualContinuFocus"
    f = msg.field.add()
    f.name = "direction"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    msg = fd.message_type.add()
    msg.name = "ReqStopManualContinuFocus"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStopAstroAutoFocus"  # leer

    # ------------------------------------------------------------------
    # astro.proto – genutzte Messages
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqStartCalibration"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStopCalibration"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqGotoDSO"
    f = msg.field.add()
    f.name = "ra"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "dec"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "target_name"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING

    msg = fd.message_type.add()
    msg.name = "ReqGotoSolarSystem"
    f = msg.field.add()
    f.name = "index"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "lon"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "lat"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "target_name"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING

    msg = fd.message_type.add()
    msg.name = "ReqStopGoto"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqGoLive"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStartEqSolving"
    f = msg.field.add()
    f.name = "lon"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "lat"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE

    msg = fd.message_type.add()
    msg.name = "ResStartEqSolving"
    f = msg.field.add()
    f.name = "azi_err"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "alt_err"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "code"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqStopEqSolving"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqCaptureRawLiveStacking"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStopCaptureRawLiveStacking"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqCaptureWideRawLiveStacking"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqStopCaptureWideRawLiveStacking"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqCheckDarkFrame"  # leer

    msg = fd.message_type.add()
    msg.name = "ResCheckDarkFrame"
    f = msg.field.add()
    f.name = "progress"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "code"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqCaptureDarkFrame"
    f = msg.field.add()
    f.name = "reshoot"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqStopCaptureDarkFrame"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqCaptureDarkFrameWithParam"
    f = msg.field.add()
    f.name = "exp_index"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "gain_index"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "bin_index"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "cap_size"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqGetDarkFrameList"  # leer

    msg = fd.message_type.add()
    msg.name = "ResGetDarkFrameInfo"
    for name, num in (("exp_index", 1), ("gain_index", 2), ("bin_index", 3)):
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ResGetDarkFrameInfoList"
    f = msg.field.add()
    f.name = "code"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "results"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE
    f.type_name = ".dwarf.ResGetDarkFrameInfo"

    msg = fd.message_type.add()
    msg.name = "ReqDelDarkFrame"
    for name, num in (("exp_index", 1), ("gain_index", 2), ("bin_index", 3)):
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqDelDarkFrameList"
    f = msg.field.add()
    f.name = "dark_list"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_REPEATED
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_MESSAGE
    f.type_name = ".dwarf.ReqDelDarkFrame"

    msg = fd.message_type.add()
    msg.name = "ResDelDarkFrameList"
    f = msg.field.add()
    f.name = "code"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqOneClickGotoDSO"
    f = msg.field.add()
    f.name = "ra"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "dec"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "target_name"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING

    msg = fd.message_type.add()
    msg.name = "ReqOneClickGotoSolarSystem"
    f = msg.field.add()
    f.name = "index"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "lon"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "lat"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "target_name"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING

    msg = fd.message_type.add()
    msg.name = "ResOneClickGoto"
    f = msg.field.add()
    f.name = "step"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "code"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "all_end"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_BOOL

    msg = fd.message_type.add()
    msg.name = "ReqStopOneClickGoto"  # leer

    # ------------------------------------------------------------------
    # motor.proto – genutzte Messages
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqMotorRun"
    f = msg.field.add()
    f.name = "id"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "speed"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE
    f = msg.field.add()
    f.name = "direction"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_BOOL
    f = msg.field.add()
    f.name = "speed_ramping"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "resolution_level"
    f.number = 5
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ResMotor"
    f = msg.field.add()
    f.name = "id"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "code"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqMotorStop"
    f = msg.field.add()
    f.name = "id"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqMotorServiceJoystick"
    for name, num in ("vector_angle", 1), ("vector_length", 2), ("speed", 3):
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_DOUBLE

    msg = fd.message_type.add()
    msg.name = "ReqMotorServiceJoystickStop"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqDualCameraLinkage"
    for name, num in ("x", 1), ("y", 2):
        f = msg.field.add()
        f.name = name
        f.number = num
        f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
        f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    # ------------------------------------------------------------------
    # system.proto (+ RGB/Power) – genutzte Messages
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqSetTime"
    f = msg.field.add()
    f.name = "timestamp"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT64

    msg = fd.message_type.add()
    msg.name = "ReqSetTimezone"
    f = msg.field.add()
    f.name = "timezone"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING

    msg = fd.message_type.add()
    msg.name = "ReqSetMtpMode"
    f = msg.field.add()
    f.name = "mode"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqSetCpuMode"
    f = msg.field.add()
    f.name = "mode"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    msg = fd.message_type.add()
    msg.name = "ReqsetMasterLock"
    f = msg.field.add()
    f.name = "lock"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_BOOL

    msg = fd.message_type.add()
    msg.name = "ReqOpenRgb"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqCloseRgb"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqOpenPowerInd"  # leer

    msg = fd.message_type.add()
    msg.name = "ReqClosePowerInd"  # leer

    # ------------------------------------------------------------------
    # panorama.proto – minimal: StartPanoramaByGrid
    # ------------------------------------------------------------------
    msg = fd.message_type.add()
    msg.name = "ReqStartPanoramaByGrid"
    f = msg.field.add()
    f.name = "rows"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "cols"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    # Motor parameters (see src/proto/panorama.proto)
    f = msg.field.add()
    f.name = "mStep1"
    f.number = 3
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "mStep2"
    f.number = 4
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "speed1"
    f.number = 5
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "speed2"
    f.number = 6
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "pulse1"
    f.number = 7
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "pulse2"
    f.number = 8
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "accelStep1"
    f.number = 9
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32
    f = msg.field.add()
    f.name = "accelStep2"
    f.number = 10
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_UINT32

    # Panorama-Notifications
    msg = fd.message_type.add()
    msg.name = "ResNotifyPanoramaProgress"
    f = msg.field.add()
    f.name = "total_count"
    f.number = 1
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
    f = msg.field.add()
    f.name = "completed_count"
    f.number = 2
    f.label = descriptor_pb2.FieldDescriptorProto.LABEL_OPTIONAL
    f.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

    # Pool & Factory
    pool = descriptor_pool.DescriptorPool()
    pool.Add(fd)
    return pool


_pool = _build_file_descriptor()


def _msg(name: str):
    # Neuere protobuf-Versionen stellen GetPrototype nicht mehr am Factory-Objekt
    # bereit; stattdessen nutzen wir die Hilfsfunktion GetMessageClass.
    desc = _pool.FindMessageTypeByName(f"dwarf.{name}")
    return message_factory.GetMessageClass(desc)


# Exponierte Message-Klassen
WsPacket = _msg("WsPacket")
ComResponse = _msg("ComResponse")

ReqOpenCamera = _msg("ReqOpenCamera")
ReqPhoto = _msg("ReqPhoto")
ReqBurst = _msg("ReqBurst")
ReqStartRecord = _msg("ReqStartRecord")
ReqStopRecord = _msg("ReqStopRecord")
ReqSetAllParams = _msg("ReqSetAllParams")
ReqGetAllParams = _msg("ReqGetAllParams")
ResGetAllParams = _msg("ResGetAllParams")
ReqStartTimelapsePhoto = _msg("ReqStartTimelapsePhoto")
ResSystemWorkingState = _msg("ResSystemWorkingState")
CommonParam = _msg("CommonParam")
ReqSetFeatureParams = _msg("ReqSetFeatureParams")

ReqNormalAutoFocus = _msg("ReqNormalAutoFocus")
ReqAstroAutoFocus = _msg("ReqAstroAutoFocus")
ReqManualSingleStepFocus = _msg("ReqManualSingleStepFocus")
ReqStartManualContinuFocus = _msg("ReqStartManualContinuFocus")
ReqStopManualContinuFocus = _msg("ReqStopManualContinuFocus")
ReqStopAstroAutoFocus = _msg("ReqStopAstroAutoFocus")

ReqStartCalibration = _msg("ReqStartCalibration")
ReqStopCalibration = _msg("ReqStopCalibration")
ReqGotoDSO = _msg("ReqGotoDSO")
ReqGotoSolarSystem = _msg("ReqGotoSolarSystem")
ReqStopGoto = _msg("ReqStopGoto")
ReqGoLive = _msg("ReqGoLive")
ReqStartEqSolving = _msg("ReqStartEqSolving")
ResStartEqSolving = _msg("ResStartEqSolving")
ReqStopEqSolving = _msg("ReqStopEqSolving")
ReqCaptureRawLiveStacking = _msg("ReqCaptureRawLiveStacking")
ReqStopCaptureRawLiveStacking = _msg("ReqStopCaptureRawLiveStacking")
ReqCaptureWideRawLiveStacking = _msg("ReqCaptureWideRawLiveStacking")
ReqStopCaptureWideRawLiveStacking = _msg("ReqStopCaptureWideRawLiveStacking")
ReqCheckDarkFrame = _msg("ReqCheckDarkFrame")
ResCheckDarkFrame = _msg("ResCheckDarkFrame")
ReqCaptureDarkFrame = _msg("ReqCaptureDarkFrame")
ReqStopCaptureDarkFrame = _msg("ReqStopCaptureDarkFrame")
ReqCaptureDarkFrameWithParam = _msg("ReqCaptureDarkFrameWithParam")
ReqGetDarkFrameList = _msg("ReqGetDarkFrameList")
ResGetDarkFrameInfo = _msg("ResGetDarkFrameInfo")
ResGetDarkFrameInfoList = _msg("ResGetDarkFrameInfoList")
ReqDelDarkFrame = _msg("ReqDelDarkFrame")
ReqDelDarkFrameList = _msg("ReqDelDarkFrameList")
ResDelDarkFrameList = _msg("ResDelDarkFrameList")
ReqOneClickGotoDSO = _msg("ReqOneClickGotoDSO")
ReqOneClickGotoSolarSystem = _msg("ReqOneClickGotoSolarSystem")
ResOneClickGoto = _msg("ResOneClickGoto")
ReqStopOneClickGoto = _msg("ReqStopOneClickGoto")

ReqMotorRun = _msg("ReqMotorRun")
ResMotor = _msg("ResMotor")
ReqMotorStop = _msg("ReqMotorStop")
ReqMotorServiceJoystick = _msg("ReqMotorServiceJoystick")
ReqMotorServiceJoystickStop = _msg("ReqMotorServiceJoystickStop")
ReqDualCameraLinkage = _msg("ReqDualCameraLinkage")

ReqSetTime = _msg("ReqSetTime")
ReqSetTimezone = _msg("ReqSetTimezone")
ReqSetMtpMode = _msg("ReqSetMtpMode")
ReqSetCpuMode = _msg("ReqSetCpuMode")
ReqsetMasterLock = _msg("ReqsetMasterLock")
ReqOpenRgb = _msg("ReqOpenRgb")
ReqCloseRgb = _msg("ReqCloseRgb")
ReqOpenPowerInd = _msg("ReqOpenPowerInd")
ReqClosePowerInd = _msg("ReqClosePowerInd")

ReqStartPanoramaByGrid = _msg("ReqStartPanoramaByGrid")
ResNotifyPanoramaProgress = _msg("ResNotifyPanoramaProgress")


__all__ = [
    # Basis
    "WsPacket",
    "ComResponse",
    # Kamera
    "ReqOpenCamera",
    "ReqPhoto",
    "ReqBurst",
    "ReqStartRecord",
    "ReqStopRecord",
    "ReqSetAllParams",
    "ReqGetAllParams",
    "ResGetAllParams",
    "ReqStartTimelapsePhoto",
    "ResSystemWorkingState",
    # Fokus
    "ReqNormalAutoFocus",
    "ReqAstroAutoFocus",
    "ReqManualSingleStepFocus",
    "ReqStartManualContinuFocus",
    "ReqStopManualContinuFocus",
    "ReqStopAstroAutoFocus",
    # Astro
    "ReqStartCalibration",
    "ReqStopCalibration",
    "ReqGotoDSO",
    "ReqGotoSolarSystem",
    "ReqStopGoto",
    "ReqGoLive",
    "ReqStartEqSolving",
    "ResStartEqSolving",
    "ReqStopEqSolving",
    "ReqCaptureRawLiveStacking",
    "ReqStopCaptureRawLiveStacking",
    "ReqCaptureWideRawLiveStacking",
    "ReqStopCaptureWideRawLiveStacking",
    "ReqCheckDarkFrame",
    "ResCheckDarkFrame",
    "ReqCaptureDarkFrame",
    "ReqStopCaptureDarkFrame",
    "ReqCaptureDarkFrameWithParam",
    "ReqGetDarkFrameList",
    "ResGetDarkFrameInfo",
    "ResGetDarkFrameInfoList",
    "ReqDelDarkFrame",
    "ReqDelDarkFrameList",
    "ResDelDarkFrameList",
    "ReqOneClickGotoDSO",
    "ReqOneClickGotoSolarSystem",
    "ResOneClickGoto",
    "ReqStopOneClickGoto",
    # Motor
    "ReqMotorRun",
    "ResMotor",
    "ReqMotorStop",
    "ReqMotorServiceJoystick",
    "ReqMotorServiceJoystickStop",
    "ReqDualCameraLinkage",
    # System/RGB/Power
    "ReqSetTime",
    "ReqSetTimezone",
    "ReqSetMtpMode",
    "ReqSetCpuMode",
    "ReqsetMasterLock",
    "ReqOpenRgb",
    "ReqCloseRgb",
    "ReqOpenPowerInd",
    "ReqClosePowerInd",
    # Panorama
    "ReqStartPanoramaByGrid",
    "ResNotifyPanoramaProgress",
]
