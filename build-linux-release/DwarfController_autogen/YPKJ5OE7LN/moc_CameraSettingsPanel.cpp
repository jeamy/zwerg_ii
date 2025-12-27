/****************************************************************************
** Meta object code from reading C++ file 'CameraSettingsPanel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/CameraSettingsPanel.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'CameraSettingsPanel.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS = QtMocHelpers::stringData(
    "CameraSettingsPanel",
    "photoRequested",
    "",
    "recordRequested",
    "recording",
    "cameraModeChanged",
    "CameraMode",
    "mode",
    "onTeleClicked",
    "onWideClicked",
    "onPhotoClicked",
    "onRecordClicked",
    "onExposureModeChanged",
    "index",
    "onExposureSliderChanged",
    "value",
    "onGainModeChanged",
    "onGainSliderChanged",
    "onIrCutToggled",
    "checked",
    "onBrightnessChanged",
    "onContrastChanged",
    "onSaturationChanged",
    "onSharpnessChanged",
    "onHueChanged",
    "onWbModeChanged",
    "onWbTemperatureChanged",
    "onPhotoCaptureFinished",
    "DwarfCameraController::CameraKind",
    "kind",
    "success",
    "code",
    "fileName",
    "onRecordFinished",
    "onRecordTimerTick"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS_t {
    uint offsetsAndSizes[70];
    char stringdata0[20];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[10];
    char stringdata5[18];
    char stringdata6[11];
    char stringdata7[5];
    char stringdata8[14];
    char stringdata9[14];
    char stringdata10[15];
    char stringdata11[16];
    char stringdata12[22];
    char stringdata13[6];
    char stringdata14[24];
    char stringdata15[6];
    char stringdata16[18];
    char stringdata17[20];
    char stringdata18[15];
    char stringdata19[8];
    char stringdata20[20];
    char stringdata21[18];
    char stringdata22[20];
    char stringdata23[19];
    char stringdata24[13];
    char stringdata25[16];
    char stringdata26[23];
    char stringdata27[23];
    char stringdata28[34];
    char stringdata29[5];
    char stringdata30[8];
    char stringdata31[5];
    char stringdata32[9];
    char stringdata33[17];
    char stringdata34[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS_t qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS = {
    {
        QT_MOC_LITERAL(0, 19),  // "CameraSettingsPanel"
        QT_MOC_LITERAL(20, 14),  // "photoRequested"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 15),  // "recordRequested"
        QT_MOC_LITERAL(52, 9),  // "recording"
        QT_MOC_LITERAL(62, 17),  // "cameraModeChanged"
        QT_MOC_LITERAL(80, 10),  // "CameraMode"
        QT_MOC_LITERAL(91, 4),  // "mode"
        QT_MOC_LITERAL(96, 13),  // "onTeleClicked"
        QT_MOC_LITERAL(110, 13),  // "onWideClicked"
        QT_MOC_LITERAL(124, 14),  // "onPhotoClicked"
        QT_MOC_LITERAL(139, 15),  // "onRecordClicked"
        QT_MOC_LITERAL(155, 21),  // "onExposureModeChanged"
        QT_MOC_LITERAL(177, 5),  // "index"
        QT_MOC_LITERAL(183, 23),  // "onExposureSliderChanged"
        QT_MOC_LITERAL(207, 5),  // "value"
        QT_MOC_LITERAL(213, 17),  // "onGainModeChanged"
        QT_MOC_LITERAL(231, 19),  // "onGainSliderChanged"
        QT_MOC_LITERAL(251, 14),  // "onIrCutToggled"
        QT_MOC_LITERAL(266, 7),  // "checked"
        QT_MOC_LITERAL(274, 19),  // "onBrightnessChanged"
        QT_MOC_LITERAL(294, 17),  // "onContrastChanged"
        QT_MOC_LITERAL(312, 19),  // "onSaturationChanged"
        QT_MOC_LITERAL(332, 18),  // "onSharpnessChanged"
        QT_MOC_LITERAL(351, 12),  // "onHueChanged"
        QT_MOC_LITERAL(364, 15),  // "onWbModeChanged"
        QT_MOC_LITERAL(380, 22),  // "onWbTemperatureChanged"
        QT_MOC_LITERAL(403, 22),  // "onPhotoCaptureFinished"
        QT_MOC_LITERAL(426, 33),  // "DwarfCameraController::Camera..."
        QT_MOC_LITERAL(460, 4),  // "kind"
        QT_MOC_LITERAL(465, 7),  // "success"
        QT_MOC_LITERAL(473, 4),  // "code"
        QT_MOC_LITERAL(478, 8),  // "fileName"
        QT_MOC_LITERAL(487, 16),  // "onRecordFinished"
        QT_MOC_LITERAL(504, 17)   // "onRecordTimerTick"
    },
    "CameraSettingsPanel",
    "photoRequested",
    "",
    "recordRequested",
    "recording",
    "cameraModeChanged",
    "CameraMode",
    "mode",
    "onTeleClicked",
    "onWideClicked",
    "onPhotoClicked",
    "onRecordClicked",
    "onExposureModeChanged",
    "index",
    "onExposureSliderChanged",
    "value",
    "onGainModeChanged",
    "onGainSliderChanged",
    "onIrCutToggled",
    "checked",
    "onBrightnessChanged",
    "onContrastChanged",
    "onSaturationChanged",
    "onSharpnessChanged",
    "onHueChanged",
    "onWbModeChanged",
    "onWbTemperatureChanged",
    "onPhotoCaptureFinished",
    "DwarfCameraController::CameraKind",
    "kind",
    "success",
    "code",
    "fileName",
    "onRecordFinished",
    "onRecordTimerTick"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSCameraSettingsPanelENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      22,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  146,    2, 0x06,    1 /* Public */,
       3,    1,  147,    2, 0x06,    2 /* Public */,
       5,    1,  150,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    0,  153,    2, 0x08,    6 /* Private */,
       9,    0,  154,    2, 0x08,    7 /* Private */,
      10,    0,  155,    2, 0x08,    8 /* Private */,
      11,    0,  156,    2, 0x08,    9 /* Private */,
      12,    1,  157,    2, 0x08,   10 /* Private */,
      14,    1,  160,    2, 0x08,   12 /* Private */,
      16,    1,  163,    2, 0x08,   14 /* Private */,
      17,    1,  166,    2, 0x08,   16 /* Private */,
      18,    1,  169,    2, 0x08,   18 /* Private */,
      20,    1,  172,    2, 0x08,   20 /* Private */,
      21,    1,  175,    2, 0x08,   22 /* Private */,
      22,    1,  178,    2, 0x08,   24 /* Private */,
      23,    1,  181,    2, 0x08,   26 /* Private */,
      24,    1,  184,    2, 0x08,   28 /* Private */,
      25,    1,  187,    2, 0x08,   30 /* Private */,
      26,    1,  190,    2, 0x08,   32 /* Private */,
      27,    4,  193,    2, 0x08,   34 /* Private */,
      33,    4,  202,    2, 0x08,   39 /* Private */,
      34,    0,  211,    2, 0x08,   44 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    4,
    QMetaType::Void, 0x80000000 | 6,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Bool,   19,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   15,
    QMetaType::Void, 0x80000000 | 28, QMetaType::Bool, QMetaType::Int, QMetaType::QString,   29,   30,   31,   32,
    QMetaType::Void, 0x80000000 | 28, QMetaType::Bool, QMetaType::Bool, QMetaType::Int,   29,    4,   30,   31,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject CameraSettingsPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSCameraSettingsPanelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<CameraSettingsPanel, std::true_type>,
        // method 'photoRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'recordRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'cameraModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CameraMode, std::false_type>,
        // method 'onTeleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWideClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onPhotoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onRecordClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onExposureModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onExposureSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onGainModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onGainSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onIrCutToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onBrightnessChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onContrastChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSaturationChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSharpnessChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onHueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onWbModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onWbTemperatureChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onPhotoCaptureFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DwarfCameraController::CameraKind, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onRecordFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<DwarfCameraController::CameraKind, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRecordTimerTick'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void CameraSettingsPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<CameraSettingsPanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->photoRequested(); break;
        case 1: _t->recordRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 2: _t->cameraModeChanged((*reinterpret_cast< std::add_pointer_t<CameraMode>>(_a[1]))); break;
        case 3: _t->onTeleClicked(); break;
        case 4: _t->onWideClicked(); break;
        case 5: _t->onPhotoClicked(); break;
        case 6: _t->onRecordClicked(); break;
        case 7: _t->onExposureModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 8: _t->onExposureSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->onGainModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 10: _t->onGainSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onIrCutToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 12: _t->onBrightnessChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 13: _t->onContrastChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 14: _t->onSaturationChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 15: _t->onSharpnessChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 16: _t->onHueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->onWbModeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 18: _t->onWbTemperatureChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 19: _t->onPhotoCaptureFinished((*reinterpret_cast< std::add_pointer_t<DwarfCameraController::CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[4]))); break;
        case 20: _t->onRecordFinished((*reinterpret_cast< std::add_pointer_t<DwarfCameraController::CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 21: _t->onRecordTimerTick(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (CameraSettingsPanel::*)();
            if (_t _q_method = &CameraSettingsPanel::photoRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (CameraSettingsPanel::*)(bool );
            if (_t _q_method = &CameraSettingsPanel::recordRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (CameraSettingsPanel::*)(CameraMode );
            if (_t _q_method = &CameraSettingsPanel::cameraModeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *CameraSettingsPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *CameraSettingsPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSCameraSettingsPanelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int CameraSettingsPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 22)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 22;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 22)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 22;
    }
    return _id;
}

// SIGNAL 0
void CameraSettingsPanel::photoRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void CameraSettingsPanel::recordRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void CameraSettingsPanel::cameraModeChanged(CameraMode _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
