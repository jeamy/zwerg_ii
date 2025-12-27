/****************************************************************************
** Meta object code from reading C++ file 'DwarfWebSocketClient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfWebSocketClient.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfWebSocketClient.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS = QtMocHelpers::stringData(
    "DwarfWebSocketClient",
    "connected",
    "",
    "disconnected",
    "messageReceived",
    "uint32_t",
    "moduleId",
    "cmd",
    "data",
    "errorOccurred",
    "error",
    "onConnected",
    "onDisconnected",
    "onBinaryMessageReceived",
    "message",
    "onTextMessageReceived",
    "onError",
    "QAbstractSocket::SocketError",
    "sendPing"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS_t {
    uint offsetsAndSizes[38];
    char stringdata0[21];
    char stringdata1[10];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[16];
    char stringdata5[9];
    char stringdata6[9];
    char stringdata7[4];
    char stringdata8[5];
    char stringdata9[14];
    char stringdata10[6];
    char stringdata11[12];
    char stringdata12[15];
    char stringdata13[24];
    char stringdata14[8];
    char stringdata15[22];
    char stringdata16[8];
    char stringdata17[29];
    char stringdata18[9];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS_t qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20),  // "DwarfWebSocketClient"
        QT_MOC_LITERAL(21, 9),  // "connected"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 12),  // "disconnected"
        QT_MOC_LITERAL(45, 15),  // "messageReceived"
        QT_MOC_LITERAL(61, 8),  // "uint32_t"
        QT_MOC_LITERAL(70, 8),  // "moduleId"
        QT_MOC_LITERAL(79, 3),  // "cmd"
        QT_MOC_LITERAL(83, 4),  // "data"
        QT_MOC_LITERAL(88, 13),  // "errorOccurred"
        QT_MOC_LITERAL(102, 5),  // "error"
        QT_MOC_LITERAL(108, 11),  // "onConnected"
        QT_MOC_LITERAL(120, 14),  // "onDisconnected"
        QT_MOC_LITERAL(135, 23),  // "onBinaryMessageReceived"
        QT_MOC_LITERAL(159, 7),  // "message"
        QT_MOC_LITERAL(167, 21),  // "onTextMessageReceived"
        QT_MOC_LITERAL(189, 7),  // "onError"
        QT_MOC_LITERAL(197, 28),  // "QAbstractSocket::SocketError"
        QT_MOC_LITERAL(226, 8)   // "sendPing"
    },
    "DwarfWebSocketClient",
    "connected",
    "",
    "disconnected",
    "messageReceived",
    "uint32_t",
    "moduleId",
    "cmd",
    "data",
    "errorOccurred",
    "error",
    "onConnected",
    "onDisconnected",
    "onBinaryMessageReceived",
    "message",
    "onTextMessageReceived",
    "onError",
    "QAbstractSocket::SocketError",
    "sendPing"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfWebSocketClientENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,
       3,    0,   75,    2, 0x06,    2 /* Public */,
       4,    3,   76,    2, 0x06,    3 /* Public */,
       9,    1,   83,    2, 0x06,    7 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    0,   86,    2, 0x08,    9 /* Private */,
      12,    0,   87,    2, 0x08,   10 /* Private */,
      13,    1,   88,    2, 0x08,   11 /* Private */,
      15,    1,   91,    2, 0x08,   13 /* Private */,
      16,    1,   94,    2, 0x08,   15 /* Private */,
      18,    0,   97,    2, 0x08,   17 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5, QMetaType::QByteArray,    6,    7,    8,
    QMetaType::Void, QMetaType::QString,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QByteArray,   14,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void, 0x80000000 | 17,   10,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfWebSocketClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfWebSocketClientENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfWebSocketClient, std::true_type>,
        // method 'connected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'disconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'messageReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<uint32_t, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onBinaryMessageReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'onTextMessageReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QAbstractSocket::SocketError, std::false_type>,
        // method 'sendPing'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DwarfWebSocketClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfWebSocketClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disconnected(); break;
        case 2: _t->messageReceived((*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<uint32_t>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[3]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->onConnected(); break;
        case 5: _t->onDisconnected(); break;
        case 6: _t->onBinaryMessageReceived((*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[1]))); break;
        case 7: _t->onTextMessageReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->onError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 9: _t->sendPing(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 8:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfWebSocketClient::*)();
            if (_t _q_method = &DwarfWebSocketClient::connected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfWebSocketClient::*)();
            if (_t _q_method = &DwarfWebSocketClient::disconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfWebSocketClient::*)(uint32_t , uint32_t , const QByteArray & );
            if (_t _q_method = &DwarfWebSocketClient::messageReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfWebSocketClient::*)(const QString & );
            if (_t _q_method = &DwarfWebSocketClient::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *DwarfWebSocketClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfWebSocketClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfWebSocketClientENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfWebSocketClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void DwarfWebSocketClient::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void DwarfWebSocketClient::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void DwarfWebSocketClient::messageReceived(uint32_t _t1, uint32_t _t2, const QByteArray & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfWebSocketClient::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
