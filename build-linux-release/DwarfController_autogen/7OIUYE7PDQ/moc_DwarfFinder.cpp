/****************************************************************************
** Meta object code from reading C++ file 'DwarfFinder.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfFinder.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfFinder.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfFinderENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfFinderENDCLASS = QtMocHelpers::stringData(
    "DwarfFinder",
    "deviceFound",
    "",
    "DwarfDeviceInfo",
    "info",
    "scanFinished",
    "scanProgress",
    "percent",
    "onSocketConnected",
    "onSocketError",
    "QAbstractSocket::SocketError",
    "socketError",
    "onDeviceInfoReceived",
    "QNetworkReply*",
    "reply"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfFinderENDCLASS_t {
    uint offsetsAndSizes[30];
    char stringdata0[12];
    char stringdata1[12];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[5];
    char stringdata5[13];
    char stringdata6[13];
    char stringdata7[8];
    char stringdata8[18];
    char stringdata9[14];
    char stringdata10[29];
    char stringdata11[12];
    char stringdata12[21];
    char stringdata13[15];
    char stringdata14[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfFinderENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfFinderENDCLASS_t qt_meta_stringdata_CLASSDwarfFinderENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "DwarfFinder"
        QT_MOC_LITERAL(12, 11),  // "deviceFound"
        QT_MOC_LITERAL(24, 0),  // ""
        QT_MOC_LITERAL(25, 15),  // "DwarfDeviceInfo"
        QT_MOC_LITERAL(41, 4),  // "info"
        QT_MOC_LITERAL(46, 12),  // "scanFinished"
        QT_MOC_LITERAL(59, 12),  // "scanProgress"
        QT_MOC_LITERAL(72, 7),  // "percent"
        QT_MOC_LITERAL(80, 17),  // "onSocketConnected"
        QT_MOC_LITERAL(98, 13),  // "onSocketError"
        QT_MOC_LITERAL(112, 28),  // "QAbstractSocket::SocketError"
        QT_MOC_LITERAL(141, 11),  // "socketError"
        QT_MOC_LITERAL(153, 20),  // "onDeviceInfoReceived"
        QT_MOC_LITERAL(174, 14),  // "QNetworkReply*"
        QT_MOC_LITERAL(189, 5)   // "reply"
    },
    "DwarfFinder",
    "deviceFound",
    "",
    "DwarfDeviceInfo",
    "info",
    "scanFinished",
    "scanProgress",
    "percent",
    "onSocketConnected",
    "onSocketError",
    "QAbstractSocket::SocketError",
    "socketError",
    "onDeviceInfoReceived",
    "QNetworkReply*",
    "reply"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfFinderENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   50,    2, 0x06,    1 /* Public */,
       5,    0,   53,    2, 0x06,    3 /* Public */,
       6,    1,   54,    2, 0x06,    4 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       8,    0,   57,    2, 0x08,    6 /* Private */,
       9,    1,   58,    2, 0x08,    7 /* Private */,
      12,    1,   61,    2, 0x08,    9 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    7,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, 0x80000000 | 13,   14,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfFinder::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfFinderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfFinderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfFinderENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfFinder, std::true_type>,
        // method 'deviceFound'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const DwarfDeviceInfo &, std::false_type>,
        // method 'scanFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'scanProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSocketConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSocketError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QAbstractSocket::SocketError, std::false_type>,
        // method 'onDeviceInfoReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QNetworkReply *, std::false_type>
    >,
    nullptr
} };

void DwarfFinder::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfFinder *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceFound((*reinterpret_cast< std::add_pointer_t<DwarfDeviceInfo>>(_a[1]))); break;
        case 1: _t->scanFinished(); break;
        case 2: _t->scanProgress((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->onSocketConnected(); break;
        case 4: _t->onSocketError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 5: _t->onDeviceInfoReceived((*reinterpret_cast< std::add_pointer_t<QNetworkReply*>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        case 5:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QNetworkReply* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfFinder::*)(const DwarfDeviceInfo & );
            if (_t _q_method = &DwarfFinder::deviceFound; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfFinder::*)();
            if (_t _q_method = &DwarfFinder::scanFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfFinder::*)(int );
            if (_t _q_method = &DwarfFinder::scanProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *DwarfFinder::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfFinder::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfFinderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfFinder::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void DwarfFinder::deviceFound(const DwarfDeviceInfo & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfFinder::scanFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DwarfFinder::scanProgress(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
