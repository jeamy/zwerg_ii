/****************************************************************************
** Meta object code from reading C++ file 'AstroNavigationPanel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "ui/AstroNavigationPanel.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AstroNavigationPanel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS = QtMocHelpers::stringData(
    "AstroNavigationPanel",
    "gotoRequested",
    "",
    "ra",
    "dec",
    "stackingStarted",
    "numFrames",
    "exposureSeconds",
    "stackingStopped",
    "starMapOverlayRequested",
    "enabled",
    "onObjectSelected",
    "CelestialObject",
    "obj",
    "onObjectDoubleClicked",
    "onGotoClicked",
    "onStopGotoClicked",
    "onCalibrateClicked",
    "onCancelCalibrationClicked",
    "onSearchTextChanged",
    "text",
    "onSearchResultClicked",
    "QListWidgetItem*",
    "item",
    "onSearchResultDoubleClicked",
    "onStarMapCoordinatesClicked",
    "onStartStackingClicked",
    "onStopStackingClicked",
    "updateStackingProgress",
    "onMagnitudeLimitChanged",
    "value",
    "onShowConstellationsToggled",
    "checked",
    "onShowGridToggled",
    "onShowLabelsToggled",
    "onAutoLocationClicked",
    "onLocationReceived",
    "location",
    "onLx200EnableToggled",
    "onLx200PortChanged",
    "port",
    "onRaDecGotoClicked"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS_t {
    uint offsetsAndSizes[84];
    char stringdata0[21];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[3];
    char stringdata4[4];
    char stringdata5[16];
    char stringdata6[10];
    char stringdata7[16];
    char stringdata8[16];
    char stringdata9[24];
    char stringdata10[8];
    char stringdata11[17];
    char stringdata12[16];
    char stringdata13[4];
    char stringdata14[22];
    char stringdata15[14];
    char stringdata16[18];
    char stringdata17[19];
    char stringdata18[27];
    char stringdata19[20];
    char stringdata20[5];
    char stringdata21[22];
    char stringdata22[17];
    char stringdata23[5];
    char stringdata24[28];
    char stringdata25[28];
    char stringdata26[23];
    char stringdata27[22];
    char stringdata28[23];
    char stringdata29[24];
    char stringdata30[6];
    char stringdata31[28];
    char stringdata32[8];
    char stringdata33[18];
    char stringdata34[20];
    char stringdata35[22];
    char stringdata36[19];
    char stringdata37[9];
    char stringdata38[21];
    char stringdata39[19];
    char stringdata40[5];
    char stringdata41[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS_t qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20),  // "AstroNavigationPanel"
        QT_MOC_LITERAL(21, 13),  // "gotoRequested"
        QT_MOC_LITERAL(35, 0),  // ""
        QT_MOC_LITERAL(36, 2),  // "ra"
        QT_MOC_LITERAL(39, 3),  // "dec"
        QT_MOC_LITERAL(43, 15),  // "stackingStarted"
        QT_MOC_LITERAL(59, 9),  // "numFrames"
        QT_MOC_LITERAL(69, 15),  // "exposureSeconds"
        QT_MOC_LITERAL(85, 15),  // "stackingStopped"
        QT_MOC_LITERAL(101, 23),  // "starMapOverlayRequested"
        QT_MOC_LITERAL(125, 7),  // "enabled"
        QT_MOC_LITERAL(133, 16),  // "onObjectSelected"
        QT_MOC_LITERAL(150, 15),  // "CelestialObject"
        QT_MOC_LITERAL(166, 3),  // "obj"
        QT_MOC_LITERAL(170, 21),  // "onObjectDoubleClicked"
        QT_MOC_LITERAL(192, 13),  // "onGotoClicked"
        QT_MOC_LITERAL(206, 17),  // "onStopGotoClicked"
        QT_MOC_LITERAL(224, 18),  // "onCalibrateClicked"
        QT_MOC_LITERAL(243, 26),  // "onCancelCalibrationClicked"
        QT_MOC_LITERAL(270, 19),  // "onSearchTextChanged"
        QT_MOC_LITERAL(290, 4),  // "text"
        QT_MOC_LITERAL(295, 21),  // "onSearchResultClicked"
        QT_MOC_LITERAL(317, 16),  // "QListWidgetItem*"
        QT_MOC_LITERAL(334, 4),  // "item"
        QT_MOC_LITERAL(339, 27),  // "onSearchResultDoubleClicked"
        QT_MOC_LITERAL(367, 27),  // "onStarMapCoordinatesClicked"
        QT_MOC_LITERAL(395, 22),  // "onStartStackingClicked"
        QT_MOC_LITERAL(418, 21),  // "onStopStackingClicked"
        QT_MOC_LITERAL(440, 22),  // "updateStackingProgress"
        QT_MOC_LITERAL(463, 23),  // "onMagnitudeLimitChanged"
        QT_MOC_LITERAL(487, 5),  // "value"
        QT_MOC_LITERAL(493, 27),  // "onShowConstellationsToggled"
        QT_MOC_LITERAL(521, 7),  // "checked"
        QT_MOC_LITERAL(529, 17),  // "onShowGridToggled"
        QT_MOC_LITERAL(547, 19),  // "onShowLabelsToggled"
        QT_MOC_LITERAL(567, 21),  // "onAutoLocationClicked"
        QT_MOC_LITERAL(589, 18),  // "onLocationReceived"
        QT_MOC_LITERAL(608, 8),  // "location"
        QT_MOC_LITERAL(617, 20),  // "onLx200EnableToggled"
        QT_MOC_LITERAL(638, 18),  // "onLx200PortChanged"
        QT_MOC_LITERAL(657, 4),  // "port"
        QT_MOC_LITERAL(662, 18)   // "onRaDecGotoClicked"
    },
    "AstroNavigationPanel",
    "gotoRequested",
    "",
    "ra",
    "dec",
    "stackingStarted",
    "numFrames",
    "exposureSeconds",
    "stackingStopped",
    "starMapOverlayRequested",
    "enabled",
    "onObjectSelected",
    "CelestialObject",
    "obj",
    "onObjectDoubleClicked",
    "onGotoClicked",
    "onStopGotoClicked",
    "onCalibrateClicked",
    "onCancelCalibrationClicked",
    "onSearchTextChanged",
    "text",
    "onSearchResultClicked",
    "QListWidgetItem*",
    "item",
    "onSearchResultDoubleClicked",
    "onStarMapCoordinatesClicked",
    "onStartStackingClicked",
    "onStopStackingClicked",
    "updateStackingProgress",
    "onMagnitudeLimitChanged",
    "value",
    "onShowConstellationsToggled",
    "checked",
    "onShowGridToggled",
    "onShowLabelsToggled",
    "onAutoLocationClicked",
    "onLocationReceived",
    "location",
    "onLx200EnableToggled",
    "onLx200PortChanged",
    "port",
    "onRaDecGotoClicked"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSAstroNavigationPanelENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      26,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,  170,    2, 0x06,    1 /* Public */,
       5,    2,  175,    2, 0x06,    4 /* Public */,
       8,    0,  180,    2, 0x06,    7 /* Public */,
       9,    1,  181,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      11,    1,  184,    2, 0x08,   10 /* Private */,
      14,    1,  187,    2, 0x08,   12 /* Private */,
      15,    0,  190,    2, 0x08,   14 /* Private */,
      16,    0,  191,    2, 0x08,   15 /* Private */,
      17,    0,  192,    2, 0x08,   16 /* Private */,
      18,    0,  193,    2, 0x08,   17 /* Private */,
      19,    1,  194,    2, 0x08,   18 /* Private */,
      21,    1,  197,    2, 0x08,   20 /* Private */,
      24,    1,  200,    2, 0x08,   22 /* Private */,
      25,    2,  203,    2, 0x08,   24 /* Private */,
      26,    0,  208,    2, 0x08,   27 /* Private */,
      27,    0,  209,    2, 0x08,   28 /* Private */,
      28,    0,  210,    2, 0x08,   29 /* Private */,
      29,    1,  211,    2, 0x08,   30 /* Private */,
      31,    1,  214,    2, 0x08,   32 /* Private */,
      33,    1,  217,    2, 0x08,   34 /* Private */,
      34,    1,  220,    2, 0x08,   36 /* Private */,
      35,    0,  223,    2, 0x08,   38 /* Private */,
      36,    1,  224,    2, 0x08,   39 /* Private */,
      38,    1,  227,    2, 0x08,   41 /* Private */,
      39,    1,  230,    2, 0x08,   43 /* Private */,
      41,    0,  233,    2, 0x08,   45 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    3,    4,
    QMetaType::Void, QMetaType::Int, QMetaType::Double,    6,    7,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   10,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void, 0x80000000 | 12,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, 0x80000000 | 22,   23,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,   30,
    QMetaType::Void, QMetaType::Bool,   32,
    QMetaType::Void, QMetaType::Bool,   32,
    QMetaType::Void, QMetaType::Bool,   32,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QJsonObject,   37,
    QMetaType::Void, QMetaType::Bool,   10,
    QMetaType::Void, QMetaType::Int,   40,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject AstroNavigationPanel::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSAstroNavigationPanelENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<AstroNavigationPanel, std::true_type>,
        // method 'gotoRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'stackingStarted'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'stackingStopped'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'starMapOverlayRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onObjectSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const CelestialObject &, std::false_type>,
        // method 'onObjectDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const CelestialObject &, std::false_type>,
        // method 'onGotoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStopGotoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCalibrateClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onCancelCalibrationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSearchTextChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'onSearchResultClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onSearchResultDoubleClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<QListWidgetItem *, std::false_type>,
        // method 'onStarMapCoordinatesClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onStartStackingClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onStopStackingClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'updateStackingProgress'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onMagnitudeLimitChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<double, std::false_type>,
        // method 'onShowConstellationsToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onShowGridToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onShowLabelsToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onAutoLocationClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onLocationReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'onLx200EnableToggled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'onLx200PortChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onRaDecGotoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void AstroNavigationPanel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<AstroNavigationPanel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->gotoRequested((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 1: _t->stackingStarted((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 2: _t->stackingStopped(); break;
        case 3: _t->starMapOverlayRequested((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 4: _t->onObjectSelected((*reinterpret_cast< std::add_pointer_t<CelestialObject>>(_a[1]))); break;
        case 5: _t->onObjectDoubleClicked((*reinterpret_cast< std::add_pointer_t<CelestialObject>>(_a[1]))); break;
        case 6: _t->onGotoClicked(); break;
        case 7: _t->onStopGotoClicked(); break;
        case 8: _t->onCalibrateClicked(); break;
        case 9: _t->onCancelCalibrationClicked(); break;
        case 10: _t->onSearchTextChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 11: _t->onSearchResultClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 12: _t->onSearchResultDoubleClicked((*reinterpret_cast< std::add_pointer_t<QListWidgetItem*>>(_a[1]))); break;
        case 13: _t->onStarMapCoordinatesClicked((*reinterpret_cast< std::add_pointer_t<double>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<double>>(_a[2]))); break;
        case 14: _t->onStartStackingClicked(); break;
        case 15: _t->onStopStackingClicked(); break;
        case 16: _t->updateStackingProgress(); break;
        case 17: _t->onMagnitudeLimitChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        case 18: _t->onShowConstellationsToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->onShowGridToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->onShowLabelsToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->onAutoLocationClicked(); break;
        case 22: _t->onLocationReceived((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 23: _t->onLx200EnableToggled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 24: _t->onLx200PortChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 25: _t->onRaDecGotoClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (AstroNavigationPanel::*)(double , double );
            if (_t _q_method = &AstroNavigationPanel::gotoRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (AstroNavigationPanel::*)(int , double );
            if (_t _q_method = &AstroNavigationPanel::stackingStarted; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (AstroNavigationPanel::*)();
            if (_t _q_method = &AstroNavigationPanel::stackingStopped; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (AstroNavigationPanel::*)(bool );
            if (_t _q_method = &AstroNavigationPanel::starMapOverlayRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *AstroNavigationPanel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AstroNavigationPanel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSAstroNavigationPanelENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int AstroNavigationPanel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 26)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 26;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 26)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 26;
    }
    return _id;
}

// SIGNAL 0
void AstroNavigationPanel::gotoRequested(double _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void AstroNavigationPanel::stackingStarted(int _t1, double _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void AstroNavigationPanel::stackingStopped()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void AstroNavigationPanel::starMapOverlayRequested(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
