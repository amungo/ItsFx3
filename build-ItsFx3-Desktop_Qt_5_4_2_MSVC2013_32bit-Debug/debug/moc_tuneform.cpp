/****************************************************************************
** Meta object code from reading C++ file 'tuneform.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../tuneform.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tuneform.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_TuneForm_t {
    QByteArrayData data[10];
    char stringdata[124];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_TuneForm_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_TuneForm_t qt_meta_stringdata_TuneForm = {
    {
QT_MOC_LITERAL(0, 0, 8), // "TuneForm"
QT_MOC_LITERAL(1, 9, 7), // "newFreq"
QT_MOC_LITERAL(2, 17, 0), // ""
QT_MOC_LITERAL(3, 18, 16), // "setDevicePointer"
QT_MOC_LITERAL(4, 35, 11), // "FX3DevIfce*"
QT_MOC_LITERAL(5, 47, 11), // "dev_pointer"
QT_MOC_LITERAL(6, 59, 17), // "onButtonApplyFreq"
QT_MOC_LITERAL(7, 77, 13), // "onBandChanged"
QT_MOC_LITERAL(8, 91, 12), // "onAGCChanged"
QT_MOC_LITERAL(9, 104, 19) // "onManualGainChanged"

    },
    "TuneForm\0newFreq\0\0setDevicePointer\0"
    "FX3DevIfce*\0dev_pointer\0onButtonApplyFreq\0"
    "onBandChanged\0onAGCChanged\0"
    "onManualGainChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_TuneForm[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    1,   47,    2, 0x0a /* Public */,
       6,    1,   50,    2, 0x0a /* Public */,
       7,    1,   53,    2, 0x0a /* Public */,
       8,    1,   56,    2, 0x0a /* Public */,
       9,    1,   59,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 4,    5,
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

void TuneForm::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        TuneForm *_t = static_cast<TuneForm *>(_o);
        switch (_id) {
        case 0: _t->newFreq((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->setDevicePointer((*reinterpret_cast< FX3DevIfce*(*)>(_a[1]))); break;
        case 2: _t->onButtonApplyFreq((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->onBandChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->onAGCChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->onManualGainChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (TuneForm::*_t)(double );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&TuneForm::newFreq)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject TuneForm::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_TuneForm.data,
      qt_meta_data_TuneForm,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *TuneForm::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TuneForm::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_TuneForm.stringdata))
        return static_cast<void*>(const_cast< TuneForm*>(this));
    return QWidget::qt_metacast(_clname);
}

int TuneForm::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void TuneForm::newFreq(double _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
