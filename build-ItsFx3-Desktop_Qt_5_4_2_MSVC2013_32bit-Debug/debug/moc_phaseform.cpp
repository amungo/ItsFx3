/****************************************************************************
** Meta object code from reading C++ file 'phaseform.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../phaseform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'phaseform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_PhaseForm_t {
    QByteArrayData data[17];
    char stringdata[255];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PhaseForm_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PhaseForm_t qt_meta_stringdata_PhaseForm = {
    {
QT_MOC_LITERAL(0, 0, 9), // "PhaseForm"
QT_MOC_LITERAL(1, 10, 13), // "ChangeNullMhz"
QT_MOC_LITERAL(2, 24, 0), // ""
QT_MOC_LITERAL(3, 25, 6), // "newVal"
QT_MOC_LITERAL(4, 32, 16), // "CurChangeOutside"
QT_MOC_LITERAL(5, 49, 5), // "value"
QT_MOC_LITERAL(6, 55, 21), // "CurChangeButtonUpSlow"
QT_MOC_LITERAL(7, 77, 21), // "CurChangeButtonUpFast"
QT_MOC_LITERAL(8, 99, 23), // "CurChangeButtonDownSlow"
QT_MOC_LITERAL(9, 123, 23), // "CurChangeButtonDownFast"
QT_MOC_LITERAL(10, 147, 13), // "CurBandChange"
QT_MOC_LITERAL(11, 161, 15), // "CurBandChangeUp"
QT_MOC_LITERAL(12, 177, 17), // "CurBandChangeDown"
QT_MOC_LITERAL(13, 195, 20), // "CalibrateApplyPhases"
QT_MOC_LITERAL(14, 216, 16), // "CalibrateDefault"
QT_MOC_LITERAL(15, 233, 13), // "ChangeEtalons"
QT_MOC_LITERAL(16, 247, 7) // "slotRun"

    },
    "PhaseForm\0ChangeNullMhz\0\0newVal\0"
    "CurChangeOutside\0value\0CurChangeButtonUpSlow\0"
    "CurChangeButtonUpFast\0CurChangeButtonDownSlow\0"
    "CurChangeButtonDownFast\0CurBandChange\0"
    "CurBandChangeUp\0CurBandChangeDown\0"
    "CalibrateApplyPhases\0CalibrateDefault\0"
    "ChangeEtalons\0slotRun"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PhaseForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   79,    2, 0x0a /* Public */,
       4,    1,   82,    2, 0x0a /* Public */,
       6,    1,   85,    2, 0x0a /* Public */,
       7,    1,   88,    2, 0x0a /* Public */,
       8,    1,   91,    2, 0x0a /* Public */,
       9,    1,   94,    2, 0x0a /* Public */,
      10,    1,   97,    2, 0x0a /* Public */,
      11,    1,  100,    2, 0x0a /* Public */,
      12,    1,  103,    2, 0x0a /* Public */,
      13,    1,  106,    2, 0x0a /* Public */,
      14,    1,  109,    2, 0x0a /* Public */,
      15,    1,  112,    2, 0x0a /* Public */,
      16,    1,  115,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void PhaseForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        PhaseForm *_t = static_cast<PhaseForm *>(_o);
        switch (_id) {
        case 0: _t->ChangeNullMhz((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->CurChangeOutside((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->CurChangeButtonUpSlow((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->CurChangeButtonUpFast((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->CurChangeButtonDownSlow((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->CurChangeButtonDownFast((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 6: _t->CurBandChange((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->CurBandChangeUp((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->CurBandChangeDown((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->CalibrateApplyPhases((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 10: _t->CalibrateDefault((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 11: _t->ChangeEtalons((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: _t->slotRun((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject PhaseForm::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_PhaseForm.data,
      qt_meta_data_PhaseForm,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *PhaseForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PhaseForm::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_PhaseForm.stringdata))
        return static_cast<void*>(const_cast< PhaseForm*>(this));
    if (!strcmp(_clname, "StreamDataHandler"))
        return static_cast< StreamDataHandler*>(const_cast< PhaseForm*>(this));
    return QWidget::qt_metacast(_clname);
}

int PhaseForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 13;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
