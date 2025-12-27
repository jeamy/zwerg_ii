/****************************************************************************
** Meta object code from reading C++ file 'DwarfAstroController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfAstroController.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfAstroController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS = QtMocHelpers::stringData(
    "DwarfAstroController",
    "calibrationStarted",
    "",
    "calibrationProgress",
    "progress",
    "calibrationCompleted",
    "success",
    "calibrationFailed",
    "error",
    "gotoStarted",
    "targetName",
    "gotoProgress",
    "step",
    "gotoCompleted",
    "gotoFailed",
    "stackingStarted",
    "stackingProgress",
    "currentFrame",
    "totalFrames",
    "stackedFrames",
    "rejectedFrames",
    "stackingStateChanged",
    "state",
    "stackingStopped",
    "stackingFailed",
    "darkFrameProgress",
    "current",
    "total",
    "darkFrameListReceived",
    "QList<QVariantMap>",
    "frames",
    "eqSolvingResult",
    "aziError",
    "altError",
    "batteryChanged",
    "percent",
    "temperatureChanged",
    "celsius"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS_t {
    uint offsetsAndSizes[76];
    char stringdata0[21];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[20];
    char stringdata4[9];
    char stringdata5[21];
    char stringdata6[8];
    char stringdata7[18];
    char stringdata8[6];
    char stringdata9[12];
    char stringdata10[11];
    char stringdata11[13];
    char stringdata12[5];
    char stringdata13[14];
    char stringdata14[11];
    char stringdata15[16];
    char stringdata16[17];
    char stringdata17[13];
    char stringdata18[12];
    char stringdata19[14];
    char stringdata20[15];
    char stringdata21[21];
    char stringdata22[6];
    char stringdata23[16];
    char stringdata24[15];
    char stringdata25[18];
    char stringdata26[8];
    char stringdata27[6];
    char stringdata28[22];
    char stringdata29[19];
    char stringdata30[7];
    char stringdata31[16];
    char stringdata32[9];
    char stringdata33[9];
    char stringdata34[15];
    char stringdata35[8];
    char stringdata36[19];
    char stringdata37[8];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS_t qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20),  // "DwarfAstroController"
        QT_MOC_LITERAL(21, 18),  // "calibrationStarted"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 19),  // "calibrationProgress"
        QT_MOC_LITERAL(61, 8),  // "progress"
        QT_MOC_LITERAL(70, 20),  // "calibrationCompleted"
        QT_MOC_LITERAL(91, 7),  // "success"
        QT_MOC_LITERAL(99, 17),  // "calibrationFailed"
        QT_MOC_LITERAL(117, 5),  // "error"
        QT_MOC_LITERAL(123, 11),  // "gotoStarted"
        QT_MOC_LITERAL(135, 10),  // "targetName"
        QT_MOC_LITERAL(146, 12),  // "gotoProgress"
        QT_MOC_LITERAL(159, 4),  // "step"
        QT_MOC_LITERAL(164, 13),  // "gotoCompleted"
        QT_MOC_LITERAL(178, 10),  // "gotoFailed"
        QT_MOC_LITERAL(189, 15),  // "stackingStarted"
        QT_MOC_LITERAL(205, 16),  // "stackingProgress"
        QT_MOC_LITERAL(222, 12),  // "currentFrame"
        QT_MOC_LITERAL(235, 11),  // "totalFrames"
        QT_MOC_LITERAL(247, 13),  // "stackedFrames"
        QT_MOC_LITERAL(261, 14),  // "rejectedFrames"
        QT_MOC_LITERAL(276, 20),  // "stackingStateChanged"
        QT_MOC_LITERAL(297, 5),  // "state"
        QT_MOC_LITERAL(303, 15),  // "stackingStopped"
        QT_MOC_LITERAL(319, 14),  // "stackingFailed"
        QT_MOC_LITERAL(334, 17),  // "darkFrameProgress"
        QT_MOC_LITERAL(352, 7),  // "current"
        QT_MOC_LITERAL(360, 5),  // "total"
        QT_MOC_LITERAL(366, 21),  // "darkFrameListReceived"
        QT_MOC_LITERAL(388, 18),  // "QList<QVariantMap>"
        QT_MOC_LITERAL(407, 6),  // "frames"
        QT_MOC_LITERAL(414, 15),  // "eqSolvingResult"
        QT_MOC_LITERAL(430, 8),  // "aziError"
        QT_MOC_LITERAL(439, 8),  // "altError"
        QT_MOC_LITERAL(448, 14),  // "batteryChanged"
        QT_MOC_LITERAL(463, 7),  // "percent"
        QT_MOC_LITERAL(471, 18),  // "temperatureChanged"
        QT_MOC_LITERAL(490, 7)   // "celsius"
    },
    "DwarfAstroController",
    "calibrationStarted",
    "",
    "calibrationProgress",
    "progress",
    "calibrationCompleted",
    "success",
    "calibrationFailed",
    "error",
    "gotoStarted",
    "targetName",
    "gotoProgress",
    "step",
    "gotoCompleted",
    "gotoFailed",
    "stackingStarted",
    "stackingProgress",
    "currentFrame",
    "totalFrames",
    "stackedFrames",
    "rejectedFrames",
    "stackingStateChanged",
    "state",
    "stackingStopped",
    "stackingFailed",
    "darkFrameProgress",
    "current",
    "total",
    "darkFrameListReceived",
    "QList<QVariantMap>",
    "frames",
    "eqSolvingResult",
    "aziError",
    "altError",
    "batteryChanged",
    "percent",
    "temperatureChanged",
    "celsius"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfAstroControllerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      18,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  122,    2, 0x06,    1 /* Public */,
       3,    1,  123,    2, 0x06,    2 /* Public */,
       5,    1,  126,    2, 0x06,    4 /* Public */,
       7,    1,  129,    2, 0x06,    6 /* Public */,
       9,    1,  132,    2, 0x06,    8 /* Public */,
      11,    1,  135,    2, 0x06,   10 /* Public */,
      13,    0,  138,    2, 0x06,   12 /* Public */,
      14,    1,  139,    2, 0x06,   13 /* Public */,
      15,    0,  142,    2, 0x06,   15 /* Public */,
      16,    4,  143,    2, 0x06,   16 /* Public */,
      21,    1,  152,    2, 0x06,   21 /* Public */,
      23,    0,  155,    2, 0x06,   23 /* Public */,
      24,    1,  156,    2, 0x06,   24 /* Public */,
      25,    2,  159,    2, 0x06,   26 /* Public */,
      28,    1,  164,    2, 0x06,   29 /* Public */,
      31,    2,  167,    2, 0x06,   31 /* Public */,
      34,    1,  172,    2, 0x06,   34 /* Public */,
      36,    1,  175,    2, 0x06,   36 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::Int,   12,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int, QMetaType::Int, QMetaType::Int,   17,   18,   19,   20,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,   26,   27,
    QMetaType::Void, 0x80000000 | 29,   30,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   32,   33,
    QMetaType::Void, QMetaType::Int,   35,
    QMetaType::Void, QMetaType::Int,   37,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfAstroController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfAstroControllerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfAstroController, std::true_type>,
        // method 'calibrationStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'calibrationProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'calibrationCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'calibrationFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'gotoStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'gotoProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'gotoCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'gotoFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'stackingStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stackingProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'stackingStateChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'stackingStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'stackingFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'darkFrameProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'darkFrameListReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QList<QVariantMap> &, std::false_type>,
        // method 'eqSolvingResult'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'batteryChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'temperatureChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void DwarfAstroController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfAstroController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->calibrationStarted(); break;
        case 1: _t->calibrationProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->calibrationCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 3: _t->calibrationFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->gotoStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->gotoProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->gotoCompleted(); break;
        case 7: _t->gotoFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->stackingStarted(); break;
        case 9: _t->stackingProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        case 10: _t->stackingStateChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 11: _t->stackingStopped(); break;
        case 12: _t->stackingFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 13: _t->darkFrameProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 14: _t->darkFrameListReceived((*reinterpret_cast< std::add_pointer_t<QList<QVariantMap>>>(_a[1]))); break;
        case 15: _t->eqSolvingResult((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 16: _t->batteryChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->temperatureChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 14:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<QVariantMap> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfAstroController::*)();
            if (_t _q_method = &DwarfAstroController::calibrationStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int );
            if (_t _q_method = &DwarfAstroController::calibrationProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(bool );
            if (_t _q_method = &DwarfAstroController::calibrationCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(const QString & );
            if (_t _q_method = &DwarfAstroController::calibrationFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(const QString & );
            if (_t _q_method = &DwarfAstroController::gotoStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int );
            if (_t _q_method = &DwarfAstroController::gotoProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)();
            if (_t _q_method = &DwarfAstroController::gotoCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(const QString & );
            if (_t _q_method = &DwarfAstroController::gotoFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)();
            if (_t _q_method = &DwarfAstroController::stackingStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int , int , int , int );
            if (_t _q_method = &DwarfAstroController::stackingProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int );
            if (_t _q_method = &DwarfAstroController::stackingStateChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)();
            if (_t _q_method = &DwarfAstroController::stackingStopped; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(const QString & );
            if (_t _q_method = &DwarfAstroController::stackingFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int , int );
            if (_t _q_method = &DwarfAstroController::darkFrameProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(const QList<QVariantMap> & );
            if (_t _q_method = &DwarfAstroController::darkFrameListReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 14;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(double , double );
            if (_t _q_method = &DwarfAstroController::eqSolvingResult; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 15;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int );
            if (_t _q_method = &DwarfAstroController::batteryChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 16;
                return;
            }
        }
        {
            using _t = void (DwarfAstroController::*)(int );
            if (_t _q_method = &DwarfAstroController::temperatureChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 17;
                return;
            }
        }
    }
}

const QMetaObject *DwarfAstroController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfAstroController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfAstroControllerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfAstroController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 18)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 18;
    }
    return _id;
}

// SIGNAL 0
void DwarfAstroController::calibrationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DwarfAstroController::calibrationProgress(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfAstroController::calibrationCompleted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfAstroController::calibrationFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DwarfAstroController::gotoStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DwarfAstroController::gotoProgress(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DwarfAstroController::gotoCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void DwarfAstroController::gotoFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void DwarfAstroController::stackingStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void DwarfAstroController::stackingProgress(int _t1, int _t2, int _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void DwarfAstroController::stackingStateChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}

// SIGNAL 11
void DwarfAstroController::stackingStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void DwarfAstroController::stackingFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 12, _a);
}

// SIGNAL 13
void DwarfAstroController::darkFrameProgress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}

// SIGNAL 14
void DwarfAstroController::darkFrameListReceived(const QList<QVariantMap> & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 14, _a);
}

// SIGNAL 15
void DwarfAstroController::eqSolvingResult(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 15, _a);
}

// SIGNAL 16
void DwarfAstroController::batteryChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 16, _a);
}

// SIGNAL 17
void DwarfAstroController::temperatureChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 17, _a);
}
QT_WARNING_POP
