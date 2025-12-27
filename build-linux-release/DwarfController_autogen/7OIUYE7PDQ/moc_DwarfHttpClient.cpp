/****************************************************************************
** Meta object code from reading C++ file 'DwarfHttpClient.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfHttpClient.h"
#include <QtNetwork/QSslError>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfHttpClient.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS = QtMocHelpers::stringData(
    "DwarfHttpClient",
    "mediaListReceived",
    "",
    "document",
    "defaultParamsConfigReceived",
    "mediaDeleted",
    "filePath",
    "deleteError",
    "error",
    "errorOccurred"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS_t {
    uint offsetsAndSizes[20];
    char stringdata0[16];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[28];
    char stringdata5[13];
    char stringdata6[9];
    char stringdata7[12];
    char stringdata8[6];
    char stringdata9[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS_t qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS = {
    {
        QT_MOC_LITERAL(0, 15),  // "DwarfHttpClient"
        QT_MOC_LITERAL(16, 17),  // "mediaListReceived"
        QT_MOC_LITERAL(34, 0),  // ""
        QT_MOC_LITERAL(35, 8),  // "document"
        QT_MOC_LITERAL(44, 27),  // "defaultParamsConfigReceived"
        QT_MOC_LITERAL(72, 12),  // "mediaDeleted"
        QT_MOC_LITERAL(85, 8),  // "filePath"
        QT_MOC_LITERAL(94, 11),  // "deleteError"
        QT_MOC_LITERAL(106, 5),  // "error"
        QT_MOC_LITERAL(112, 13)   // "errorOccurred"
    },
    "DwarfHttpClient",
    "mediaListReceived",
    "",
    "document",
    "defaultParamsConfigReceived",
    "mediaDeleted",
    "filePath",
    "deleteError",
    "error",
    "errorOccurred"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfHttpClientENDCLASS[] = {

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
       5,    1,   50,    2, 0x06,    5 /* Public */,
       7,    2,   53,    2, 0x06,    7 /* Public */,
       9,    1,   58,    2, 0x06,   10 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QJsonDocument,    3,
    QMetaType::Void, QMetaType::QJsonDocument,    3,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    6,    8,
    QMetaType::Void, QMetaType::QString,    8,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfHttpClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfHttpClientENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfHttpClient, std::true_type>,
        // method 'mediaListReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonDocument &, std::false_type>,
        // method 'defaultParamsConfigReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonDocument &, std::false_type>,
        // method 'mediaDeleted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'deleteError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'errorOccurred'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void DwarfHttpClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfHttpClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->mediaListReceived((*reinterpret_cast< std::add_pointer_t<QJsonDocument>>(_a[1]))); break;
        case 1: _t->defaultParamsConfigReceived((*reinterpret_cast< std::add_pointer_t<QJsonDocument>>(_a[1]))); break;
        case 2: _t->mediaDeleted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->deleteError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 4: _t->errorOccurred((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfHttpClient::*)(const QJsonDocument & );
            if (_t _q_method = &DwarfHttpClient::mediaListReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfHttpClient::*)(const QJsonDocument & );
            if (_t _q_method = &DwarfHttpClient::defaultParamsConfigReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfHttpClient::*)(const QString & );
            if (_t _q_method = &DwarfHttpClient::mediaDeleted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfHttpClient::*)(const QString & , const QString & );
            if (_t _q_method = &DwarfHttpClient::deleteError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (DwarfHttpClient::*)(const QString & );
            if (_t _q_method = &DwarfHttpClient::errorOccurred; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
    }
}

const QMetaObject *DwarfHttpClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfHttpClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfHttpClientENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfHttpClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void DwarfHttpClient::mediaListReceived(const QJsonDocument & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfHttpClient::defaultParamsConfigReceived(const QJsonDocument & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfHttpClient::mediaDeleted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfHttpClient::deleteError(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void DwarfHttpClient::errorOccurred(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
