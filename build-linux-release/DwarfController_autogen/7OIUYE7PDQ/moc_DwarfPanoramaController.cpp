/****************************************************************************
** Meta object code from reading C++ file 'DwarfPanoramaController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfPanoramaController.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfPanoramaController.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS = QtMocHelpers::stringData(
    "DwarfPanoramaController",
    "panoramaStarted",
    "",
    "rows",
    "cols",
    "panoramaProgress",
    "completed",
    "total",
    "panoramaFinished",
    "panoramaStopped",
    "panoramaFailed",
    "error"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS_t {
    uint offsetsAndSizes[24];
    char stringdata0[24];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[5];
    char stringdata4[5];
    char stringdata5[17];
    char stringdata6[10];
    char stringdata7[6];
    char stringdata8[17];
    char stringdata9[16];
    char stringdata10[15];
    char stringdata11[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS_t qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 23),  // "DwarfPanoramaController"
        QT_MOC_LITERAL(24, 15),  // "panoramaStarted"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 4),  // "rows"
        QT_MOC_LITERAL(46, 4),  // "cols"
        QT_MOC_LITERAL(51, 16),  // "panoramaProgress"
        QT_MOC_LITERAL(68, 9),  // "completed"
        QT_MOC_LITERAL(78, 5),  // "total"
        QT_MOC_LITERAL(84, 16),  // "panoramaFinished"
        QT_MOC_LITERAL(101, 15),  // "panoramaStopped"
        QT_MOC_LITERAL(117, 14),  // "panoramaFailed"
        QT_MOC_LITERAL(132, 5)   // "error"
    },
    "DwarfPanoramaController",
    "panoramaStarted",
    "",
    "rows",
    "cols",
    "panoramaProgress",
    "completed",
    "total",
    "panoramaFinished",
    "panoramaStopped",
    "panoramaFailed",
    "error"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfPanoramaControllerENDCLASS[] = {

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
       1,    2,   44,    2, 0x06,    1 /* Public */,
       5,    2,   49,    2, 0x06,    4 /* Public */,
       8,    0,   54,    2, 0x06,    7 /* Public */,
       9,    0,   55,    2, 0x06,    8 /* Public */,
      10,    1,   56,    2, 0x06,    9 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfPanoramaController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfPanoramaControllerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfPanoramaController, std::true_type>,
        // method 'panoramaStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'panoramaProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'panoramaFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'panoramaStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'panoramaFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void DwarfPanoramaController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfPanoramaController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->panoramaStarted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 1: _t->panoramaProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 2: _t->panoramaFinished(); break;
        case 3: _t->panoramaStopped(); break;
        case 4: _t->panoramaFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfPanoramaController::*)(int , int );
            if (_t _q_method = &DwarfPanoramaController::panoramaStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfPanoramaController::*)(int , int );
            if (_t _q_method = &DwarfPanoramaController::panoramaProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfPanoramaController::*)();
            if (_t _q_method = &DwarfPanoramaController::panoramaFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfPanoramaController::*)();
            if (_t _q_method = &DwarfPanoramaController::panoramaStopped; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfPanoramaController::*)(const QString & );
            if (_t _q_method = &DwarfPanoramaController::panoramaFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *DwarfPanoramaController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfPanoramaController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfPanoramaControllerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfPanoramaController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DwarfPanoramaController::panoramaStarted(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfPanoramaController::panoramaProgress(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfPanoramaController::panoramaFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DwarfPanoramaController::panoramaStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void DwarfPanoramaController::panoramaFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
