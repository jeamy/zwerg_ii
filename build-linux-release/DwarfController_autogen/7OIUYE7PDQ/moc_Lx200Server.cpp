/****************************************************************************
** Meta object code from reading C++ file 'Lx200Server.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/Lx200Server.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Lx200Server.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSLx200ServerENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSLx200ServerENDCLASS = QtMocHelpers::stringData(
    "Lx200Server",
    "runningChanged",
    "",
    "running",
    "clientConnected",
    "QHostAddress",
    "peerAddress",
    "peerPort",
    "clientDisconnected",
    "errorOccurred",
    "error",
    "gotoRequested",
    "raDeg",
    "decDeg",
    "stopRequested",
    "syncRequested",
    "onNewConnection",
    "onClientReadyRead",
    "onClientDisconnected"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSLx200ServerENDCLASS_t {
    uint offsetsAndSizes[38];
    char stringdata0[12];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[8];
    char stringdata4[16];
    char stringdata5[13];
    char stringdata6[12];
    char stringdata7[9];
    char stringdata8[19];
    char stringdata9[14];
    char stringdata10[6];
    char stringdata11[14];
    char stringdata12[6];
    char stringdata13[7];
    char stringdata14[14];
    char stringdata15[14];
    char stringdata16[16];
    char stringdata17[18];
    char stringdata18[21];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSLx200ServerENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSLx200ServerENDCLASS_t qt_meta_stringdata_CLASSLx200ServerENDCLASS = {
    {
        QT_MOC_LITERAL(0, 11),  // "Lx200Server"
        QT_MOC_LITERAL(12, 14),  // "runningChanged"
        QT_MOC_LITERAL(27, 0),  // ""
        QT_MOC_LITERAL(28, 7),  // "running"
        QT_MOC_LITERAL(36, 15),  // "clientConnected"
        QT_MOC_LITERAL(52, 12),  // "QHostAddress"
        QT_MOC_LITERAL(65, 11),  // "peerAddress"
        QT_MOC_LITERAL(77, 8),  // "peerPort"
        QT_MOC_LITERAL(86, 18),  // "clientDisconnected"
        QT_MOC_LITERAL(105, 13),  // "errorOccurred"
        QT_MOC_LITERAL(119, 5),  // "error"
        QT_MOC_LITERAL(125, 13),  // "gotoRequested"
        QT_MOC_LITERAL(139, 5),  // "raDeg"
        QT_MOC_LITERAL(145, 6),  // "decDeg"
        QT_MOC_LITERAL(152, 13),  // "stopRequested"
        QT_MOC_LITERAL(166, 13),  // "syncRequested"
        QT_MOC_LITERAL(180, 15),  // "onNewConnection"
        QT_MOC_LITERAL(196, 17),  // "onClientReadyRead"
        QT_MOC_LITERAL(214, 20)   // "onClientDisconnected"
    },
    "Lx200Server",
    "runningChanged",
    "",
    "running",
    "clientConnected",
    "QHostAddress",
    "peerAddress",
    "peerPort",
    "clientDisconnected",
    "errorOccurred",
    "error",
    "gotoRequested",
    "raDeg",
    "decDeg",
    "stopRequested",
    "syncRequested",
    "onNewConnection",
    "onClientReadyRead",
    "onClientDisconnected"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSLx200ServerENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   74,    2, 0x06,    1 /* Public */,
       4,    2,   77,    2, 0x06,    3 /* Public */,
       8,    2,   82,    2, 0x06,    6 /* Public */,
       9,    1,   87,    2, 0x06,    9 /* Public */,
      11,    2,   90,    2, 0x06,   11 /* Public */,
      14,    0,   95,    2, 0x06,   14 /* Public */,
      15,    2,   96,    2, 0x06,   15 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      16,    0,  101,    2, 0x08,   18 /* Private */,
      17,    0,  102,    2, 0x08,   19 /* Private */,
      18,    0,  103,    2, 0x08,   20 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, 0x80000000 | 5, QMetaType::UShort,    6,    7,
    QMetaType::Void, 0x80000000 | 5, QMetaType::UShort,    6,    7,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   12,   13,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,   12,   13,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject Lx200Server::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSLx200ServerENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSLx200ServerENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSLx200ServerENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<Lx200Server, std::true_type>,
        // method 'runningChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'clientConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QHostAddress &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'clientDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QHostAddress &, std::false_type>,
        QtPrivate::TypeAndForceComplete<quint16, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'gotoRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'stopRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'syncRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onNewConnection'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClientReadyRead'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onClientDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void Lx200Server::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<Lx200Server *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->runningChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 1: _t->clientConnected((*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 2: _t->clientDisconnected((*reinterpret_cast< std::add_pointer_t<QHostAddress>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<quint16>>(_a[2]))); break;
        case 3: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->gotoRequested((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 5: _t->stopRequested(); break;
        case 6: _t->syncRequested((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 7: _t->onNewConnection(); break;
        case 8: _t->onClientReadyRead(); break;
        case 9: _t->onClientDisconnected(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (Lx200Server::*)(bool );
            if (_t _q_method = &Lx200Server::runningChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)(const QHostAddress & , quint16 );
            if (_t _q_method = &Lx200Server::clientConnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)(const QHostAddress & , quint16 );
            if (_t _q_method = &Lx200Server::clientDisconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)(const QString & );
            if (_t _q_method = &Lx200Server::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)(double , double );
            if (_t _q_method = &Lx200Server::gotoRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)();
            if (_t _q_method = &Lx200Server::stopRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (Lx200Server::*)(double , double );
            if (_t _q_method = &Lx200Server::syncRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *Lx200Server::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Lx200Server::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSLx200ServerENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Lx200Server::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void Lx200Server::runningChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void Lx200Server::clientConnected(const QHostAddress & _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void Lx200Server::clientDisconnected(const QHostAddress & _t1, quint16 _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void Lx200Server::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void Lx200Server::gotoRequested(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}

// SIGNAL 5
void Lx200Server::stopRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void Lx200Server::syncRequested(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
