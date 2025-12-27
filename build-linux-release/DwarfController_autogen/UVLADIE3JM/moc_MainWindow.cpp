/****************************************************************************
** Meta object code from reading C++ file 'MainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "MainWindow.h"
#include <QtNetwork/QSslError>
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MainWindow.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDraggablePiPENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDraggablePiPENDCLASS = QtMocHelpers::stringData(
    "DraggablePiP",
    "doubleClicked",
    ""
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDraggablePiPENDCLASS_t {
    uint offsetsAndSizes[6];
    char stringdata0[13];
    char stringdata1[14];
    char stringdata2[1];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDraggablePiPENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDraggablePiPENDCLASS_t qt_meta_stringdata_CLASSDraggablePiPENDCLASS = {
    {
        QT_MOC_LITERAL(0, 12),  // "DraggablePiP"
        QT_MOC_LITERAL(13, 13),  // "doubleClicked"
        QT_MOC_LITERAL(27, 0)   // ""
    },
    "DraggablePiP",
    "doubleClicked",
    ""
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDraggablePiPENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   20,    2, 0x06,    1 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DraggablePiP::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSDraggablePiPENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDraggablePiPENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDraggablePiPENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DraggablePiP, std::true_type>,
        // method 'doubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DraggablePiP::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DraggablePiP *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->doubleClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DraggablePiP::*)();
            if (_t _q_method = &DraggablePiP::doubleClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *DraggablePiP::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DraggablePiP::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDraggablePiPENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DraggablePiP::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void DraggablePiP::doubleClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSMainWindowENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMainWindowENDCLASS = QtMocHelpers::stringData(
    "MainWindow",
    "onConnectClicked",
    "",
    "onCancelConnectClicked",
    "onScanClicked",
    "onCancelScanClicked",
    "onSubnetTextChanged",
    "text",
    "onWebSocketConnected",
    "onWebSocketDisconnected",
    "onWebSocketError",
    "error",
    "onDeviceFound",
    "DwarfDeviceInfo",
    "info",
    "onScanFinished",
    "onScanProgress",
    "percent",
    "onDeviceSelected",
    "QListWidgetItem*",
    "item",
    "onDisconnectClicked",
    "onCameraTeleMessage",
    "uint32_t",
    "cmd",
    "data",
    "onCameraWideMessage",
    "onPipStreamClicked",
    "onCameraSourceTele",
    "onCameraSourceWide",
    "onMotorLeftPressed",
    "onMotorLeftReleased",
    "onMotorRightPressed",
    "onMotorRightReleased",
    "onMotorUpPressed",
    "onMotorUpReleased",
    "onMotorDownPressed",
    "onMotorDownReleased",
    "onFocusMinusClicked",
    "onFocusPlusClicked",
    "onFocusAutoClicked",
    "onMotorSpeedSliderChanged",
    "value",
    "onMainViewPointClicked",
    "normalizedPos",
    "onOpenGalleryClicked",
    "onMediaListReceived",
    "document",
    "onMediaListError",
    "onDefaultParamsConfigReceived",
    "onChangeDownloadDirClicked",
    "onMediaItemClicked",
    "onMediaItemActivated",
    "onDownloadStarted",
    "fileName",
    "onDownloadFinished",
    "localPath",
    "onDownloadError",
    "onPhotoCaptureFinished",
    "DwarfCameraController::CameraKind",
    "kind",
    "success",
    "code",
    "onRecordFinished",
    "recording",
    "onStarMapOverlayRequested",
    "enabled",
    "onGalleryOverlayRequested"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMainWindowENDCLASS_t {
    uint offsetsAndSizes[136];
    char stringdata0[11];
    char stringdata1[17];
    char stringdata2[1];
    char stringdata3[23];
    char stringdata4[14];
    char stringdata5[20];
    char stringdata6[20];
    char stringdata7[5];
    char stringdata8[21];
    char stringdata9[24];
    char stringdata10[17];
    char stringdata11[6];
    char stringdata12[14];
    char stringdata13[16];
    char stringdata14[5];
    char stringdata15[15];
    char stringdata16[15];
    char stringdata17[8];
    char stringdata18[17];
    char stringdata19[17];
    char stringdata20[5];
    char stringdata21[20];
    char stringdata22[20];
    char stringdata23[9];
    char stringdata24[4];
    char stringdata25[5];
    char stringdata26[20];
    char stringdata27[19];
    char stringdata28[19];
    char stringdata29[19];
    char stringdata30[19];
    char stringdata31[20];
    char stringdata32[20];
    char stringdata33[21];
    char stringdata34[17];
    char stringdata35[18];
    char stringdata36[19];
    char stringdata37[20];
    char stringdata38[20];
    char stringdata39[19];
    char stringdata40[19];
    char stringdata41[26];
    char stringdata42[6];
    char stringdata43[23];
    char stringdata44[14];
    char stringdata45[21];
    char stringdata46[20];
    char stringdata47[9];
    char stringdata48[17];
    char stringdata49[30];
    char stringdata50[27];
    char stringdata51[19];
    char stringdata52[21];
    char stringdata53[18];
    char stringdata54[9];
    char stringdata55[19];
    char stringdata56[10];
    char stringdata57[16];
    char stringdata58[23];
    char stringdata59[34];
    char stringdata60[5];
    char stringdata61[8];
    char stringdata62[5];
    char stringdata63[17];
    char stringdata64[10];
    char stringdata65[26];
    char stringdata66[8];
    char stringdata67[26];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMainWindowENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMainWindowENDCLASS_t qt_meta_stringdata_CLASSMainWindowENDCLASS = {
    {
        QT_MOC_LITERAL(0, 10),  // "MainWindow"
        QT_MOC_LITERAL(11, 16),  // "onConnectClicked"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 22),  // "onCancelConnectClicked"
        QT_MOC_LITERAL(52, 13),  // "onScanClicked"
        QT_MOC_LITERAL(66, 19),  // "onCancelScanClicked"
        QT_MOC_LITERAL(86, 19),  // "onSubnetTextChanged"
        QT_MOC_LITERAL(106, 4),  // "text"
        QT_MOC_LITERAL(111, 20),  // "onWebSocketConnected"
        QT_MOC_LITERAL(132, 23),  // "onWebSocketDisconnected"
        QT_MOC_LITERAL(156, 16),  // "onWebSocketError"
        QT_MOC_LITERAL(173, 5),  // "error"
        QT_MOC_LITERAL(179, 13),  // "onDeviceFound"
        QT_MOC_LITERAL(193, 15),  // "DwarfDeviceInfo"
        QT_MOC_LITERAL(209, 4),  // "info"
        QT_MOC_LITERAL(214, 14),  // "onScanFinished"
        QT_MOC_LITERAL(229, 14),  // "onScanProgress"
        QT_MOC_LITERAL(244, 7),  // "percent"
        QT_MOC_LITERAL(252, 16),  // "onDeviceSelected"
        QT_MOC_LITERAL(269, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(286, 4),  // "item"
        QT_MOC_LITERAL(291, 19),  // "onDisconnectClicked"
        QT_MOC_LITERAL(311, 19),  // "onCameraTeleMessage"
        QT_MOC_LITERAL(331, 8),  // "uint32_t"
        QT_MOC_LITERAL(340, 3),  // "cmd"
        QT_MOC_LITERAL(344, 4),  // "data"
        QT_MOC_LITERAL(349, 19),  // "onCameraWideMessage"
        QT_MOC_LITERAL(369, 18),  // "onPipStreamClicked"
        QT_MOC_LITERAL(388, 18),  // "onCameraSourceTele"
        QT_MOC_LITERAL(407, 18),  // "onCameraSourceWide"
        QT_MOC_LITERAL(426, 18),  // "onMotorLeftPressed"
        QT_MOC_LITERAL(445, 19),  // "onMotorLeftReleased"
        QT_MOC_LITERAL(465, 19),  // "onMotorRightPressed"
        QT_MOC_LITERAL(485, 20),  // "onMotorRightReleased"
        QT_MOC_LITERAL(506, 16),  // "onMotorUpPressed"
        QT_MOC_LITERAL(523, 17),  // "onMotorUpReleased"
        QT_MOC_LITERAL(541, 18),  // "onMotorDownPressed"
        QT_MOC_LITERAL(560, 19),  // "onMotorDownReleased"
        QT_MOC_LITERAL(580, 19),  // "onFocusMinusClicked"
        QT_MOC_LITERAL(600, 18),  // "onFocusPlusClicked"
        QT_MOC_LITERAL(619, 18),  // "onFocusAutoClicked"
        QT_MOC_LITERAL(638, 25),  // "onMotorSpeedSliderChanged"
        QT_MOC_LITERAL(664, 5),  // "value"
        QT_MOC_LITERAL(670, 22),  // "onMainViewPointClicked"
        QT_MOC_LITERAL(693, 13),  // "normalizedPos"
        QT_MOC_LITERAL(707, 20),  // "onOpenGalleryClicked"
        QT_MOC_LITERAL(728, 19),  // "onMediaListReceived"
        QT_MOC_LITERAL(748, 8),  // "document"
        QT_MOC_LITERAL(757, 16),  // "onMediaListError"
        QT_MOC_LITERAL(774, 29),  // "onDefaultParamsConfigReceived"
        QT_MOC_LITERAL(804, 26),  // "onChangeDownloadDirClicked"
        QT_MOC_LITERAL(831, 18),  // "onMediaItemClicked"
        QT_MOC_LITERAL(850, 20),  // "onMediaItemActivated"
        QT_MOC_LITERAL(871, 17),  // "onDownloadStarted"
        QT_MOC_LITERAL(889, 8),  // "fileName"
        QT_MOC_LITERAL(898, 18),  // "onDownloadFinished"
        QT_MOC_LITERAL(917, 9),  // "localPath"
        QT_MOC_LITERAL(927, 15),  // "onDownloadError"
        QT_MOC_LITERAL(943, 22),  // "onPhotoCaptureFinished"
        QT_MOC_LITERAL(966, 33),  // "DwarfCameraController::Camera..."
        QT_MOC_LITERAL(1000, 4),  // "kind"
        QT_MOC_LITERAL(1005, 7),  // "success"
        QT_MOC_LITERAL(1013, 4),  // "code"
        QT_MOC_LITERAL(1018, 16),  // "onRecordFinished"
        QT_MOC_LITERAL(1035, 9),  // "recording"
        QT_MOC_LITERAL(1045, 25),  // "onStarMapOverlayRequested"
        QT_MOC_LITERAL(1071, 7),  // "enabled"
        QT_MOC_LITERAL(1079, 25)   // "onGalleryOverlayRequested"
    },
    "MainWindow",
    "onConnectClicked",
    "",
    "onCancelConnectClicked",
    "onScanClicked",
    "onCancelScanClicked",
    "onSubnetTextChanged",
    "text",
    "onWebSocketConnected",
    "onWebSocketDisconnected",
    "onWebSocketError",
    "error",
    "onDeviceFound",
    "DwarfDeviceInfo",
    "info",
    "onScanFinished",
    "onScanProgress",
    "percent",
    "onDeviceSelected",
    "QListWidgetItem*",
    "item",
    "onDisconnectClicked",
    "onCameraTeleMessage",
    "uint32_t",
    "cmd",
    "data",
    "onCameraWideMessage",
    "onPipStreamClicked",
    "onCameraSourceTele",
    "onCameraSourceWide",
    "onMotorLeftPressed",
    "onMotorLeftReleased",
    "onMotorRightPressed",
    "onMotorRightReleased",
    "onMotorUpPressed",
    "onMotorUpReleased",
    "onMotorDownPressed",
    "onMotorDownReleased",
    "onFocusMinusClicked",
    "onFocusPlusClicked",
    "onFocusAutoClicked",
    "onMotorSpeedSliderChanged",
    "value",
    "onMainViewPointClicked",
    "normalizedPos",
    "onOpenGalleryClicked",
    "onMediaListReceived",
    "document",
    "onMediaListError",
    "onDefaultParamsConfigReceived",
    "onChangeDownloadDirClicked",
    "onMediaItemClicked",
    "onMediaItemActivated",
    "onDownloadStarted",
    "fileName",
    "onDownloadFinished",
    "localPath",
    "onDownloadError",
    "onPhotoCaptureFinished",
    "DwarfCameraController::CameraKind",
    "kind",
    "success",
    "code",
    "onRecordFinished",
    "recording",
    "onStarMapOverlayRequested",
    "enabled",
    "onGalleryOverlayRequested"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMainWindowENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      45,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  284,    2, 0x08,    1 /* Private */,
       3,    0,  285,    2, 0x08,    2 /* Private */,
       4,    0,  286,    2, 0x08,    3 /* Private */,
       5,    0,  287,    2, 0x08,    4 /* Private */,
       6,    1,  288,    2, 0x08,    5 /* Private */,
       8,    0,  291,    2, 0x08,    7 /* Private */,
       9,    0,  292,    2, 0x08,    8 /* Private */,
      10,    1,  293,    2, 0x08,    9 /* Private */,
      12,    1,  296,    2, 0x08,   11 /* Private */,
      15,    0,  299,    2, 0x08,   13 /* Private */,
      16,    1,  300,    2, 0x08,   14 /* Private */,
      18,    1,  303,    2, 0x08,   16 /* Private */,
      21,    0,  306,    2, 0x08,   18 /* Private */,
      22,    2,  307,    2, 0x08,   19 /* Private */,
      26,    2,  312,    2, 0x08,   22 /* Private */,
      27,    0,  317,    2, 0x08,   25 /* Private */,
      28,    0,  318,    2, 0x08,   26 /* Private */,
      29,    0,  319,    2, 0x08,   27 /* Private */,
      30,    0,  320,    2, 0x08,   28 /* Private */,
      31,    0,  321,    2, 0x08,   29 /* Private */,
      32,    0,  322,    2, 0x08,   30 /* Private */,
      33,    0,  323,    2, 0x08,   31 /* Private */,
      34,    0,  324,    2, 0x08,   32 /* Private */,
      35,    0,  325,    2, 0x08,   33 /* Private */,
      36,    0,  326,    2, 0x08,   34 /* Private */,
      37,    0,  327,    2, 0x08,   35 /* Private */,
      38,    0,  328,    2, 0x08,   36 /* Private */,
      39,    0,  329,    2, 0x08,   37 /* Private */,
      40,    0,  330,    2, 0x08,   38 /* Private */,
      41,    1,  331,    2, 0x08,   39 /* Private */,
      43,    1,  334,    2, 0x08,   41 /* Private */,
      45,    0,  337,    2, 0x08,   43 /* Private */,
      46,    1,  338,    2, 0x08,   44 /* Private */,
      48,    1,  341,    2, 0x08,   46 /* Private */,
      49,    1,  344,    2, 0x08,   48 /* Private */,
      50,    0,  347,    2, 0x08,   50 /* Private */,
      51,    1,  348,    2, 0x08,   51 /* Private */,
      52,    1,  351,    2, 0x08,   53 /* Private */,
      53,    1,  354,    2, 0x08,   55 /* Private */,
      55,    2,  357,    2, 0x08,   57 /* Private */,
      57,    2,  362,    2, 0x08,   60 /* Private */,
      58,    4,  367,    2, 0x08,   63 /* Private */,
      63,    4,  376,    2, 0x08,   68 /* Private */,
      65,    1,  385,    2, 0x08,   73 /* Private */,
      67,    1,  388,    2, 0x08,   75 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, 0x80000000 | 13,   14,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 23, QMetaType::QByteArray,   24,   25,
    QMetaType::Void, 0x80000000 | 23, QMetaType::QByteArray,   24,   25,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,   42,
    QMetaType::Void, QMetaType::QPointF,   44,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QJsonDocument,   47,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void, QMetaType::QJsonDocument,   47,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void, QMetaType::QString,   54,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   54,   56,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   54,   11,
    QMetaType::Void, 0x80000000 | 59, QMetaType::Bool, QMetaType::Int, QMetaType::QString,   60,   61,   62,   54,
    QMetaType::Void, 0x80000000 | 59, QMetaType::Bool, QMetaType::Bool, QMetaType::Int,   60,   64,   61,   62,
    QMetaType::Void, QMetaType::Bool,   66,
    QMetaType::Void, QMetaType::Bool,   66,

       0        // eod
};

Q_CONSTINIT const QMetaObject MainWindow::staticMetaObject = { {
    QMetaObject::SuperData::link<QMainWindow::staticMetaObject>(),
    qt_meta_stringdata_CLASSMainWindowENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMainWindowENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMainWindowENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MainWindow, std::true_type>,
        // method 'onConnectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelConnectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onScanClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelScanClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSubnetTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onWebSocketConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWebSocketDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onWebSocketError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onDeviceFound'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DwarfDeviceInfo &, std::false_type>,
        // method 'onScanFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onScanProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onDeviceSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onDisconnectClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCameraTeleMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'onCameraWideMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'onPipStreamClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCameraSourceTele'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCameraSourceWide'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorLeftPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorLeftReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorRightPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorRightReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorUpPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorUpReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorDownPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorDownReleased'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFocusMinusClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFocusPlusClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onFocusAutoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMotorSpeedSliderChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onMainViewPointClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QPointF &, std::false_type>,
        // method 'onOpenGalleryClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMediaListReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonDocument &, std::false_type>,
        // method 'onMediaListError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onDefaultParamsConfigReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonDocument &, std::false_type>,
        // method 'onChangeDownloadDirClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMediaItemClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onMediaItemActivated'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onDownloadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onDownloadFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onDownloadError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
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
        // method 'onStarMapOverlayRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onGalleryOverlayRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>
    >,
    nullptr
} };

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MainWindow *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onConnectClicked(); break;
        case 1: _t->onCancelConnectClicked(); break;
        case 2: _t->onScanClicked(); break;
        case 3: _t->onCancelScanClicked(); break;
        case 4: _t->onSubnetTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->onWebSocketConnected(); break;
        case 6: _t->onWebSocketDisconnected(); break;
        case 7: _t->onWebSocketError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onDeviceFound((*reinterpret_cast< std::add_pointer_t<DwarfDeviceInfo>>(_a[1]))); break;
        case 9: _t->onScanFinished(); break;
        case 10: _t->onScanProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->onDeviceSelected((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 12: _t->onDisconnectClicked(); break;
        case 13: _t->onCameraTeleMessage((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 14: _t->onCameraWideMessage((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 15: _t->onPipStreamClicked(); break;
        case 16: _t->onCameraSourceTele(); break;
        case 17: _t->onCameraSourceWide(); break;
        case 18: _t->onMotorLeftPressed(); break;
        case 19: _t->onMotorLeftReleased(); break;
        case 20: _t->onMotorRightPressed(); break;
        case 21: _t->onMotorRightReleased(); break;
        case 22: _t->onMotorUpPressed(); break;
        case 23: _t->onMotorUpReleased(); break;
        case 24: _t->onMotorDownPressed(); break;
        case 25: _t->onMotorDownReleased(); break;
        case 26: _t->onFocusMinusClicked(); break;
        case 27: _t->onFocusPlusClicked(); break;
        case 28: _t->onFocusAutoClicked(); break;
        case 29: _t->onMotorSpeedSliderChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 30: _t->onMainViewPointClicked((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 31: _t->onOpenGalleryClicked(); break;
        case 32: _t->onMediaListReceived((*reinterpret_cast< std::add_pointer_t<QJsonDocument>>(_a[1]))); break;
        case 33: _t->onMediaListError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 34: _t->onDefaultParamsConfigReceived((*reinterpret_cast< std::add_pointer_t<QJsonDocument>>(_a[1]))); break;
        case 35: _t->onChangeDownloadDirClicked(); break;
        case 36: _t->onMediaItemClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 37: _t->onMediaItemActivated((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 38: _t->onDownloadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 39: _t->onDownloadFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 40: _t->onDownloadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 41: _t->onPhotoCaptureFinished((*reinterpret_cast< std::add_pointer_t<DwarfCameraController::CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[4]))); break;
        case 42: _t->onRecordFinished((*reinterpret_cast< std::add_pointer_t<DwarfCameraController::CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 43: _t->onStarMapOverlayRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 44: _t->onGalleryOverlayRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMainWindowENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 45)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 45;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 45)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 45;
    }
    return _id;
}
QT_WARNING_POP
