/****************************************************************************
** Meta object code from reading C++ file 'SpectrumForm.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../SpectrumForm.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SpectrumForm.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_SpectrumForm_t {
    QByteArrayData data[12];
    char stringdata[152];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SpectrumForm_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SpectrumForm_t qt_meta_stringdata_SpectrumForm = {
    {
QT_MOC_LITERAL(0, 0, 12), // "SpectrumForm"
QT_MOC_LITERAL(1, 13, 13), // "ChangeNullMhz"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 6), // "newVal"
QT_MOC_LITERAL(4, 35, 16), // "CurChangeOutside"
QT_MOC_LITERAL(5, 52, 5), // "value"
QT_MOC_LITERAL(6, 58, 7), // "slotRun"
QT_MOC_LITERAL(7, 66, 15), // "channelsChanged"
QT_MOC_LITERAL(8, 82, 19), // "scalesShiftsChanged"
QT_MOC_LITERAL(9, 102, 15), // "bandTypeChanged"
QT_MOC_LITERAL(10, 118, 16), // "avgSimpleChanged"
QT_MOC_LITERAL(11, 135, 16) // "avgMatrixChanged"

    },
    "SpectrumForm\0ChangeNullMhz\0\0newVal\0"
    "CurChangeOutside\0value\0slotRun\0"
    "channelsChanged\0scalesShiftsChanged\0"
    "bandTypeChanged\0avgSimpleChanged\0"
    "avgMatrixChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SpectrumForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   54,    2, 0x0a /* Public */,
       4,    1,   57,    2, 0x0a /* Public */,
       6,    1,   60,    2, 0x08 /* Private */,
       7,    1,   63,    2, 0x08 /* Private */,
       8,    1,   66,    2, 0x08 /* Private */,
       9,    1,   69,    2, 0x08 /* Private */,
      10,    1,   72,    2, 0x08 /* Private */,
      11,    1,   75,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void SpectrumForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        SpectrumForm *_t = static_cast<SpectrumForm *>(_o);
        switch (_id) {
        case 0: _t->ChangeNullMhz((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->CurChangeOutside((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slotRun((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->channelsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->scalesShiftsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->bandTypeChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->avgSimpleChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->avgMatrixChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObject SpectrumForm::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SpectrumForm.data,
      qt_meta_data_SpectrumForm,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *SpectrumForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SpectrumForm::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_SpectrumForm.stringdata))
        return static_cast<void*>(const_cast< SpectrumForm*>(this));
    if (!strcmp(_clname, "StreamDataHandler"))
        return static_cast< StreamDataHandler*>(const_cast< SpectrumForm*>(this));
    return QWidget::qt_metacast(_clname);
}

int SpectrumForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
