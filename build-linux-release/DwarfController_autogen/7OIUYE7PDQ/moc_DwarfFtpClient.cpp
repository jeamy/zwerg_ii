/****************************************************************************
** Meta object code from reading C++ file 'DwarfFtpClient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfFtpClient.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfFtpClient.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS = QtMocHelpers::stringData(
    "DwarfFtpClient",
    "downloadStarted",
    "",
    "remotePath",
    "downloadProgress",
    "received",
    "total",
    "downloadFinished",
    "localPath",
    "downloadDataReady",
    "data",
    "errorOccurred",
    "error",
    "onControlConnected",
    "onControlReadyRead",
    "onControlError",
    "QAbstractSocket::SocketError",
    "onDataConnected",
    "onDataReadyRead",
    "onDataDisconnected",
    "onDataError"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS_t {
    uint offsetsAndSizes[42];
    char stringdata0[15];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[11];
    char stringdata4[17];
    char stringdata5[9];
    char stringdata6[6];
    char stringdata7[17];
    char stringdata8[10];
    char stringdata9[18];
    char stringdata10[5];
    char stringdata11[14];
    char stringdata12[6];
    char stringdata13[19];
    char stringdata14[19];
    char stringdata15[15];
    char stringdata16[29];
    char stringdata17[16];
    char stringdata18[16];
    char stringdata19[19];
    char stringdata20[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS_t qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS = {
    {
        QT_MOC_LITERAL(0, 14),  // "DwarfFtpClient"
        QT_MOC_LITERAL(15, 15),  // "downloadStarted"
        QT_MOC_LITERAL(31, 0),  // ""
        QT_MOC_LITERAL(32, 10),  // "remotePath"
        QT_MOC_LITERAL(43, 16),  // "downloadProgress"
        QT_MOC_LITERAL(60, 8),  // "received"
        QT_MOC_LITERAL(69, 5),  // "total"
        QT_MOC_LITERAL(75, 16),  // "downloadFinished"
        QT_MOC_LITERAL(92, 9),  // "localPath"
        QT_MOC_LITERAL(102, 17),  // "downloadDataReady"
        QT_MOC_LITERAL(120, 4),  // "data"
        QT_MOC_LITERAL(125, 13),  // "errorOccurred"
        QT_MOC_LITERAL(139, 5),  // "error"
        QT_MOC_LITERAL(145, 18),  // "onControlConnected"
        QT_MOC_LITERAL(164, 18),  // "onControlReadyRead"
        QT_MOC_LITERAL(183, 14),  // "onControlError"
        QT_MOC_LITERAL(198, 28),  // "QAbstractSocket::SocketError"
        QT_MOC_LITERAL(227, 15),  // "onDataConnected"
        QT_MOC_LITERAL(243, 15),  // "onDataReadyRead"
        QT_MOC_LITERAL(259, 18),  // "onDataDisconnected"
        QT_MOC_LITERAL(278, 11)   // "onDataError"
    },
    "DwarfFtpClient",
    "downloadStarted",
    "",
    "remotePath",
    "downloadProgress",
    "received",
    "total",
    "downloadFinished",
    "localPath",
    "downloadDataReady",
    "data",
    "errorOccurred",
    "error",
    "onControlConnected",
    "onControlReadyRead",
    "onControlError",
    "QAbstractSocket::SocketError",
    "onDataConnected",
    "onDataReadyRead",
    "onDataDisconnected",
    "onDataError"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfFtpClientENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   86,    2, 0x06,    1 /* Public */,
       4,    3,   89,    2, 0x06,    3 /* Public */,
       7,    2,   96,    2, 0x06,    7 /* Public */,
       9,    2,  101,    2, 0x06,   10 /* Public */,
      11,    2,  106,    2, 0x06,   13 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      13,    0,  111,    2, 0x08,   16 /* Private */,
      14,    0,  112,    2, 0x08,   17 /* Private */,
      15,    1,  113,    2, 0x08,   18 /* Private */,
      17,    0,  116,    2, 0x08,   20 /* Private */,
      18,    0,  117,    2, 0x08,   21 /* Private */,
      19,    0,  118,    2, 0x08,   22 /* Private */,
      20,    1,  119,    2, 0x08,   23 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,    5,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    8,
    QMetaType::Void, QMetaType::QString, QMetaType::QByteArray,    3,   10,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,   12,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 16,   12,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfFtpClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfFtpClientENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfFtpClient, std::true_type>,
        // method 'downloadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'downloadFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadDataReady'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QByteArray &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onControlConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onControlReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onControlError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QAbstractSocket::SocketError, std::false_type>,
        // method 'onDataConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDataReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDataDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onDataError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QAbstractSocket::SocketError, std::false_type>
    >,
    nullptr
} };

void DwarfFtpClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfFtpClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->downloadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->downloadProgress((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3]))); break;
        case 2: _t->downloadFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->downloadDataReady((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QByteArray>>(_a[2]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 5: _t->onControlConnected(); break;
        case 6: _t->onControlReadyRead(); break;
        case 7: _t->onControlError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        case 8: _t->onDataConnected(); break;
        case 9: _t->onDataReadyRead(); break;
        case 10: _t->onDataDisconnected(); break;
        case 11: _t->onDataError((*reinterpret_cast< std::add_pointer_t<QAbstractSocket::SocketError>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 7:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QAbstractSocket::SocketError >(); break;
            }
            break;
        case 11:
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
            using _t = void (DwarfFtpClient::*)(const QString & );
            if (_t _q_method = &DwarfFtpClient::downloadStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfFtpClient::*)(const QString & , qint64 , qint64 );
            if (_t _q_method = &DwarfFtpClient::downloadProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfFtpClient::*)(const QString & , const QString & );
            if (_t _q_method = &DwarfFtpClient::downloadFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfFtpClient::*)(const QString & , const QByteArray & );
            if (_t _q_method = &DwarfFtpClient::downloadDataReady; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfFtpClient::*)(const QString & , const QString & );
            if (_t _q_method = &DwarfFtpClient::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *DwarfFtpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfFtpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfFtpClientENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfFtpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void DwarfFtpClient::downloadStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfFtpClient::downloadProgress(const QString & _t1, qint64 _t2, qint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfFtpClient::downloadFinished(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfFtpClient::downloadDataReady(const QString & _t1, const QByteArray & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DwarfFtpClient::errorOccurred(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
