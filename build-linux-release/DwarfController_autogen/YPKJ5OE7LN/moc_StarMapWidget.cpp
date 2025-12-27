/****************************************************************************
** Meta object code from reading C++ file 'StarMapWidget.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/StarMapWidget.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StarMapWidget.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSStarMapWidgetENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSStarMapWidgetENDCLASS = QtMocHelpers::stringData(
    "StarMapWidget",
    "objectSelected",
    "",
    "CelestialObject",
    "obj",
    "objectDoubleClicked",
    "gotoRequested",
    "ra",
    "dec",
    "coordinatesClicked",
    "updateSky"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSStarMapWidgetENDCLASS_t {
    uint offsetsAndSizes[22];
    char stringdata0[14];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[16];
    char stringdata4[4];
    char stringdata5[20];
    char stringdata6[14];
    char stringdata7[3];
    char stringdata8[4];
    char stringdata9[19];
    char stringdata10[10];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSStarMapWidgetENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSStarMapWidgetENDCLASS_t qt_meta_stringdata_CLASSStarMapWidgetENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "StarMapWidget"
        QT_MOC_LITERAL(14, 14),  // "objectSelected"
        QT_MOC_LITERAL(29, 0),  // ""
        QT_MOC_LITERAL(30, 15),  // "CelestialObject"
        QT_MOC_LITERAL(46, 3),  // "obj"
        QT_MOC_LITERAL(50, 19),  // "objectDoubleClicked"
        QT_MOC_LITERAL(70, 13),  // "gotoRequested"
        QT_MOC_LITERAL(84, 2),  // "ra"
        QT_MOC_LITERAL(87, 3),  // "dec"
        QT_MOC_LITERAL(91, 18),  // "coordinatesClicked"
        QT_MOC_LITERAL(110, 9)   // "updateSky"
    },
    "StarMapWidget",
    "objectSelected",
    "",
    "CelestialObject",
    "obj",
    "objectDoubleClicked",
    "gotoRequested",
    "ra",
    "dec",
    "coordinatesClicked",
    "updateSky"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSStarMapWidgetENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   44,    2, 0x06,    1 /* Public */,
       5,    1,   47,    2, 0x06,    3 /* Public */,
       6,    2,   50,    2, 0x06,    5 /* Public */,
       9,    2,   55,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    0,   60,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    7,    8,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    7,    8,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject StarMapWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsView::staticMetaObject>(),
    qt_meta_stringdata_CLASSStarMapWidgetENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSStarMapWidgetENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSStarMapWidgetENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StarMapWidget, std::true_type>,
        // method 'objectSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const CelestialObject &, std::false_type>,
        // method 'objectDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const CelestialObject &, std::false_type>,
        // method 'gotoRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'coordinatesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'updateSky'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void StarMapWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StarMapWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->objectSelected((*reinterpret_cast< std::add_pointer_t<CelestialObject>>(_a[1]))); break;
        case 1: _t->objectDoubleClicked((*reinterpret_cast< std::add_pointer_t<CelestialObject>>(_a[1]))); break;
        case 2: _t->gotoRequested((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 3: _t->coordinatesClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 4: _t->updateSky(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StarMapWidget::*)(const CelestialObject & );
            if (_t _q_method = &StarMapWidget::objectSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StarMapWidget::*)(const CelestialObject & );
            if (_t _q_method = &StarMapWidget::objectDoubleClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StarMapWidget::*)(double , double );
            if (_t _q_method = &StarMapWidget::gotoRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StarMapWidget::*)(double , double );
            if (_t _q_method = &StarMapWidget::coordinatesClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *StarMapWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StarMapWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSStarMapWidgetENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QGraphicsView::qt_metacast(_clname);
}

int StarMapWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsView::qt_metacall(_c, _id, _a);
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
void StarMapWidget::objectSelected(const CelestialObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void StarMapWidget::objectDoubleClicked(const CelestialObject & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void StarMapWidget::gotoRequested(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void StarMapWidget::coordinatesClicked(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
