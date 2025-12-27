/****************************************************************************
** Meta object code from reading C++ file 'DwarfMessageDispatcher.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfMessageDispatcher.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfMessageDispatcher.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS = QtMocHelpers::stringData(
    "DwarfMessageDispatcher",
    "astroMessage",
    "",
    "std::uint32_t",
    "cmd",
    "data",
    "systemMessage",
    "rgbPowerMessage",
    "motorMessage",
    "trackMessage",
    "focusMessage",
    "notifyMessage",
    "panoramaMessage",
    "cameraTeleMessage",
    "cameraWideMessage",
    "unknownMessage",
    "moduleId",
    "dispatch"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS_t {
    uint offsetsAndSizes[36];
    char stringdata0[23];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[4];
    char stringdata5[5];
    char stringdata6[14];
    char stringdata7[16];
    char stringdata8[13];
    char stringdata9[13];
    char stringdata10[13];
    char stringdata11[14];
    char stringdata12[16];
    char stringdata13[18];
    char stringdata14[18];
    char stringdata15[15];
    char stringdata16[9];
    char stringdata17[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS_t qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS = {
    {
        QT_MOC_LITERAL(0, 22),  // "DwarfMessageDispatcher"
        QT_MOC_LITERAL(23, 12),  // "astroMessage"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 13),  // "std::uint32_t"
        QT_MOC_LITERAL(51, 3),  // "cmd"
        QT_MOC_LITERAL(55, 4),  // "data"
        QT_MOC_LITERAL(60, 13),  // "systemMessage"
        QT_MOC_LITERAL(74, 15),  // "rgbPowerMessage"
        QT_MOC_LITERAL(90, 12),  // "motorMessage"
        QT_MOC_LITERAL(103, 12),  // "trackMessage"
        QT_MOC_LITERAL(116, 12),  // "focusMessage"
        QT_MOC_LITERAL(129, 13),  // "notifyMessage"
        QT_MOC_LITERAL(143, 15),  // "panoramaMessage"
        QT_MOC_LITERAL(159, 17),  // "cameraTeleMessage"
        QT_MOC_LITERAL(177, 17),  // "cameraWideMessage"
        QT_MOC_LITERAL(195, 14),  // "unknownMessage"
        QT_MOC_LITERAL(210, 8),  // "moduleId"
        QT_MOC_LITERAL(219, 8)   // "dispatch"
    },
    "DwarfMessageDispatcher",
    "astroMessage",
    "",
    "std::uint32_t",
    "cmd",
    "data",
    "systemMessage",
    "rgbPowerMessage",
    "motorMessage",
    "trackMessage",
    "focusMessage",
    "notifyMessage",
    "panoramaMessage",
    "cameraTeleMessage",
    "cameraWideMessage",
    "unknownMessage",
    "moduleId",
    "dispatch"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfMessageDispatcherENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      11,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   86,    2, 0x06,    1 /* Public */,
       6,    2,   91,    2, 0x06,    4 /* Public */,
       7,    2,   96,    2, 0x06,    7 /* Public */,
       8,    2,  101,    2, 0x06,   10 /* Public */,
       9,    2,  106,    2, 0x06,   13 /* Public */,
      10,    2,  111,    2, 0x06,   16 /* Public */,
      11,    2,  116,    2, 0x06,   19 /* Public */,
      12,    2,  121,    2, 0x06,   22 /* Public */,
      13,    2,  126,    2, 0x06,   25 /* Public */,
      14,    2,  131,    2, 0x06,   28 /* Public */,
      15,    3,  136,    2, 0x06,   31 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      17,    3,  143,    2, 0x0a,   35 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, QMetaType::QByteArray,    4,    5,
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, QMetaType::QByteArray,   16,    4,    5,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 3, QMetaType::QByteArray,   16,    4,    5,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfMessageDispatcher::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfMessageDispatcherENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfMessageDispatcher, std::true_type>,
        // method 'astroMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'systemMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'rgbPowerMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'motorMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'trackMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'focusMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'notifyMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'panoramaMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'cameraTeleMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'cameraWideMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'unknownMessage'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'dispatch'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<std::uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>
    >,
    nullptr
} };

void DwarfMessageDispatcher::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfMessageDispatcher *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->astroMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 1: _t->systemMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 2: _t->rgbPowerMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 3: _t->motorMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 4: _t->trackMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 5: _t->focusMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 6: _t->notifyMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 7: _t->panoramaMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 8: _t->cameraTeleMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 9: _t->cameraWideMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 10: _t->unknownMessage((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[3]))); break;
        case 11: _t->dispatch((*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<std::uint32_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::astroMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::systemMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::rgbPowerMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::motorMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::trackMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::focusMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::notifyMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::panoramaMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::cameraTeleMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::cameraWideMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (DwarfMessageDispatcher::*)(std::uint32_t , std::uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfMessageDispatcher::unknownMessage; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
    }
}

const QMetaObject *DwarfMessageDispatcher::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfMessageDispatcher::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfMessageDispatcherENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfMessageDispatcher::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void DwarfMessageDispatcher::astroMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfMessageDispatcher::systemMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfMessageDispatcher::rgbPowerMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfMessageDispatcher::motorMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DwarfMessageDispatcher::trackMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void DwarfMessageDispatcher::focusMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}

// SIGNAL 6
void DwarfMessageDispatcher::notifyMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}

// SIGNAL 7
void DwarfMessageDispatcher::panoramaMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void DwarfMessageDispatcher::cameraTeleMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void DwarfMessageDispatcher::cameraWideMessage(std::uint32_t _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 9, _a);
}

// SIGNAL 10
void DwarfMessageDispatcher::unknownMessage(std::uint32_t _t1, std::uint32_t _t2, const QByteArray & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 10, _a);
}
QT_WARNING_POP
