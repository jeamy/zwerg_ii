/****************************************************************************
** Meta object code from reading C++ file 'DwarfMtpClient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfMtpClient.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfMtpClient.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS = QtMocHelpers::stringData(
    "DwarfMtpClient",
    "toolCheckCompleted",
    "",
    "available",
    "message",
    "deleteStarted",
    "filePath",
    "deleteFinished",
    "success",
    "error"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS_t {
    uint offsetsAndSizes[20];
    char stringdata0[15];
    char stringdata1[19];
    char stringdata2[1];
    char stringdata3[10];
    char stringdata4[8];
    char stringdata5[14];
    char stringdata6[9];
    char stringdata7[15];
    char stringdata8[8];
    char stringdata9[6];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS_t qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS = {
    {
        QT_MOC_LITERAL(0, 14),  // "DwarfMtpClient"
        QT_MOC_LITERAL(15, 18),  // "toolCheckCompleted"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 9),  // "available"
        QT_MOC_LITERAL(45, 7),  // "message"
        QT_MOC_LITERAL(53, 13),  // "deleteStarted"
        QT_MOC_LITERAL(67, 8),  // "filePath"
        QT_MOC_LITERAL(76, 14),  // "deleteFinished"
        QT_MOC_LITERAL(91, 7),  // "success"
        QT_MOC_LITERAL(99, 5)   // "error"
    },
    "DwarfMtpClient",
    "toolCheckCompleted",
    "",
    "available",
    "message",
    "deleteStarted",
    "filePath",
    "deleteFinished",
    "success",
    "error"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfMtpClientENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   32,    2, 0x06,    1 /* Public */,
       5,    1,   37,    2, 0x06,    4 /* Public */,
       7,    3,   40,    2, 0x06,    6 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,    3,    4,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, QMetaType::QString,    8,    6,    9,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfMtpClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfMtpClientENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfMtpClient, std::true_type>,
        // method 'toolCheckCompleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void DwarfMtpClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfMtpClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->toolCheckCompleted((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->deleteStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: _t->deleteFinished((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfMtpClient::*)(bool , const QString & );
            if (_t _q_method = &DwarfMtpClient::toolCheckCompleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfMtpClient::*)(const QString & );
            if (_t _q_method = &DwarfMtpClient::deleteStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfMtpClient::*)(bool , const QString & , const QString & );
            if (_t _q_method = &DwarfMtpClient::deleteFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *DwarfMtpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfMtpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfMtpClientENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfMtpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void DwarfMtpClient::toolCheckCompleted(bool _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfMtpClient::deleteStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfMtpClient::deleteFinished(bool _t1, const QString & _t2, const QString & _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
