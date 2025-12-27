/****************************************************************************
** Meta object code from reading C++ file 'DwarfCameraController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfCameraController.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfCameraController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS = QtMocHelpers::stringData(
    "DwarfCameraController",
    "errorOccurred",
    "",
    "message",
    "allParamsReceived",
    "CameraKind",
    "kind",
    "photoTaken",
    "photoCaptureFinished",
    "success",
    "code",
    "fileName",
    "recordFinished",
    "recording"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS_t {
    uint offsetsAndSizes[28];
    char stringdata0[22];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[18];
    char stringdata5[11];
    char stringdata6[5];
    char stringdata7[11];
    char stringdata8[21];
    char stringdata9[8];
    char stringdata10[5];
    char stringdata11[9];
    char stringdata12[15];
    char stringdata13[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS_t qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 21),  // "DwarfCameraController"
        QT_MOC_LITERAL(22, 13),  // "errorOccurred"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 7),  // "message"
        QT_MOC_LITERAL(45, 17),  // "allParamsReceived"
        QT_MOC_LITERAL(63, 10),  // "CameraKind"
        QT_MOC_LITERAL(74, 4),  // "kind"
        QT_MOC_LITERAL(79, 10),  // "photoTaken"
        QT_MOC_LITERAL(90, 20),  // "photoCaptureFinished"
        QT_MOC_LITERAL(111, 7),  // "success"
        QT_MOC_LITERAL(119, 4),  // "code"
        QT_MOC_LITERAL(124, 8),  // "fileName"
        QT_MOC_LITERAL(133, 14),  // "recordFinished"
        QT_MOC_LITERAL(148, 9)   // "recording"
    },
    "DwarfCameraController",
    "errorOccurred",
    "",
    "message",
    "allParamsReceived",
    "CameraKind",
    "kind",
    "photoTaken",
    "photoCaptureFinished",
    "success",
    "code",
    "fileName",
    "recordFinished",
    "recording"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfCameraControllerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   44,    2, 0x06,    1 /* Public */,
       4,    1,   47,    2, 0x06,    3 /* Public */,
       7,    1,   50,    2, 0x06,    5 /* Public */,
       8,    4,   53,    2, 0x06,    7 /* Public */,
      12,    4,   62,    2, 0x06,   12 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5,    6,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Bool, QMetaType::Int, QMetaType::QString,    6,    9,   10,   11,
    QMetaType::Void, 0x80000000 | 5, QMetaType::Bool, QMetaType::Bool, QMetaType::Int,    6,   13,    9,   10,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfCameraController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfCameraControllerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfCameraController, std::true_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'allParamsReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CameraKind, std::false_type>,
        // method 'photoTaken'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CameraKind, std::false_type>,
        // method 'photoCaptureFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CameraKind, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'recordFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<CameraKind, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void DwarfCameraController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfCameraController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->allParamsReceived((*reinterpret_cast< std::add_pointer_t<CameraKind>>(_a[1]))); break;
        case 2: _t->photoTaken((*reinterpret_cast< std::add_pointer_t<CameraKind>>(_a[1]))); break;
        case 3: _t->photoCaptureFinished((*reinterpret_cast< std::add_pointer_t<CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[4]))); break;
        case 4: _t->recordFinished((*reinterpret_cast< std::add_pointer_t<CameraKind>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[3])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[4]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfCameraController::*)(const QString & );
            if (_t _q_method = &DwarfCameraController::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfCameraController::*)(CameraKind );
            if (_t _q_method = &DwarfCameraController::allParamsReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfCameraController::*)(CameraKind );
            if (_t _q_method = &DwarfCameraController::photoTaken; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfCameraController::*)(CameraKind , bool , int , const QString & );
            if (_t _q_method = &DwarfCameraController::photoCaptureFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfCameraController::*)(CameraKind , bool , bool , int );
            if (_t _q_method = &DwarfCameraController::recordFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *DwarfCameraController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfCameraController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfCameraControllerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfCameraController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void DwarfCameraController::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfCameraController::allParamsReceived(CameraKind _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfCameraController::photoTaken(CameraKind _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfCameraController::photoCaptureFinished(CameraKind _t1, bool _t2, int _t3, const QString & _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DwarfCameraController::recordFinished(CameraKind _t1, bool _t2, bool _t3, int _t4)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t4))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
