/****************************************************************************
** Meta object code from reading C++ file 'DwarfFtpDownloader.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "net/DwarfFtpDownloader.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DwarfFtpDownloader.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS = QtMocHelpers::stringData(
    "DwarfFtpDownloader",
    "downloadStarted",
    "",
    "fileName",
    "downloadFinished",
    "localPath",
    "downloadError",
    "error",
    "downloadProgress",
    "received",
    "total",
    "onFtpDownloadStarted",
    "remotePath",
    "onFtpDownloadFinished",
    "onFtpDownloadProgress",
    "onFtpError"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS_t {
    uint offsetsAndSizes[32];
    char stringdata0[19];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[17];
    char stringdata5[10];
    char stringdata6[14];
    char stringdata7[6];
    char stringdata8[17];
    char stringdata9[9];
    char stringdata10[6];
    char stringdata11[21];
    char stringdata12[11];
    char stringdata13[22];
    char stringdata14[22];
    char stringdata15[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS_t qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS = {
    {
        QT_MOC_LITERAL(0, 18),  // "DwarfFtpDownloader"
        QT_MOC_LITERAL(19, 15),  // "downloadStarted"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 8),  // "fileName"
        QT_MOC_LITERAL(45, 16),  // "downloadFinished"
        QT_MOC_LITERAL(62, 9),  // "localPath"
        QT_MOC_LITERAL(72, 13),  // "downloadError"
        QT_MOC_LITERAL(86, 5),  // "error"
        QT_MOC_LITERAL(92, 16),  // "downloadProgress"
        QT_MOC_LITERAL(109, 8),  // "received"
        QT_MOC_LITERAL(118, 5),  // "total"
        QT_MOC_LITERAL(124, 20),  // "onFtpDownloadStarted"
        QT_MOC_LITERAL(145, 10),  // "remotePath"
        QT_MOC_LITERAL(156, 21),  // "onFtpDownloadFinished"
        QT_MOC_LITERAL(178, 21),  // "onFtpDownloadProgress"
        QT_MOC_LITERAL(200, 10)   // "onFtpError"
    },
    "DwarfFtpDownloader",
    "downloadStarted",
    "",
    "fileName",
    "downloadFinished",
    "localPath",
    "downloadError",
    "error",
    "downloadProgress",
    "received",
    "total",
    "onFtpDownloadStarted",
    "remotePath",
    "onFtpDownloadFinished",
    "onFtpDownloadProgress",
    "onFtpError"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDwarfFtpDownloaderENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       4,    2,   65,    2, 0x06,    3 /* Public */,
       6,    2,   70,    2, 0x06,    6 /* Public */,
       8,    3,   75,    2, 0x06,    9 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,   82,    2, 0x08,   13 /* Private */,
      13,    2,   85,    2, 0x08,   15 /* Private */,
      14,    3,   90,    2, 0x08,   18 /* Private */,
      15,    2,   97,    2, 0x08,   22 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    7,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,    3,    9,   10,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   12,    5,
    QMetaType::Void, QMetaType::QString, QMetaType::LongLong, QMetaType::LongLong,   12,    9,   10,
    QMetaType::Void, QMetaType::QString, QMetaType::QString,   12,    7,

       0        // eod
};

Q_CONSTINIT const QMetaObject DwarfFtpDownloader::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDwarfFtpDownloaderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DwarfFtpDownloader, std::true_type>,
        // method 'downloadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'downloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onFtpDownloadStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onFtpDownloadFinished'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onFtpDownloadProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        QtPrivate::TypeAndForceComplete<qint64, std::false_type>,
        // method 'onFtpError'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>
    >,
    nullptr
} };

void DwarfFtpDownloader::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DwarfFtpDownloader *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->downloadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->downloadFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 2: _t->downloadError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 3: _t->downloadProgress((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3]))); break;
        case 4: _t->onFtpDownloadStarted((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->onFtpDownloadFinished((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 6: _t->onFtpDownloadProgress((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<qint64>>(_a[3]))); break;
        case 7: _t->onFtpError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DwarfFtpDownloader::*)(const QString & );
            if (_t _q_method = &DwarfFtpDownloader::downloadStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DwarfFtpDownloader::*)(const QString & , const QString & );
            if (_t _q_method = &DwarfFtpDownloader::downloadFinished; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DwarfFtpDownloader::*)(const QString & , const QString & );
            if (_t _q_method = &DwarfFtpDownloader::downloadError; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DwarfFtpDownloader::*)(const QString & , qint64 , qint64 );
            if (_t _q_method = &DwarfFtpDownloader::downloadProgress; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *DwarfFtpDownloader::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DwarfFtpDownloader::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDwarfFtpDownloaderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int DwarfFtpDownloader::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DwarfFtpDownloader::downloadStarted(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DwarfFtpDownloader::downloadFinished(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DwarfFtpDownloader::downloadError(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void DwarfFtpDownloader::downloadProgress(const QString & _t1, qint64 _t2, qint64 _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
